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
                        case UTF8: { return "UTF8"; };
                        case ASCII:
                        default: { return "ASCII"; };
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
                unsigned glyph;
	            CR::Vec2<unsigned> coors;
	            CR::Vec2<unsigned> size;
	            CR::Vec2<unsigned> orig;
	            CR::Vec2<unsigned> index;
                FontGlyph(){
                    glyph = 0;
                }
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
                    type = FontStyleType::SOLID;
                }
                FontStyle(unsigned size, unsigned encoding, float thickness, unsigned type){
                    this->size = size;
                    this->encoding = encoding;
                    this->outlineThickness = thickness;
                    this->type = type;
                }                
                FontStyle(){
                    size = 16;
                    encoding = CR::Gfx::FontEncondig::ASCII;
                    outlineThickness = 0.0f;
                    type = FontStyleType::SOLID;
                }                
            };

            struct FontResource : CR::Rsc::Resource {
                unsigned atlas;
                std::unordered_map<unsigned, CR::Gfx::FontGlyph> glyphMap;
                CR::Gfx::FontStyle style;
                unsigned vertAdvance;
                unsigned advanceX;
                unsigned horiBearingY;
                FontResource() : Resource() {
                    rscType = CR::Rsc::ResourceType::FONT;
                    srcType = CR::Rsc::SourceType::FILE;
                    style = CR::Gfx::FontStyle(16);
                }
                void unload();
            };         

            struct Font : CR::Rsc::Proxy {
                Font(const std::string &path, const CR::Gfx::FontStyle &style = CR::Gfx::FontStyle(16, CR::Gfx::FontEncondig::ASCII, 1.0f, FontStyleType::SOLID));
                Font();                
                bool load(const std::string &path, const CR::Gfx::FontStyle &style = CR::Gfx::FontStyle(16, CR::Gfx::FontEncondig::ASCII, 1.0f, FontStyleType::SOLID));
                void unload();
                CR::Gfx::FontResource *getRsc(){
                    return static_cast<CR::Gfx::FontResource*>(rsc.get());
                }                
            };   
        }     

    }

#endif