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
        
        namespace Rsc {

            struct Index {
                std::string hash;
                std::string fname;
                std::string fpath;
                std::string path; //abs
                size_t size;
                std::vector<std::string> tags;
                void read(const std::string &path);
                bool isIt(const std::string &tag);
                void autotag();
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
                    SHADER,
                    TEXTURE,
                    MODEL,
                    FRAMEBUFFER,
                    NONE
                };
            }

            struct Resource {
                std::shared_ptr<CR::Rsc::Index> file;
                std::mutex accesMutex;
                
                bool rscLoaded;
                unsigned rscType;
                unsigned srcType;
                unsigned rscId;

                Resource(){
                    rscType = ResourceType::NONE;
                    srcType = SourceType::MEMORY;
                    rscLoaded = false;
                    rscId = 0;
                    file = std::shared_ptr<CR::Rsc::Index>(new Index());
                }

                virtual bool load(){
                    return true;
                }

                virtual bool unload(){
                    return false;
                }

            
            };

        }
    }

#endif