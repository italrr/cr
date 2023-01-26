#include <cmath>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
#include FT_FREETYPE_H

#include "Graphics.hpp"
#include "Font.hpp"

#define ENCODING_ASCII_RANGE_MIN 32
#define ENCODING_ASCII_RANGE_MAX 128



struct SingleGlyph {
	int w, h;
	int bw, bh;
	unsigned char *buffer; // bitmap
	size_t size;
    unsigned symbol;

    CR::Vec2<unsigned> _coors;
    CR::Vec2<float> _size;
    CR::Vec2<int> _orig;      
    CR::Vec2<unsigned> _index;  


    void transferSpec(CR::Gfx::FontGlyph &to, const CR::Vec2<unsigned> &atlasSize){
        // Atlas coordinates
        to.coors.x =    static_cast<float>(this->_coors.x)  / static_cast<float>(atlasSize.x);
        to.coors.y =    static_cast<float>(this->_coors.y)  / static_cast<float>(atlasSize.y);
        to.index.x =    static_cast<float>(this->_index.x)   / static_cast<float>(atlasSize.x);
        to.index.y =    static_cast<float>(this->_index.y)   / static_cast<float>(atlasSize.y);
        // Top/Left
        to.orig.x =     this->_orig.x;
        to.orig.y =     this->_orig.y;
        // Size 
        to.size.x =     this->_size.x;
        to.size.y =     this->_size.y;
        // Bitmap Size
        to.bmpSize.x = this->_coors.x;
        to.bmpSize.y = this->_coors.y;
    }

    void readSpecs(FT_BitmapGlyph &glyph, FT_Face &face){
        this->_coors.x =    glyph->bitmap.width;
        this->_coors.y =    glyph->bitmap.rows;
        this->_orig.x =     glyph->left;
        this->_orig.y =     glyph->top;
        this->_size.x =     face->glyph->advance.x >> 6;
        this->_size.y =     face->glyph->metrics.horiBearingY >> 6;
    }

