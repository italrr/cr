#include <pthread.h>
#include <vector>
#include <map>

#include "Log.hpp"
#include "Job.hpp"
#include "Tools.hpp"


/*

    Scheduler

*/

struct _Job {
    int id;
    int hooked;
    int lead;
    _Job(){
        id = 0;
        hooked = 0;
    }
    pthread_t thread;
    std::shared_ptr<CR::Job> handle;
    std::function<void(CR::Job &ctx)> funct;
};
static std::map<int, _Job> pool =  {};

static void *thread_job(void *ctx){
    _Job *job = static_cast<_Job*>(ctx);
    while(job->handle->status == CR::JobStatus::Waiting){
        CR::sleep(16);
    }
    if(job->handle->status != CR::JobStatus::Running){
        pthread_exit(0);
        return NULL;
    }
    auto &spec = job->handle->spec;
    // std::vector<std::shared_ptr<CR::PiggybackJob>> backlog;
    // run onStart
    job->handle->initTime = CR::ticks();
    std::unique_lock<std::mutex> lkstart(job->handle->accesMutex);
    job->handle->onStart->lambda(*job->handle.get());
    if(job->handle->onStart->hasNext()){
        job->handle->backlog.push_back(job->handle->onStart->next);
    }
    lkstart.unlock();      
    do {
        // run backlog
        std::unique_lock<std::mutex> lk(job->handle->accesMutex);
        if(job->handle->backlog.size() > 0){
            std::vector<std::shared_ptr<CR::PiggybackJob>> backlog;
            for(int i = 0; i < job->handle->backlog.size(); ++i){
                job->handle->backlog[i]->lambda(*job->handle.get());
                if(job->handle->backlog[i]->hasNext()){
                    backlog.push_back(job->handle->backlog[i]->next);
                }
            }
            job->handle->backlog = backlog;
        }
        lk.unlock();     
        // run thread   
        job->funct(*job->handle.get());
        if(spec.looped && !spec.lowLatency){
            CR::sleep(16);
        }
    } while (spec.looped && job->handle->status == CR::JobStatus::Running);
    // run onEnd
    std::unique_lock<std::mutex> lkend(job->handle->accesMutex);
    job->handle->onEnd->lambda(*job->handle.get());   
    if(job->handle->onEnd->hasNext()){
        job->handle->backlog.push_back(job->handle->onEnd->next);
    }     
    lkend.unlock();        
    job->handle->status = CR::JobStatus::Done;
    pthread_exit(0);
    return NULL;
}

struct _Scheduler {
    int startId;
    pthread_mutex_t genIdMutex;
    pthread_mutex_t poolMutex;
    uint64 lastTick;

    void init(){
        startId = CR::Math::random(25, 50);
        pthread_mutex_init(&genIdMutex, NULL);
        pthread_mutex_init(&poolMutex, NULL);
    }

    void end(){
        // stop everything and clean up
        pthread_mutex_lock(&poolMutex);
        CR::log("[JOB] stopping all jobs(%i)...\n", pool.size());
        for(auto &it : pool){
            it.second.handle->status = CR::JobStatus::Stopped;
        }
        for(auto &it : pool){
            if(it.second.handle->spec.threaded){
                pthread_join(it.second.thread, NULL); 
            }
        }        
        while(pool.size() > 0){ janitor(false); }
        pthread_mutex_unlock(&poolMutex);
        pthread_mutex_destroy(&genIdMutex); 
        pthread_mutex_destroy(&poolMutex);
        CR::log("[JOB] done\n");
    }

