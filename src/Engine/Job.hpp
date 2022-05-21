#ifndef CR_JOB_HPP
    #define CR_JOB_HPP

    #include <functional>
    #include <memory>

    #include "Types.hpp"
    #include "Tools.hpp"
    #include "Result.hpp"

    namespace CR {

        struct JobSpec {
            bool looped;
            bool lowLatency;
            bool threaded;
            std::shared_ptr<CR::SmallPacket> payload;
            std::vector<std::string> tags;
            std::shared_ptr<CR::Result> result;
            JobSpec(){
                this->looped = false;
                this->lowLatency = false;
                this->threaded = true;
                this->payload = std::make_shared<CR::SmallPacket>(CR::SmallPacket());
            }
            JobSpec(bool threaded, bool looped, bool lowLatency, const std::vector<std::string> &tags = {}){
                this->threaded = threaded;
                this->looped = looped;
                this->lowLatency = lowLatency;
                this->tags = tags;
                this->payload = std::make_shared<CR::SmallPacket>(CR::SmallPacket());
            }
            bool hasTag(const std::string &tag){
                for(int i = 0; i < this->tags.size(); ++i){
                    if(this->tags[i] == tag){
                        return true;
                    }
                }
                return false;
            }
        };

        enum JobStatus : uint8 {
            Stopped,
            Running,
            Waiting,
            Done
        };

        struct PiggybackJob {
            std::function<void(CR::Job &ctx)> lambda;
            std::shared_ptr<PiggybackJob> next;
            bool stale;
            PiggybackJob(){
                next = std::shared_ptr<PiggybackJob>(NULL);
                stale = false;
                lambda = [](CR::Job &ctx){
                    return;
                };
            }
            void set(const std::function<void(CR::Job &ctx)> &lambda){
                this->lambda = lambda;
            }
            bool hasNext(){
                return next.get() != NULL;
            }
            std::shared_ptr<PiggybackJob> hook(const std::function<void(CR::Job &ctx)> &lambda){
                this->next = std::make_shared<PiggybackJob>(PiggybackJob());
                this->next->lambda = lambda;
                return this->next; 
            }
        };

        struct Job {
            // do not change these
            int id;
            uint8 status;
            uint64 initTime;
            std::vector<std::shared_ptr<CR::Result>> listeners;
            bool succDeps;
            CR::JobSpec spec;
            std::mutex accesMutex;
            std::vector<std::shared_ptr<CR::PiggybackJob>> backlog;
            std::shared_ptr<CR::PiggybackJob> onEnd;
            std::shared_ptr<CR::PiggybackJob> onStart;
            // interfacing
            void stop();
            Job();     
            std::shared_ptr<CR::PiggybackJob> addBacklog(const std::function<void(CR::Job &ctx)> &lambda);   
            std::shared_ptr<CR::PiggybackJob> setOnEnd(const std::function<void(CR::Job &ctx)> &onEnd);     
            std::shared_ptr<CR::PiggybackJob> setOnStart(const std::function<void(CR::Job &ctx)> &onStart);
            std::shared_ptr<CR::Job> hook(std::function<void(CR::Job &ctx)> funct, bool threaded);
            std::shared_ptr<CR::Job> hook(std::function<void(CR::Job &ctx)> funct, bool threaded, bool looped, bool lowLatency);
            std::shared_ptr<CR::Job> hook(std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec);
        };

        std::vector<std::shared_ptr<CR::Job>> findJobs(const std::vector<std::string> &tags, int minmatch = 1);
        std::shared_ptr<CR::Job> findJob(const std::vector<std::string> &tags, int minmatch = 1);
        std::shared_ptr<CR::Job> findJob(int id);

        std::shared_ptr<CR::Job> spawn(std::function<void(CR::Job &ctx)> funct, bool threaded);
        std::shared_ptr<CR::Job> spawn(std::function<void(CR::Job &ctx)> funct, bool threaded, bool looped, bool lowLatency);
        std::shared_ptr<CR::Job> spawn(std::function<void(CR::Job &ctx)> funct, const CR::JobSpec &spec);

        std::shared_ptr<CR::Job> expect(const std::vector<std::shared_ptr<CR::Result>> &results, std::function<void(CR::Job &ctx)> funct, bool lowLatency = true);
    }
    

#endif