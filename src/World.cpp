#include <algorithm>
#include "Engine/Log.hpp"
#include "World.hpp"

static CR::T_OBJID genObjId(){
    static std::mutex mutex;
    mutex.lock();
        static CR::T_OBJID start = CR::Math::random(25, 35);
        auto n = ++start;
    mutex.unlock();
    return n;
}

static CR::T_OBJID genWorldId(){
    static std::mutex mutex;
    mutex.lock();
        static CR::T_OBJID start = CR::Math::random(1001, 1099);
        auto n = ++start;
    mutex.unlock();
    return n;
}

CR::World::World(){
    this->currentTick = 0;
    this->puppetMode = false;
    this->wId = genWorldId();
    this->state = WorldState::IDLE;
    this->auditBacklog.push_back(this->createAudit(AuditType::GAME_SIM_START_OF_FRAME));  
    this->auditBacklog.push_back(this->createAudit(AuditType::GAME_SIM_CREATED));
}


void CR::World::start(){
    this->currentTick = 0;
    this->tickRate = 1000 / 60; // hardcoded for now
    if(!puppetMode){
        this->auditBacklog.push_back(this->createAudit(AuditType::GAME_SIM_STARTED));    
        setState(CR::WorldState::RUNNING); // manually start simulation for now  
    }else{
        this->auditBacklog.clear();
    }
    CR::log("World[%i] started simulation [%s]\n", this->wId, puppetMode ? "PUPPET" : "REAL");    
}

void CR::World::reqEnd(){
     
}

std::shared_ptr<CR::Audit> CR::World::createAudit(CR::T_AUDIT type){
    auto audit = CR::createAudit(type);
    audit->state = state;
    audit->tick = this->currentTick;
    return audit;
}

std::shared_ptr<CR::Audit> CR::World::createAudit(const std::string &msg){
    auto audit = CR::createAudit(msg);
    audit->state = state;
    audit->tick = this->currentTick;
    return audit;
}


bool CR::World::apply(const std::shared_ptr<CR::Audit> &audit){
    return true;
}


void CR::World::setState(T_STATE nstate){
    this->prevState = nstate;
    this->state = nstate;
    this->lastState = CR::ticks();
    auto audit = this->createAudit(AuditType::GAME_SIM_STATE_CHANGED);
    audit->state = nstate;
    this->auditBacklog.push_back(audit);
}

bool CR::World::run(const std::vector<std::shared_ptr<CR::Audit>> &audits){
    if(!puppetMode){ // client
        CR::log("World[%i] run providing audits was used in non-puppetMode\n");
        return false;
    }   
    // TODO: manually run audits 

    return true;
}

void CR::World::setPuppet(bool puppetMode, CR::T_OBJID wId){
    this->puppetMode = puppetMode;
    this->wId = wId;
}

bool CR::World::run(unsigned ticks){
    if(puppetMode){ // client
        CR::log("World[%i] run was used in puppetMode\n");
        return false;
    }

    // apply floating audits (not generated by the simulation ie external)
    if(auditBacklog.size() > 0){
        for(unsigned i = 0; i < auditBacklog.size(); ++i){
            auto &audit = auditBacklog[i];
            if(audit->tick != currentTick) continue;
            audit->time = CR::ticks();
            audit->order = i;
            if(apply(audit)){
                this->auditHistory[audit->tick].push_back(audit);   
            }else{
                CR::log("World[%i] Fatal error has occured applying an audit: game will stop\n");
                CR::Core::exit(1); // TODO: Find a better way to handle this
                return false;
            }
        }
        auditBacklog.clear();
    }

    this->auditBacklog.push_back(this->createAudit(AuditType::GAME_SIM_END_OF_FRAME));       

    // run sim per tick
    for(unsigned i = 0; i < ticks; ++i){
        ++currentTick;
        for(unsigned i = 0; i < objects.size(); ++i){
            objects[i]->onStep();
        }
        totalSimTime += tickRate;
    }

    this->auditBacklog.push_back(this->createAudit(AuditType::GAME_SIM_START_OF_FRAME));

    std::sort(auditBacklog.begin(), auditBacklog.end(), [&](std::shared_ptr<CR::Audit> &a, std::shared_ptr<CR::Audit> &b) {
        return a->tick < b->tick;
    });   

    bool newChanges = auditBacklog.size() > 0;    

    // apply after audit after tick
    if(auditBacklog.size() > 0){
        for(unsigned i = 0; i < auditBacklog.size(); ++i){
            auto &audit = auditBacklog[i];
            audit->time = CR::ticks();
            audit->order = i;
            if(apply(audit)){
                this->auditHistory[audit->tick].push_back(audit);   
            }else{
                CR::log("World[%i] Fatal error has occured applying an audit: game will stop\n");
                CR::Core::exit(1); // TODO: Find a better way to handle this
                return false;
            }
        }
        auditBacklog.clear();
    }
    
    return newChanges;
}