    std::shared_ptr<CR::Job> spawn(std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec){
        int id = generateId();
        std::shared_ptr<CR::Job> handle = std::shared_ptr<CR::Job>(new CR::Job());
        handle->id = id;
        handle->status = CR::JobStatus::Waiting;
        handle->spec = spec;
        _Job job = _Job();
        job.id = id;
        job.handle = handle;
        job.hooked = 0;
        job.lead = 0;
        job.funct = funct;
        pthread_mutex_lock(&poolMutex);
        pool[id] = job;
        if(spec.threaded){
            int rc  = pthread_create(&pool[id].thread, NULL, thread_job, (void*)&pool[id]);
            if (rc){
                CR::log("[JOB] attention: failed to spawn thread id '%i'. return code: '%i'\n", id, rc);
                handle->status = CR::JobStatus::Stopped;
            }            
        }
        pthread_mutex_unlock(&poolMutex);
        return handle;
    }


    std::shared_ptr<CR::Job> expect(std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec, const std::vector<std::shared_ptr<CR::Result>> &listeners){
        int id = generateId();
        std::shared_ptr<CR::Job> handle = std::shared_ptr<CR::Job>(new CR::Job());
        handle->id = id;
        handle->listeners = listeners;
        handle->status = CR::JobStatus::Waiting;
        handle->spec = spec;
        _Job job = _Job();
        job.id = id;
        job.handle = handle;
        job.hooked = 0;
        job.lead = 0;
        job.funct = funct;
        pthread_mutex_lock(&poolMutex);
        pool[id] = job;
        if(spec.threaded){
            int rc  = pthread_create(&pool[id].thread, NULL, thread_job, (void*)&pool[id]);
            if (rc){
                CR::log("[JOB] attention: failed to spawn thread id '%i'. return code: '%i'\n", id, rc);
                handle->status = CR::JobStatus::Stopped;
            }            
        }
        pthread_mutex_unlock(&poolMutex);
        return handle;
    }    


    std::vector<std::shared_ptr<CR::Job>> findJobs(const std::vector<std::string> &tags, int minmatch = 1){
        std::vector<std::shared_ptr<CR::Job>> jobs;
        pthread_mutex_lock(&poolMutex);
        int matches = 0;
        auto findMatches = [](const std::vector<std::string> &from, const std::vector<std::string> &tags){
            int n = 0;
            if(from.size() == 0){
                return n;
            }
            for(int i = 0; i < from.size(); ++i){
                for(int j = 0; j < tags.size(); ++j){
                    if(CR::String::toLower(from[i]) == CR::String::toLower(tags[j])){
                        ++n;
                    }
                }
            }
            return n;
        };
        for(auto &it : pool){
            auto &j = it.second.handle;
            int nmatches = findMatches(j->spec.tags, tags);
            if(nmatches >= minmatch){
                jobs.push_back(j);
            }
        }
        pthread_mutex_unlock(&poolMutex);        
        return jobs;
    }

    std::shared_ptr<CR::Job> findJob(const std::vector<std::string> &tags, int minmatch = 1){
        auto job = std::shared_ptr<CR::Job>(NULL);
        pthread_mutex_lock(&poolMutex);
        int matches = 0;
        auto findMatches = [](const std::vector<std::string> &from, const std::vector<std::string> &tags){
            int n = 0;
            if(from.size() == 0){
                return n;
            }
            for(int i = 0; i < from.size(); ++i){
                for(int j = 0; j < tags.size(); ++j){
                    if(CR::String::toLower(from[i]) == CR::String::toLower(tags[j])){
                        ++n;
                    }
                }
            }
            return n;
        };
        for(auto &it : pool){
            auto &j = it.second.handle;
            int nmatches = findMatches(j->spec.tags, tags);
            if(nmatches >= minmatch){
                job = j;
                break;
            }
        }
        pthread_mutex_unlock(&poolMutex);        
        return job;
    }

    std::shared_ptr<CR::Job> findJob(int id){
        auto job = std::shared_ptr<CR::Job>(NULL);
        pthread_mutex_lock(&poolMutex);
        auto it = pool.find(id);
        if(it != pool.end()){
            job = it->second.handle;
        }
        pthread_mutex_unlock(&poolMutex);        
        return job;
    }

