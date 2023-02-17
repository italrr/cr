#ifndef CR_MODEL_HPP
    #define CR_MODEL_HPP

    #include "Types.hpp"
    #include "Resource.hpp"
    #include "Graphics.hpp"

    namespace CR {
        namespace Gfx {

            struct ModelResource : CR::Rsc::Resource {
                std::vector<std::shared_ptr<CR::Gfx::Mesh>> mesh;
                CR::Mat<4,4, float> gInvTrans;               
                ModelResource() : Resource() {
                    rscType = CR::Rsc::ResourceType::MODEL;
                    srcType = CR::Rsc::SourceType::FILE;    
                }
                void unload();
            };         

            struct Model : CR::Rsc::Proxy {
                Model(const std::string &path);
                Model();                
                bool load(const std::string &path);
                void unload();
                CR::Gfx::ModelResource *getRsc(){
                    return static_cast<CR::Gfx::ModelResource*>(rsc.get());
                }                
            }; 
        }
    }
#endif