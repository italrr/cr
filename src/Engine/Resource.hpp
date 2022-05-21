#ifndef CR_RESOURCES_HPP
    #define CR_RESOURCES_HPP

    #include <unordered_map>
    #include <memory>
    #include <mutex>

    #include "Types.hpp"
    #include "Result.hpp"
    
    #include "Indexer.hpp"
    #include "Job.hpp"

    namespace CR {
        
        namespace Resource {

            namespace ResourceType {
                enum ResourceType : int {
                    SHADER,
                    TEXTURE,
                    MODEL,
                    NONE
                };
            }

            struct Resource {
                std::mutex accesMutex;
                bool rscLoaded;
                int rscType;
                int rscId;
                std::shared_ptr<CR::Indexing::Index> file;
                Resource(){
                    rscType = ResourceType::NONE;
                    rscLoaded = false;
                }
                Resource(const Resource &rsc){
                    this->file = rsc.file;
                    this->rscType = rsc.rscLoaded;
                    this->rscId = rsc.rscId;
                    this->rscLoaded = rsc.rscLoaded;
                }
                void setId(int id){
                    std::unique_lock<std::mutex> lk(accesMutex);
                    this->rscId = id;
                    lk.unlock();
                }
                void setType(int type){
                    std::unique_lock<std::mutex> lk(accesMutex);
                    this->rscLoaded = type;
                    lk.unlock();
                }        
                void setLoaded(bool v){
                    std::unique_lock<std::mutex> lk(accesMutex);
                    this->rscLoaded = v;
                    lk.unlock();                    
                }
                void setFile(const std::shared_ptr<CR::Indexing::Index> &file){
                    std::unique_lock<std::mutex> lk(accesMutex);
                    this->file = file;
                    lk.unlock();                      
                }
                virtual std::shared_ptr<CR::Result> unload(){
                    return CR::makeResult(CR::ResultType::Success);
                }
                virtual std::shared_ptr<CR::Result> load(const std::shared_ptr<CR::Indexing::Index> &file){
                    return CR::makeResult(CR::ResultType::Success);
                }
            };

            struct ResourceManager {
                int lastId;
                std::mutex accesMutex;
                std::unordered_map<int, std::shared_ptr<Resource>> resources;
                std::shared_ptr<CR::Result> load(const std::string &name, const std::shared_ptr<Resource> &holder);
                std::shared_ptr<CR::Result> load(const std::shared_ptr<CR::Indexing::Index> &file, const std::shared_ptr<Resource> &holder);
                std::shared_ptr<CR::Resource::Resource> findByName(const std::string &name);
                std::shared_ptr<CR::Resource::Resource> findByHash(const std::string &hash);
                std::shared_ptr<CR::Resource::Resource> findById(int id);
                ResourceManager();
            };

        }
    }

#endif