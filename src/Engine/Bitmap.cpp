#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "3rdparty/stb_image.h"
#include "3rdparty/stb_image_write.h"

#include "Graphics.hpp"
#include "Bitmap.hpp"

CR::Gfx::Bitmap CR::Gfx::Bitmap::sub(const CR::Rect<unsigned> &box){
    return this->sub(box.x, box.y, box.w, box.h);
}

CR::Gfx::Bitmap CR::Gfx::Bitmap::sub(unsigned x, unsigned y, unsigned width, unsigned height){
    CR::Gfx::Bitmap result;
    result.build(CR::Color(0, 0, 0, 0), this->format, width, height);
    
    for(unsigned _x = 0; _x < width; ++_x){
        for(unsigned _y = 0; _y < height; ++_y){
            result.pixels[_x + _y * width] = this->pixels[(_x+x) + (_y+y) * this->width];
        }
        
    }
    
    return result;
}

void CR::Gfx::Bitmap::paste(const CR::Gfx::Bitmap &src, const CR::Vec2<unsigned> &vec){
    this->paste(src, vec.x, vec.y);
}

void CR::Gfx::Bitmap::paste(const CR::Gfx::Bitmap &src, unsigned x, unsigned y){
    
    for(unsigned _x = 0; _x < src.width; ++_x){
        for(unsigned _y = 0; _y < src.height; ++_y){
            this->pixels[(_x+x) + (_y+y) * this->width] = src.pixels[_x + _y * src.width];
        }
    }

}

std::vector<uint8>  CR::Gfx::Bitmap::getFlatArray() const{
    std::vector<uint8> pixels;
    for(unsigned i = 0; i < this->pixels.size(); ++i){
        auto &p = this->pixels[i];
        switch(this->format){
            case ImageFormat::RGBA: {
                pixels.push_back(p.r);
                pixels.push_back(p.g);
                pixels.push_back(p.b);
                pixels.push_back(p.a);
            } break;
            case ImageFormat::RGB: {
                pixels.push_back(p.r);
                pixels.push_back(p.g);
                pixels.push_back(p.b);
            } break;     
            case ImageFormat::RED: {
                pixels.push_back(p.r);
            } break;   
            case ImageFormat::GREEN: {
                pixels.push_back(p.g);
            } break; 
            case ImageFormat::BLUE: {
                pixels.push_back(p.b);
            } break;      
            case ImageFormat::RG: {
                pixels.push_back(p.r);
                pixels.push_back(p.g);
            } break;                                                    
        } 
    }
    return pixels;
}

CR::Gfx::Bitmap CR::Gfx::Bitmap::copy(){
    CR::Gfx::Bitmap result;
    result.width = this->width;
    result.height = this->height;
    result.format = this->format;
    result.channels = this->channels;
    for(unsigned i = 0; i < this->width * this->height; ++i){
        result.pixels[i] = this->pixels[i];
    }
    return result;
}

CR::Rect<unsigned> CR::Gfx::Bitmap::autocrop(){
    unsigned min_x = this->width;
    unsigned min_y = this->height;
    unsigned max_x = 0;
    unsigned max_y = 0;
    
    for(unsigned y = 0; y < this->height; ++y){
        for(unsigned x = 0; x < this->width; ++x){
            unsigned i = x + y * this->width;
            auto &p = this->pixels[i];
            if(p.a != 0){
                min_x = std::min(min_x, x);
                min_y = std::min(min_y, y);		
                max_x = std::max(max_x, x);
                max_y = std::max(max_y, y);						
            }
        }
    }
    
    unsigned w = (max_x - min_x) + 1;
    unsigned h = (max_y - min_y) + 1;
    return CR::Rect<unsigned>(min_x, min_y, w, h);
}

CR::Gfx::Bitmap &CR::Gfx::Bitmap::build(const CR::Color &p, unsigned format, unsigned width, unsigned height){
    this->width = width;
    this->height = height;
    this->format = format;
    this->channels = format == CR::Gfx::ImageFormat::RGBA ? 4 : 3;
    this->pixels.resize(width * height);
    for(unsigned i = 0; i < this->width * this->height; ++i){
        this->pixels[i] = p;
    }
    return *this;
}

