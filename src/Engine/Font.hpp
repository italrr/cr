#ifndef CR_FONT_HPP
    #define CR_FONT_HPP

    #include "Types.hpp"
    #include "Resource.hpp"

    namespace CR {
        namespace Gfx {

            namespace FontEncondig {
                enum FontEnconding : unsigned {
                    ASCII,
                    UTF8
                };
            };

            struct FontGlyph {

            };

            struct FontResource : CR::Rsc::Resource {
                std::vector<int> textureAtlas;
                unsigned size; // in pixels;
                FontResource() : Resource() {
                    rscType = CR::Rsc::ResourceType::FONT;
                    srcType = CR::Rsc::SourceType::FILE;
                }
                void unload();
            };         

            struct Font : CR::Rsc::Proxy {
                Font(const std::string &path, unsigned);
                Font();                
                bool load(const std::string &path);
                bool load(const std::string &path, const std::string &mask);
                void unload();
                CR::Gfx::FontResource *getRsc(){
                    return static_cast<CR::Gfx::FontResource*>(rsc.get());
                }                
            };   
        }     

    }

#endif