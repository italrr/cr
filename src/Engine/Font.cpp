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
	// solid makes a base with the exact same level per channel
	void solid(int w, int h, unsigned char* src){
		int channels = 4;
		this->w = w;
		this->h = h;
		size = w * h * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		for(int i = 0; i < w*h; ++i){
			buffer[i*4 + 0] = src[i];
			buffer[i*4 + 1] = src[i];
			buffer[i*4 + 2] = src[i];
			buffer[i*4 + 3] = src[i];
		}
	}    
	// strong makes a base with strong opposite (> 0 -> 0, == 0 -> 255)
	void strong(int w, int h, unsigned char* src){
		int channels = 4;
		this->w = w;
		this->h = h;
		size = w * h * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		for(int i = 0; i < w*h; ++i){
			unsigned char v = src[i] > 0 ? 0 : 255;
			buffer[i*4 + 0] = v;
			buffer[i*4 + 1] = v;
			buffer[i*4 + 2] = v;
			buffer[i*4 + 3] = src[i];
		}
	}    
	// bliz paints over a strong base
	void blitz(int w, int h, unsigned char* src){
		// blitz expects a smaller bitmap
		int offsetx = (this->w - w) / 2;
		int offsety = (this->h - h) / 2;
		for(int y = 0; y < h; ++y){
			for(int x = 0; x < w; ++x){
				int i = y * w  + x; //src
				int j = (y + offsety)  * this->w + x + offsetx; // target
				buffer[j*4 + 0] = src[i];
				buffer[j*4 + 1] = src[i];
				buffer[j*4 + 2] = src[i];
				// buffer[j*4 + 3] = (buffer[j*4 + 3] + src[i]); // disregard alpha			
			}
		}
	}    
	void clear(){
		if(buffer != NULL){
			delete buffer;
		}
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

static void renderGlyph(SingleGlyph *glyph, const CR::Gfx::FontStyle &style){
    switch(style.type){
        case CR::Gfx::FontStyleType::SOLID: {
            FT_Glyph_To_Bitmap(&glyph->glyph, FT_RENDER_MODE_NORMAL, NULL, true);
            FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
            glyph->solid(bitmap->bitmap.width, bitmap->bitmap.rows, bitmap->bitmap.buffer);
        } break;
        case CR::Gfx::FontStyleType::OUTLINE_ONLY: {
			FT_Glyph_To_Bitmap(&glyph->stroke, FT_RENDER_MODE_NORMAL, NULL, true);
			FT_BitmapGlyph bitmapStroke = reinterpret_cast<FT_BitmapGlyph>(glyph->stroke);     
            glyph->solid(bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows, bitmapStroke->bitmap.buffer);
        } break;
        case CR::Gfx::FontStyleType::SOLID_OUTLINED: {
            FT_Glyph_To_Bitmap(&glyph->glyph, FT_RENDER_MODE_NORMAL, NULL, true);
            FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
			FT_Glyph_To_Bitmap(&glyph->stroke, FT_RENDER_MODE_NORMAL, NULL, true);
			FT_BitmapGlyph bitmapStroke = reinterpret_cast<FT_BitmapGlyph>(glyph->stroke);                        
			glyph->strong(bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows, bitmapStroke->bitmap.buffer);
			glyph->blitz(bitmap->bitmap.width, bitmap->bitmap.rows, bitmap->bitmap.buffer);	
        } break;

        case CR::Gfx::FontStyleType::SOLID_SHADOWED: {

        } break;                        
    }
}

static void genAtlas(std::shared_ptr<GlyphMapping> &mapping){

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



    CR::log("[GFX] Loaded Font %s | Size %iPX | Encoding %s\n", path.c_str(), rscFont->size, CR::Gfx::FontEncondig::str(rscFont->encoding).c_str());


    FT_Done_FreeType(library);
}

void CR::Gfx::Font::unload(){

}