std::vector<std::vector<CR::Rect<unsigned>>> CR::Gfx::Bitmap::findBoxes(){

    auto get_box = [&](unsigned fx, unsigned fy, unsigned sw, unsigned sh){
        unsigned min_x = sw;
        unsigned min_y = sh;
        unsigned max_x = 0;
        unsigned max_y = 0;
        
        
        auto is_row_emty = [&](unsigned y){
            for(unsigned x = 0; x < sw; ++x){
                auto &p = this->pixels[(x+fx) + (y+fy) * this->width];
                if(p.a != 0){
                    return false;
                }
            }
            return true;
        };				
        
        for(unsigned y = 0; y < sh; ++y){
            for(unsigned x = 0; x < sw; ++x){
                if(is_row_emty(y)){
                    continue;
                }
                auto &p = this->pixels[(x+fx) + (y+fy) * this->width];
                if(p.a != 0){
                    min_x = std::min(min_x, x);
                    min_y = std::min(min_y, y);		
                    max_x = std::max(max_x, x);
                    max_y = std::max(max_y, y);		
                }
            }
        }
        
        
        min_x += fx;
        min_y += fy;
        max_x += fx;
        max_y += fy;			
        
        
        int w = (max_x - min_x) + 1;
        int h = (max_y - min_y) + 1;
        
        return CR::Rect<unsigned>(min_x, min_y, w, h);			
    };
    
    auto is_row_emty = [&](unsigned y){
        for(unsigned x = 0; x < this->width; ++x){
            auto &p = this->pixels[x + y * this->width];
            if(p.a != 0){
                return false;
            }
        }
        return true;
    };		
    
    
    auto get_line = [&](unsigned stX, unsigned stY, unsigned height){
        std::vector<CR::Rect<unsigned>> result;
        
        unsigned cursor_x = stX;
        unsigned cursor_dx = stX;
        
        auto is_column_emty = [&](unsigned x){
            for(unsigned y = stY; y < stY+height; ++y){
                auto &p = this->pixels[x + y * this->width];
                if(p.a != 0){
                    return false;
                }
            }
            return true;
        };			

        while(cursor_x+1 < this->width){
            bool is_empty = true;
            for(unsigned x = cursor_x; x < this->width; ++x){
                if(!is_column_emty(x)){
                    cursor_x = x;
                    is_empty = false;
                    break;
                }
            }
            if(is_empty){
                break;
            }
            cursor_dx = cursor_x+1;
            for(unsigned x = cursor_dx; x < this->width; ++x){
                if(is_column_emty(x)){
                    cursor_dx = x;
                    break;
                }
            }				
            
            //std::cout << "CURSOR " << cursor_x << " " << cursor_dx << " " << cursor_dx-cursor_x << std::endl;				
            auto b = get_box(cursor_x, stY, cursor_dx-cursor_x, height);
            //std::cout << "BOX " << b.x << " " << b.y << " " << b.w << " " << b.h << std::endl;			
            //std::cout << std::endl;
            result.push_back(b);
            cursor_x = cursor_dx+1;
            
                    
        }
        //std::cout << result.size() << std::endl;
        return result;
    };
    

    
    unsigned cursor_y = 0;
    unsigned cursor_dy = 0;
    std::vector<std::vector<CR::Rect<unsigned>>> out;
    
    while(cursor_y+1 < this->height){
        bool is_empty = true;
        for(unsigned y = cursor_y; y < this->height; ++y){
            if(!is_row_emty(y)){
                cursor_y = y;
                is_empty = false;
                break;
            }
        }
        if(is_empty){
            break;
        }

        cursor_dy = cursor_y + 1;
        
        for(unsigned y = cursor_dy; y < this->height; ++y){
            if(is_row_emty(y)){
                cursor_dy = y;
                break;
            }
        }

        //std::cout << cursor_y << " " << cursor_dy << "\theight: " << cursor_dy-cursor_y << std::endl;
        auto line = get_line(0, cursor_y, cursor_dy-cursor_y);
        //std::cout << line.size() << std::endl;
        out.push_back(line);
        cursor_y = cursor_dy+1;			
    }
    
    return out;
}


bool CR::Gfx::Bitmap::load(const std::string &path){
    int width, height, chan;
    
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &chan, 0); 
    
    if(!data){
        return false;
    }		
    
    this->width = width;
    this->height = height;
    
    this->format = chan == 4 ? ImageFormat::RGBA : ImageFormat::RGB;
    this->channels = chan;
    this->pixels.resize(width * height);
    
    unsigned total = width * height * chan;
    for(unsigned i = 0; i < width * height; ++i){
        auto &pixel = this->pixels[i];
        pixel.r = data[i * 4 + 0];
        pixel.g = data[i * 4 + 1];
        pixel.b = data[i * 4 + 2];
        pixel.a = data[i * 4 + 3];
    }
    
    stbi_image_free(data);
    
    return true;
}

bool CR::Gfx::Bitmap::write(const std::string &path){
    unsigned char *data = new unsigned char[width * height * this->channels];

    for(unsigned i = 0; i < this->width * this->height; ++i){
        auto &pixel = this->pixels[i];
        data[i * 4 + 0] = pixel.r;
        data[i * 4 + 1] = pixel.g;
        data[i * 4 + 2] = pixel.b;
        data[i * 4 + 3] = pixel.a;
    }	

    stbi_write_png(path.c_str(), this->width, this->height, 4, data, this->width * this->channels);
    
    delete data;
    
    return true;
}