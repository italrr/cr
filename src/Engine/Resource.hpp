#ifndef CR_RESOURCES_HPP
    #define CR_RESOURCES_HPP

    #include <unordered_map>
    #include <memory>
    #include <mutex>

    #include "Types.hpp"
    #include "Result.hpp"
    
    #include "Job.hpp"

    namespace CR {
        
        namespace Rsc {

            struct Index {
                std::string hash;
                std::string fname;
                std::string fpath;
                std::string path; //abs
                std::string format;
                size_t size;
                bool loaded;
                std::vector<std::string> tags;
                void read(const std::string &path);
                bool isIt(const std::string &tag);
                void autotag();
                Index(){
                    this->loaded = false;
                }
            };            

            namespace SourceType {
                enum SourceType : unsigned {
                    FILE,
                    MEMORY
                };
            };

            namespace ResourceType {
                enum ResourceType : unsigned {
                    BINARY,
                    JSON,
                    TEXT,
                    FONT,
                    SHADER,
                    TEXTURE,
                    MODEL,
                    FRAMEBUFFER,
                    NONE
                };
            }

            namespace AllocationResult {
                enum AllocationResult : unsigned {
                    PROXY,
                    ANEW,
                    NONE
                };                  
            };

            struct Resource {
                std::shared_ptr<CR::Rsc::Index> file;
                
                bool rscLoaded;
                unsigned rscType;
                unsigned srcType;
                unsigned rscId;

                Resource(){
                    this->file = std::shared_ptr<CR::Rsc::Index>(new Index());
                    this->rscLoaded = false;
                    this->rscType = CR::Rsc::ResourceType::NONE;
                    this->srcType = CR::Rsc::SourceType::MEMORY;
                }

                virtual void unload(){
                    this->rscLoaded = false;
                }
            };


            struct Proxy {
                std::shared_ptr<CR::Rsc::Resource> rsc;
                Proxy(){
                    this->rsc = std::make_shared<CR::Rsc::Resource>(CR::Rsc::Resource());
                }
                ~Proxy(){
                    
                }
                unsigned findAllocByPath(const std::string &path);
                bool allocate(const std::shared_ptr<CR::Rsc::Resource> &rsc);
            };

        }
    }

#endif