    std::shared_ptr<CR::Job> hook(int lead, std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec){
        int id = generateId();
        std::shared_ptr<CR::Job> handle = std::shared_ptr<CR::Job>(new CR::Job());
        handle->id = id;
        handle->status = CR::JobStatus::Waiting;
        handle->spec = spec;
        _Job job = _Job();
        job.id = id;
        job.handle = handle;
        job.hooked = 0;
        job.lead = 0;
        job.funct = funct;
        pthread_mutex_lock(&poolMutex);
        pool[id] = job;
        auto &jref = pool[id];
        auto it = pool.find(lead);
        if(it != pool.end()){
            it->second.hooked = id;
            jref.lead = lead;
            handle->status = CR::JobStatus::Waiting;
            handle->spec.payload = it->second.handle->spec.payload; // inherit payload from lead
        }else{
            CR::log("[JOB] failed to hook job to non-existing job id %i. starting it right away instead...\n", lead);
        }
        if(spec.threaded){
            int rc  = pthread_create(&jref.thread, NULL, thread_job, (void*)&pool[id]);
            if (rc){
                CR::log("[JOB] attention: failed to spawn thread id '%i'. return code: '%i'\n", id, rc);
                handle->status = CR::JobStatus::Stopped;
            }            
        }

        pthread_mutex_unlock(&poolMutex);
        return handle;
    }

    void stop(CR::Job *job){
        job->status = CR::JobStatus::Stopped;     
    }

    void janitor(bool lock = true){
        if(lock) pthread_mutex_lock(&poolMutex);
        std::vector<int> toRemove;
        for(auto &it : pool){
            auto &job = it.second;
            auto *handle = it.second.handle.get();
            switch(handle->status){
                case CR::JobStatus::Done:
                case CR::JobStatus::Stopped: {
                    if(job.hooked != 0){
                        auto hit = pool.find(job.hooked);
                        if(hit != pool.end()){
                            hit->second.handle->status = CR::JobStatus::Running;
                            job.hooked = 0;
                        }
                    }
                    if(handle->spec.threaded){
                        pthread_join(it.second.thread, NULL); 
                    }                    
                    toRemove.push_back(handle->id);
                    handle->id = 0;
                } break;
                case CR::JobStatus::Waiting: {
                    auto it = pool.find(job.lead);
                    if(it == pool.end()){
                        job.handle->status = CR::JobStatus::Running;
                    }
                } break;
                case CR::JobStatus::Running: {
                    if(handle->spec.threaded){
                        break;
                    }
                    job.funct(*handle);
                    if(!handle->spec.looped){
                        handle->status = CR::JobStatus::Done;
                        toRemove.push_back(handle->id);
                        handle->id = 0;
                    }
                } break;
            }
        }
        for(int i = 0; i < toRemove.size(); ++i){
            pool.erase(toRemove[i]);
        }
        if(lock) pthread_mutex_unlock(&poolMutex);       
    }

    int generateId(){
        pthread_mutex_lock(&genIdMutex);
        int nid = ++startId;
        pthread_mutex_unlock(&genIdMutex);
        return nid;
    }

};
static _Scheduler sch = _Scheduler();

/*

    Job

*/

void CR::Job::stop(){
    sch.stop(this);
}

CR::Job::Job(){
    onEnd = std::make_shared<CR::PiggybackJob>(CR::PiggybackJob());
    onStart = std::make_shared<CR::PiggybackJob>(CR::PiggybackJob());
    succDeps = false;
}

std::shared_ptr<CR::PiggybackJob> CR::Job::addBacklog(const std::function<void(CR::Job &ctx)> &lambda){
    std::unique_lock<std::mutex> lk(accesMutex);
    auto pgb = std::make_shared<CR::PiggybackJob>(CR::PiggybackJob());
    pgb->set(lambda);
    this->backlog.push_back(pgb);
    lk.unlock();    
    return pgb;
}