    SingleGlyph(){
        buffer = NULL;
    }
	// solid makes a base with the exact same level per channel
	void solid(int w, int h, unsigned char* src){
		int channels = 4;
		this->w = w;
		this->h = h;
		this->bw = w;
		this->bh = h;        
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
	void strong(int x, int y, int w, int h, int bw, int bh, unsigned char* src){
		int channels = 2;
		this->w = w;
		this->h = h;
		this->bw = bw == 0 ? w : bw;
		this->bh = bh == 0 ? h : bh;
		size = this->bw * this->bh * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		int offsetx = std::round((float)(this->w - w) / 2.0f) + x;
		int offsety = std::round((float)(this->h - h) / 2.0f) + y;		
		for(int y = 0; y < h; ++y){
			for(int x = 0; x < w; ++x){
				int i = y * w  + x;
				int j = (y + offsety)  * this->bw + x + offsetx;		
				buffer[j*channels + 1] = src[i];
			}
		}	
	}    
	// bliz paints over a strong base
	void blitz(int x, int y, int w, int h, int bw, int bh, unsigned char* src){
		// blitz expects a smaller bitmap
		int channels = 2;
		int offsetx = CR::Math::round((float)(this->w - w) / 2.0f) + x;
		int offsety = CR::Math::round((float)(this->h - h) / 2.0f) + y;
		for(int y = 0; y < h; ++y){
			for(int x = 0; x < w; ++x){
				int i = y * w  + x; //src
				int j = (y + offsety)  * (bw == 0 ? this->w : bw) + x + offsetx; // target
				// buffer[j*4 + 0] = src[i];
				// buffer[j*4 + 1] = src[i];
				// buffer[j*4 + 2] = src[i];
				// buffer[j*4 + 3] = (buffer[j*4 + 3] + src[i]); // disregard alpha			
				buffer[j*channels + 0] = src[i];
			}
		}
	}    
	void clear(){
		if(buffer != NULL){
			delete buffer;
		}
	}    
};



void CR::Gfx::FontResource::unload(){

}

CR::Gfx::Font::Font(const std::string &path, const CR::Gfx::FontStyle &style){
    load(path, style);
}

CR::Gfx::Font::Font(){

}


static void renderGlyph(FT_Face &face, FT_Glyph &glyph, FT_Glyph &stroke, SingleGlyph *sg, const CR::Gfx::FontStyle &style){
    switch(style.type){
        case CR::Gfx::FontStyleType::SOLID: {
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
            FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
            sg->solid(bitmap->bitmap.width, bitmap->bitmap.rows, bitmap->bitmap.buffer);
            sg->readSpecs(bitmap, face);         
        } break;
        case CR::Gfx::FontStyleType::OUTLINE_ONLY: {
			FT_Glyph_To_Bitmap(&stroke, FT_RENDER_MODE_NORMAL, NULL, true);
			FT_BitmapGlyph bitmapStroke = reinterpret_cast<FT_BitmapGlyph>(stroke);     
            sg->solid(bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows, bitmapStroke->bitmap.buffer);
            sg->readSpecs(bitmapStroke, face);
        } break;
        case CR::Gfx::FontStyleType::SOLID_OUTLINED: {
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
            FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
			FT_Glyph_To_Bitmap(&stroke, FT_RENDER_MODE_NORMAL, NULL, true);
			FT_BitmapGlyph bitmapStroke = reinterpret_cast<FT_BitmapGlyph>(stroke);   
			sg->strong(0, 0, bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows, 0, 0, bitmapStroke->bitmap.buffer);
			sg->blitz(0, 0, bitmap->bitmap.width, bitmap->bitmap.rows, 0, 0, bitmap->bitmap.buffer);	
            sg->readSpecs(bitmapStroke, face);
        } break;
        case CR::Gfx::FontStyleType::SOLID_SHADOWED: {

        } break;                        
    }
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

    rscFont->style = style;
    rscFont->file->read(path);

    FT_Library library; 
    FT_Face face;
    FT_Stroker stroker;

	if(FT_Init_FreeType(&library)){
		CR::log("[GFX] Font::load: Failed to start FreeType: FT_Init_FreeType\n");
		return false;
	}

	if(FT_New_Face(library, path.c_str(), 0, &face)) {
		CR::log("[GFX] Font::genMapping: Failed loading font '%s: FT_New_Face\n", path.c_str());
        return false;
	}	

	if(FT_Set_Char_Size(face, 0, style.size << 6, 96, 96)){
		CR::log("[GFX] Font::genMapping: Failed loading font '%s': FT_Set_Char_Size\n", path.c_str());	
        return false;
	}

    std::vector<FT_UInt> mapping;

    switch(style.encoding){
        case CR::Gfx::FontEncondig::ASCII: {
            for(unsigned i = ENCODING_ASCII_RANGE_MIN; i < ENCODING_ASCII_RANGE_MAX; ++i){
                auto cindex = FT_Get_Char_Index(face, i);
                mapping.push_back(cindex);
                rsc->ASCIITrans[i] = cindex;
            }
        } break;
        default:
        case CR::Gfx::FontEncondig::UTF8: {
            CR::log("[GFX] Font::load: UTF-8 is unimplemented\n");
        } break;         
    }

    unsigned maxHeight = 0;
    static const CR::Vec2<unsigned> maxSize = {8192, 8192};
    CR::Vec2<unsigned> expectedSize = {0, 0};
    CR::Vec2<unsigned> cursor = {0, 0};

    FT_Stroker_New(library, &stroker);
    FT_Stroker_Set(stroker, (int)(style.outlineThickness * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    rsc->vertAdvance = face->size->metrics.height >> 6;
    rsc->advanceX = face->glyph->advance.x >> 6;
    rsc->horiBearingY = face->glyph->metrics.horiBearingY >> 6;    

    FT_Glyph dummyg;
    auto dummid = FT_Get_Char_Index(face, 'A');
    FT_Load_Glyph(face, dummid, FT_LOAD_DEFAULT);
    FT_Get_Glyph(face->glyph, &dummyg);

    // Get Glyphs
    std::unordered_map<FT_UInt, std::shared_ptr<SingleGlyph>> sgs;
    for(unsigned i = 0; i < mapping.size(); ++i){
        FT_Glyph glyph, stroke;
        std::shared_ptr<SingleGlyph> sg = std::make_shared<SingleGlyph>(SingleGlyph());
        auto id = mapping[i];
        sg->symbol = i;
        // Load glyph
        if(FT_Load_Glyph(face, id, FT_LOAD_DEFAULT)){
            CR::log("[GFX] Font::genMapping: Failed to load glyph '%c' for font '%s'\n", (char)i, path.c_str());
            continue;
        }
        // Get base glyph
        if(FT_Get_Glyph(face->glyph, &glyph)){
            CR::log("[GFX] Font::genMapping: Failed to load glyph '%c' for font '%s'\n", (char)i, path.c_str());
            continue;
        }     
        // Get stroke glyph
        if(FT_Get_Glyph(face->glyph, &stroke)){
            CR::log("[GFX] Font::genMapping: Failed to load glyph '%c' for font '%s'\n", (char)i, path.c_str());
            continue;
        }             
        FT_Glyph_StrokeBorder(&stroke, stroker, 0, 1);
        renderGlyph(face, glyph, stroke, sg.get(), style);

        if(sg->w == 0 || sg->h == 0){ continue; }

        maxHeight = std::max(sg->_coors.y, maxHeight);
        sgs[id] = sg;

        sg->_index.x = cursor.x;
        sg->_index.y = cursor.y;

        // move to cursor
        auto xAdd = sg->_coors.x + 2;
        auto yAdd = maxHeight + 2;
        if(cursor.x + xAdd < maxSize.x){ 
            cursor.x += xAdd;
        }else{
            cursor.x = 0;
            cursor.y += yAdd;
        }

        expectedSize.x = std::max(cursor.x, expectedSize.x);
        expectedSize.y = std::max(maxHeight, std::max(cursor.y, expectedSize.y));

        FT_Done_Glyph(glyph);
        FT_Done_Glyph(stroke);
    }

    auto useImgFormat = CR::Gfx::ImageFormat::RGBA;

    switch(style.type){
        case Gfx::FontStyleType::OUTLINE_ONLY:
        case Gfx::FontStyleType::SOLID: {
            useImgFormat = CR::Gfx::ImageFormat::RGBA;
        } break;
        case Gfx::FontStyleType::SOLID_OUTLINED: {
            useImgFormat = CR::Gfx::ImageFormat::RG;
        } break;        
    }


    rsc->atlas = CR::Gfx::createTexture2D(0, expectedSize.x, expectedSize.y, useImgFormat, 1);
    rsc->atlasSize.set(expectedSize.x, expectedSize.y);

    // paste into atlas
    for(unsigned i = 0; i < mapping.size(); ++i){
        auto it = sgs.find(mapping[i]);
        if(it == sgs.end()){
            continue;
        }
        auto sg = it->second;
        // Paste glyph into atlas
        CR::Gfx::pasteSubTexture2D(rsc->atlas, sg->buffer, sg->bw, sg->bh, sg->_index.x, sg->_index.y, useImgFormat, 1);
        // Transfer glyph data
        rsc->glyphMap[mapping[i]] = CR::Gfx::FontGlyph();
        auto &gproxy = rsc->glyphMap[mapping[i]];
        gproxy.glyph = sg->symbol;
        sg->transferSpec(gproxy, expectedSize);
    }

    CR::log("[GFX] Loaded Font %s | Size %ipx | Encoding %s\n", path.c_str(), rscFont->style.size, CR::Gfx::FontEncondig::str(rscFont->style.encoding).c_str());

    FT_Stroker_Done(stroker);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return true;
}

void CR::Gfx::Font::unload(){

}