CR::T_OBJID CR::World::add(const std::shared_ptr<CR::Object> &obj){
    if(obj->world != NULL || obj->id != CR::OBJ_ID_INVALID){
        std::string errmsg = CR::String::format("World[%i] Failed to add object %i[%p]: it belongs to another world or wasn't properly destroyed", this->wId, obj->id, obj.get());
        this->auditBacklog.push_back(this->createAudit(errmsg));
        CR::log("%s\n", errmsg.c_str());
        return CR::OBJ_ID_INVALID;
    }
    for(unsigned i = 0; i < this->objects.size(); ++i){
        if(objects[i]->id == obj->id || objects[i].get() == obj.get()){          
            std::string errmsg = CR::String::format("World[%i] Failed to add object %i[%p]: it already exists in it", this->wId, objects[i]->id, objects[i].get());
            this->auditBacklog.push_back(this->createAudit(errmsg));
            CR::log("%s\n", errmsg.c_str());
            return CR::OBJ_ID_INVALID;
        }
    }
    obj->id = genObjId();
    obj->world = this;
    obj->destroyed = false;
    CR::log("World[%i] added object[%i] at <%i,%i>\n", this->wId, obj->id);
    auto audit = this->createAudit(AuditType::OBJECT_CREATED);
    audit->affEnt.push_back(obj->id);
    audit->data.write(&obj->sigType, sizeof(T_OBJSIG));
    audit->data.write(&obj->loc->index, sizeof(T_WORLDPOS));
    audit->data.write(&obj->loc->level, sizeof(T_WORLDPOS));
    this->auditBacklog.push_back(audit);
    obj->onCreate();
    return obj->id;
}

bool CR::World::exists(CR::T_OBJID id){
    for(unsigned i = 0; i < this->objects.size(); ++i){
        if(objects[i]->id == id){
            return true;
        }
    }
    return false;
}

std::shared_ptr<CR::Object> CR::World::get(CR::T_OBJID id){
    for(unsigned i = 0; i < this->objects.size(); ++i){
        if(this->objects[i]->id == id){
            return this->objects[i];
        }
    }
    return std::shared_ptr<CR::Object>(NULL);
}

bool CR::World::destroy(T_OBJID id){
    if(!exists(id)){
        std::string errmsg = CR::String::format("World[%i] Failed to destroy object %i[%p]: it's not part of it", this->wId, id);
        this->auditBacklog.push_back(this->createAudit(errmsg));
        CR::log("%s\n", errmsg.c_str());        
        return false;
    }
    auto obj = get(id);
    if(obj.get() == NULL){
        std::string errmsg = CR::String::format("World[%i] FATAL ERROR: Failed to destroy object %i[%p]: this object is null", this->wId, id);
        this->auditBacklog.push_back(this->createAudit(errmsg));
        CR::log("%s\n", errmsg.c_str());        
        return false; 
    }
    if(obj->destroyed){
        return true;
    }
    for(unsigned i = 0; i < this->toRemObjs.size(); ++i){
        if(this->toRemObjs[i] == id){
            return true;
        }
    }

    this->toRemObjs.push_back(id);
    obj->destroyed = true;
}

void CR::World::render(const CR::Vec2<unsigned> &offset){

}