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
                static std::string str(unsigned type){
                    switch(type){
                        case ASCII: { return "ASCII"; };
                        case UTF8: { return "UTF8"; };
                    }
                }
            };

            namespace FontStyleType {
                enum FontStyleType : unsigned {
                    SOLID,
                    OUTLINE_ONLY,
                    SOLID_OUTLINED,
                    SOLID_SHADOWED
                };
            }        
    
            struct FontGlyph {
                std::string glyph;
	            CR::Vec2<unsigned> coors;
	            CR::Vec2<unsigned> size;
	            CR::Vec2<unsigned> orig;
	            CR::Vec2<unsigned> index;
            };

            struct FontStyle {
                unsigned type;
                unsigned encoding;
                unsigned size;
                float outlineThickness;
                FontStyle(unsigned size){
                    this->size = size;
                    encoding = CR::Gfx::FontEncondig::ASCII;
                    outlineThickness = 0.0f;
                }
                FontStyle(unsigned size, unsigned encoding, float thickness){
                    this->size = size;
                    this->encoding = encoding;
                    this->outlineThickness = thickness;
                }                
                FontStyle(){
                    size = 16;
                    encoding = CR::Gfx::FontEncondig::ASCII;
                    outlineThickness = 0.0f;
                }                
            };

            struct FontResource : CR::Rsc::Resource {
                std::vector<int> textureAtlas;
                std::unordered_map<std::string, CR::Gfx::FontGlyph> glyphMap;
                unsigned encoding;
                unsigned size; // in pixels;
                float outThickness;
                FontResource() : Resource() {
                    rscType = CR::Rsc::ResourceType::FONT;
                    srcType = CR::Rsc::SourceType::FILE;
                    encoding = FontEncondig::ASCII;
                }
                void unload();
            };         

            struct Font : CR::Rsc::Proxy {
                Font(const std::string &path, const CR::Gfx::FontStyle &style = CR::Gfx::FontStyle(16, CR::Gfx::FontEncondig::ASCII, 1.0f));
                Font();                
                bool load(const std::string &path, const CR::Gfx::FontStyle &style = CR::Gfx::FontStyle(16, CR::Gfx::FontEncondig::ASCII, 1.0f));
                void unload();
                CR::Gfx::FontResource *getRsc(){
                    return static_cast<CR::Gfx::FontResource*>(rsc.get());
                }                
            };   
        }     

    }

#endif