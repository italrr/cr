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
	            CR::Vec2<float> coors;
	            CR::Vec2<float> size;
	            CR::Vec2<float> orig;
	            CR::Vec2<float> index;
                CR::Vec2<unsigned> bmpSize;
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

            namespace TextAlignType {
                enum TextAlignType : unsigned {
                    LEFT,
                    CENTER,
                    RIGHT
                };
            }

            struct TextRenderOpts {
                CR::Color outline;
                CR::Color fill;
                bool autobreak;
                float maxWidth; // for auto break
                unsigned alignment;
                float spaceWidth;
                float lineHeight;                
                float horBearingBonus;
                TextRenderOpts(){
                    outline.set(1.0f, 1.0f, 1.0f, 1.0f);
                    fill.set(0.0f, 0.0f, 0.0f, 1.0f);
                    autobreak = false;
                    maxWidth = 0;
                    alignment = TextAlignType::LEFT;
                    spaceWidth = 0; // 0 uses default
                    lineHeight = 0; // 0 used default
                    horBearingBonus = 0;
                }
                TextRenderOpts(const CR::Color &fill){
                    this->fill = fill;
                    autobreak = false;
                    maxWidth = 0;
                    alignment = TextAlignType::LEFT;
                    spaceWidth = 0; // 0 uses default
                    lineHeight = 0; // 0 used default
                    horBearingBonus = 0;                    
                }
                TextRenderOpts(const CR::Color &fill, const CR::Color &outline){
                    this->fill = fill;
                    this->outline = outline;
                    autobreak = false;
                    maxWidth = 0;
                    alignment = TextAlignType::LEFT;
                    spaceWidth = 0; // 0 uses default
                    lineHeight = 0; // 0 used default
                    horBearingBonus = 0;                    
                }                
            };

            struct FontResource : CR::Rsc::Resource {
                unsigned atlas;
                CR::Vec2<unsigned> atlasSize;
                std::unordered_map<unsigned, CR::Gfx::FontGlyph> glyphMap;
                std::unordered_map<unsigned, unsigned> ASCIITrans;
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
                float getWidth(const std::string &str);
                float getHeight();
                CR::Gfx::FontResource *getRsc(){
                    return static_cast<CR::Gfx::FontResource*>(rsc.get());
                }                
            };   
            
        }     

    }

#endif