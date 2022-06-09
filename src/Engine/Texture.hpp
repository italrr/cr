#ifndef CR_TEXTURE_HPP
    #define CR_TEXTURE_HPP

    #include "Types.hpp"
    #include "Resource.hpp"
    
    namespace CR {

        namespace Gfx {

            struct TextureResource : CR::Rsc::Resource {
                int textureId;
                Vec2<int> size;
                int channels;
                TextureResource() : Resource() {
                    rscType = CR::Rsc::ResourceType::TEXTURE;
                    srcType = CR::Rsc::SourceType::FILE;
                    textureId = 0;
                }
                void unload();
            };         

            struct Texture : CR::Rsc::Proxy {
                bool load(const std::string &path);
                Texture(const std::string &path);
                Texture();
                void unload();
                std::shared_ptr<CR::Gfx::TextureResource> getRsc(){
                    return std::static_pointer_cast<CR::Gfx::TextureResource>(rsc);
                }                
            };

        }
    }


#endif