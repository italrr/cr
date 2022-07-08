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
                Texture(const std::string &path);
                Texture();                
                bool load(const std::string &path);
                bool load(const std::string &path, const std::string &mask);
                void unload();
                CR::Gfx::TextureResource *getRsc(){
                    return static_cast<CR::Gfx::TextureResource*>(rsc.get());
                }                
            };

        }
    }


#endif