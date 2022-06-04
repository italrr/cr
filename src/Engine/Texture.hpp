#ifndef CR_TEXTURE_HPP
    #define CR_TEXTURE_HPP

    #include "Types.hpp"
    #include "Resource.hpp"
    
    namespace CR {

        namespace Gfx {

            struct Texture : CR::Resource::Resource {
                int textureId;
                Vec2<int> size;
                int channels;
                Texture(){
                    rscType = CR::Resource::ResourceType::TEXTURE;
                    textureId = 0;
                }
                std::shared_ptr<CR::Result> unload();
                std::shared_ptr<CR::Result> load(const std::shared_ptr<CR::Indexing::Index> &file);
            };            

        }     

        static inline std::shared_ptr<CR::Gfx::Texture> qLoadTexture(const std::string &path){
            auto texture = std::make_shared<CR::Gfx::Texture>(CR::Gfx::Texture()); 
            auto indexer = CR::getIndexer();
            auto mgnr = CR::getResourceMngr();
            auto textureFile = indexer->findByPath(path);

            if(textureFile.get() == NULL){
                CR::log("Failed to load texture '%s': Doesn't exist or wasn't indexed\n", path.c_str());
                return std::shared_ptr<CR::Gfx::Texture>(NULL);
            }


            auto result = mgnr->load(textureFile, texture);
            
            if(!result->isSuccessful()){
                CR::log("Failed to load texture '%s': %s\n", path.c_str(), result->msg.c_str());
                return std::shared_ptr<CR::Gfx::Texture>(NULL);                
            }

            return texture;
        }


    }


#endif