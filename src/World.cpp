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
    this->wId = genWorldId();
    this->state = WorldState::IDLE;
    this->auditBacklog.push_back(this->createFrame(FrameType::GAME_SIM_CREATED));
}

void CR::World::start(){
    this->auditBacklog.push_back(this->createFrame(FrameType::GAME_SIM_STARTED));    
    CR::log("World[%i] started simulation\n", this->wId);    
}

void CR::World::reqEnd(){
     
}

std::shared_ptr<CR::Frame> CR::World::createFrame(CR::T_FRAME type){
    auto frame = CR::createFrame(type);
    frame->state = state;
    frame->tick = this->currentTick;
    return frame;
}

std::shared_ptr<CR::Frame> CR::World::createFrame(const std::string &msg){
    auto frame = CR::createFrame(msg);
    frame->state = state;
    frame->tick = this->currentTick;
    return frame;
}


bool CR::World::apply(const std::shared_ptr<CR::Frame> &audit){
    return true;
}


void CR::World::setState(T_STATE nstate){
    this->prevState = nstate;
    this->state = nstate;
    this->lastState = CR::ticks();
    auto frame = this->createFrame(FrameType::GAME_SIM_STATE_CHANGED);
    frame->state = nstate;
    this->auditBacklog.push_back(frame);
}

void CR::World::run(unsigned ticks){
    // game & sim
    for(unsigned i = 0; i < ticks; ++i){

        ++currentTick;
    }
    // audits
    for(unsigned i = 0; i < auditBacklog.size(); ++i){
        auto &audit = auditBacklog[i];
        audit->time = CR::ticks();
        audit->order = auditHistory.size();
        if(apply(audit)){
            this->auditHistory.push_back(audit);   
        }else{
            CR::log("World[%i] fatal error has occured applying a frame: game will stop\n");
            CR::Core::exit(1); // TODO: Find a better way to handle this
            return;
        }
    }
    auditBacklog.clear();
}

CR::T_OBJID CR::World::add(const std::shared_ptr<CR::Object> &obj){
    if(obj->world != NULL || obj->id != CR::OBJ_ID_INVALID){
        std::string errmsg = CR::String::format("World[%i] failed to add object %i[%p]: it belongs to another world or wasn't properly destroyed", this->wId, obj->id, obj.get());
        this->auditBacklog.push_back(this->createFrame(errmsg));
        CR::log("%s\n", errmsg.c_str());
        return CR::OBJ_ID_INVALID;
    }
    for(unsigned i = 0; i < this->objects.size(); ++i){
        if(objects[i]->id == obj->id || objects[i].get() == obj.get()){          
            std::string errmsg = CR::String::format("World[%i] failed to add object %i[%p]: it already exists in it", this->wId, objects[i]->id, objects[i].get());
            this->auditBacklog.push_back(this->createFrame(errmsg));
            CR::log("%s\n", errmsg.c_str());
            return CR::OBJ_ID_INVALID;
        }
    }
    obj->id = genObjId();
    obj->world = this;
    obj->destroyed = false;
    CR::log("World[%i] added object[%i] at <%i,%i>\n", this->wId, obj->id);
    auto frame = this->createFrame(FrameType::OBJECT_CREATED);
    frame->affEnt.push_back(obj->id);
    frame->data.write(&obj->sigType, sizeof(T_OBJSIG));
    frame->data.write(&obj->loc->index, sizeof(T_WORLDPOS));
    frame->data.write(&obj->loc->level, sizeof(T_WORLDPOS));
    this->auditBacklog.push_back(frame);
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
        std::string errmsg = CR::String::format("World[%i] failed to destroy object %i[%p]: it's not part of it", this->wId, id);
        this->auditBacklog.push_back(this->createFrame(errmsg));
        CR::log("%s\n", errmsg.c_str());        
        return false;
    }
    auto obj = get(id);
    if(obj.get() == NULL){
        std::string errmsg = CR::String::format("[FATAL ERROR] World[%i] failed to destroy object %i[%p]: this object is null", this->wId, id);
        this->auditBacklog.push_back(this->createFrame(errmsg));
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