std::shared_ptr<CR::PiggybackJob> CR::Job::setOnEnd(const std::function<void(CR::Job &ctx)> &onEnd){
    std::unique_lock<std::mutex> lk(accesMutex);
    auto pgb = std::make_shared<CR::PiggybackJob>(CR::PiggybackJob());
    pgb->set(onEnd);
    this->onEnd = pgb;
    lk.unlock();   
    return pgb; 
}

std::shared_ptr<CR::PiggybackJob> CR::Job::setOnStart(const std::function<void(CR::Job &ctx)> &onStart){
    std::unique_lock<std::mutex> lk(accesMutex);
    auto pgb = std::make_shared<CR::PiggybackJob>(CR::PiggybackJob());
    pgb->set(onStart);
    this->onStart = pgb;
    lk.unlock();   
    return pgb; 
}

std::shared_ptr<CR::Job> CR::Job::hook(std::function<void(CR::Job &ctx)> funct, bool threaded){
    CR::JobSpec spec;
    spec.looped = false;
    spec.threaded = threaded;
    spec.lowLatency = false;
    return sch.hook(id, funct, spec);
}

std::shared_ptr<CR::Job> CR::Job::hook(std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec){
    return sch.hook(id, funct, spec);
}

std::shared_ptr<CR::Job> CR::Job::hook(std::function<void(CR::Job &ctx)> funct, bool threaded, bool looped, bool lowLatency){
    CR::JobSpec spec;
    spec.looped = looped;
    spec.threaded = threaded;
    spec.lowLatency = lowLatency;
    return sch.hook(id, funct, spec);
}

std::shared_ptr<CR::Job> CR::spawn(std::function<void(CR::Job &ctx)> funct, bool threaded){
    CR::JobSpec spec;
    spec.looped = false;
    spec.threaded = threaded;
    spec.lowLatency = false;
    return spawn(funct, spec);
}

std::shared_ptr<CR::Job> CR::spawn(std::function<void(CR::Job &ctx)> funct, bool threaded, bool looped, bool lowLatency){
    CR::JobSpec spec;
    spec.looped = looped;
    spec.threaded = threaded;
    spec.lowLatency = lowLatency;
    return spawn(funct, spec);
}

std::shared_ptr<CR::Job> CR::expect(const std::vector<std::shared_ptr<CR::Result>> &results, std::function<void(CR::Job &ctx)> funct, bool lowLatency){
    CR::JobSpec spec;
    spec.looped = true;
    spec.threaded = true;
    spec.lowLatency = lowLatency;
    return sch.expect([funct](CR::Job &ctx){
        int t = 0;
        int allsucc = 0;
        for(int i = 0 ; i < ctx.listeners.size(); ++i){
            if(ctx.listeners[i]->done){
                ++t;
            }
            if(ctx.listeners[i]->isSuccessful()){
                ++allsucc;
            }
        }
        if(t == ctx.listeners.size()){
            if(allsucc == ctx.listeners.size()){
                ctx.succDeps = true;
            }
            funct(ctx);
            ctx.stop();
        }
        return;
    }, spec, results);
}

std::shared_ptr<CR::Job> CR::spawn(std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec){
    return sch.spawn(funct, spec);
}

std::vector<std::shared_ptr<CR::Job>> CR::findJobs(const std::vector<std::string> &tags, int minmatch){
    return sch.findJobs(tags, minmatch);
}

std::shared_ptr<CR::Job> CR::findJob(const std::vector<std::string> &tags, int minmatch){
    return sch.findJob(tags, minmatch);
}

std::shared_ptr<CR::Job> CR::findJob(int id){
    return sch.findJob(id);
}

/*

    CR module interface

*/
void __CR_init_job(){
    sch.init();
}

void __CR_end_job(){
    sch.end();
    // this is hanging on windows...
    // TODO: check it out later
    // pthread_exit(NULL);
}

void __CR_update_job(){
    sch.janitor();
}