#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
#include FT_FREETYPE_H

#include "Font.hpp"

#define ENCODING_ASCII_RANGE_MIN 32
#define ENCODING_ASCII_RANGE_MAX 128

void CR::Gfx::FontResource::unload(){

}





CR::Gfx::Font::Font(const std::string &path, const CR::Gfx::FontStyle &style){
    load(path, style);
}

CR::Gfx::Font::Font(){

}

struct SingleGlyph {
	int w, h;
	int bw, bh;
	unsigned char *buffer; // bitmap
	size_t size;
    FT_UInt cindex;
    FT_Glyph glyph;
    FT_Glyph stroke;
    FT_UInt symbol;
    unsigned atlasIndex;
    SingleGlyph(){
        buffer = NULL;
        atlasIndex = 0;
    }
};

struct GlyphMapping {
    std::unordered_map<FT_UInt, std::shared_ptr<SingleGlyph>> map;
    std::vector<FT_UInt> list;
};

static std::shared_ptr<GlyphMapping> genMapping(const std::string &path, const CR::Gfx::FontStyle &style, FT_Library &library){
    auto mapping = std::make_shared<GlyphMapping>(GlyphMapping());
    
	FT_Face face;
    FT_Stroker stroker;

	if(FT_New_Face(library, path.c_str(), 0, &face)) {
		CR::log("[GFX] Font::genMapping: Failed loading font '%s: FT_New_Face\n", path.c_str());
		FT_Done_FreeType(library);
		return mapping;
	}	

	if(FT_Set_Char_Size(face, 0, style.size << 6, 96, 96)){
		CR::log("[GFX] Font::genMapping: Failed loading font '%s': FT_Set_Char_Size\n", path.c_str());
		FT_Done_FreeType(library);		
		return mapping;
	}
    
    FT_Stroker_New(library, &stroker);
    FT_Stroker_Set(stroker, (int)(style.outlineThickness * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    switch(style.encoding){
        case CR::Gfx::FontEncondig::ASCII: {
            for(unsigned i = ENCODING_ASCII_RANGE_MIN; i < ENCODING_ASCII_RANGE_MAX; i++){    
                std::shared_ptr<SingleGlyph> glSingle = std::make_shared<SingleGlyph>(SingleGlyph());
                
                FT_UInt cindex = FT_Get_Char_Index(face, i);
                mapping->list.push_back(cindex);
                
                glSingle->symbol = cindex;
                FT_Glyph glyph, stroke;

                // Load glyph
                if(FT_Load_Glyph(face, cindex, FT_LOAD_DEFAULT)){
                    CR::log("[GFX] Font::genMapping: Failed to load glyph '%c' for font '%s'\n", (char)i, path.c_str());
                    continue;
                }

                // Get glyph (base)
                if(FT_Get_Glyph(face->glyph, &glyph)){
                    CR::log("[GFX] Font::genMapping: Failed to load glyph '%c' for font '%s'\n", (char)i, path.c_str());
                    continue;
                }

                // Get glyph (stroke)
                if(FT_Get_Glyph(face->glyph, &stroke)){
                    CR::log("[GFX] Font::genMapping: Failed to load glyph '%c' for font '%s'\n", (char)i, path.c_str());
                    continue;
                }        
                FT_Glyph_StrokeBorder(&stroke, stroker, 0, 1);	       

                glSingle->glyph = glyph;
                glSingle->stroke = stroke;

                mapping->map[cindex] = glSingle;
            }
        } break;
        default:
        case CR::Gfx::FontEncondig::UTF8: {
            CR::log("[GFX] Font::genMapping: Failed to generate atlas for 'UTF8': Unimplemented\n");
        } break;            
    }

    FT_Done_Face(face);
    FT_Stroker_Done(stroker);

    return mapping;
}

static void genAtlas(){

}

bool CR::Gfx::Font::load(const std::string &path, const CR::Gfx::FontStyle &style){
    // Skip checking for already loaded resources

    // auto result = this->findAllocByPath(path);
    // if(result == CR::Rsc::AllocationResult::PROXY){
    //     return true;
    // } 

    if(!CR::File::exists(path)){
        CR::log("[GFX] Font::load: failed to load font '%s': file doesn't exist\n", path.c_str());
        return false;        
    }    

    auto rscFont = std::make_shared<CR::Gfx::FontResource>(CR::Gfx::FontResource());
    auto rsc = std::static_pointer_cast<CR::Gfx::FontResource>(rscFont);

    allocate(rscFont);

    rscFont->encoding = style.encoding;
    rscFont->size = style.size;
    rscFont->file->read(path);


    FT_Library library;     

	if(FT_Init_FreeType(&library)){
		CR::log("[GFX] Font::load: Failed to start FreeType: FT_Init_FreeType\n");
		return false;
	}

    auto mapping = genMapping(path, style, library);



    CR::log("[GFX] Loaded Font %s | Size %ipx | Encoding %s\n", path.c_str(), rscFont->size, CR::Gfx::FontEncondig::str(rscFont->encoding));


    FT_Done_FreeType(library);
}

void CR::Gfx::Font::unload(){

}