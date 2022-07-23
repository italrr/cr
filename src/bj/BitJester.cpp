#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

namespace ImageFormat {
	enum ImageFormat : unsigned {
		RGB = 3,
		RGBA = 4	
	};
}

struct Pixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	Pixel(unsigned r, unsigned g, unsigned b, unsigned a){
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	Pixel(){
		r = g = b = a = 255;
	}
};

struct Box {
	int x;
	int y;
	int w;
	int h;
	
	Box(){
		
	}
	
	Box(unsigned x, unsigned y, unsigned w, unsigned h){
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
};

struct Vec2 {
	unsigned x;
	unsigned y;
	
	Vec2(){
		
	}
	
	Vec2(unsigned x, unsigned y){
		this->x = x;
		this->y = y;
	}
	
};

struct Bitmap {
	std::vector<Pixel> pixels;
	unsigned width;
	unsigned height;
	unsigned format;
	unsigned channels;
	
	Bitmap sub(const Box &box){
		return this->sub(box.x, box.y, box.w, box.h);
	}
	
	Bitmap sub(unsigned x, unsigned y, unsigned width, unsigned height){
		Bitmap result;
		result.build(Pixel(0, 0, 0, 0), this->format, width, height);
		
		for(unsigned _x = 0; _x < width; ++_x){
			for(unsigned _y = 0; _y < height; ++_y){
				result.pixels[_x + _y * width] = this->pixels[(_x+x) + (_y+y) * this->width];
			}
			
		}
		
		return result;
	}
	
	void paste(const Bitmap &src, const Vec2 &vec){
		this->paste(src, vec.x, vec.y);
	}
	
	void paste(const Bitmap &src, unsigned x, unsigned y){
		
		for(unsigned _x = 0; _x < src.width; ++_x){
			for(unsigned _y = 0; _y < src.height; ++_y){
				this->pixels[(_x+x) + (_y+y) * this->width] = src.pixels[_x + _y * src.width];
			}
		}

	}
		
	
	Bitmap copy(){
		Bitmap result;
		result.width = this->width;
		result.height = this->height;
		result.format = this->format;
		result.channels = this->channels;
		for(unsigned i = 0; i < this->width * this->height; ++i){
			result.pixels[i] = this->pixels[i];
		}
		return result;
	}
	
	Box autocrop(){
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
		return Box(min_x, min_y, w, h);
	}

	Bitmap &build(const Pixel &p, unsigned format, unsigned width, unsigned height){
		this->width = width;
		this->height = height;
		this->format = format;
		this->channels = format == ImageFormat::RGBA ? 4 : 3;
		this->pixels.resize(width * height);
		for(unsigned i = 0; i < this->width * this->height; ++i){
			this->pixels[i] = p;
		}
		return *this;
	}
	
	std::vector<std::vector<Box>> find_boxes(){
	
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
			
			return Box(min_x, min_y, w, h);			
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
			std::vector<Box> result;
			
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
		std::vector<std::vector<Box>> out;
		
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
	
	
	bool load(const std::string &path){
		int width, height, chan;
		
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &chan, 0); 
		
		if(!data){
			std::cout << "Failed to load " <<  path << std::endl;
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
	
	bool write(const std::string &path){
		unsigned char *data = new unsigned char[width * height * this->format];
	
		for(unsigned i = 0; i < this->width * this->height; ++i){
			auto &pixel = this->pixels[i];
			data[i * 4 + 0] = pixel.r;
			data[i * 4 + 1] = pixel.g;
			data[i * 4 + 2] = pixel.b;
			data[i * 4 + 3] = pixel.a;
		}	
	
		stbi_write_png(path.c_str(), this->width, this->height, 4, data, this->width * this->format);
		
		delete data;
		
		return true;
	}
};

int main(int argc, char* argv[]){
	Bitmap src;
	src.load("source.png");

	
	float O_PASTE_W = 90;
	float O_PASTE_H = 90;
	
	
	Bitmap out;
	out.build(Pixel(0, 0, 0, 0), ImageFormat::RGBA, 15*O_PASTE_W, 15*O_PASTE_H);


	auto boxes = src.find_boxes();
	
	unsigned cursor_y = 0;
	
	
	for(unsigned y = 0; y < boxes.size(); ++y){
		auto &line = boxes[y];	
		for(unsigned x = 0; x < line.size(); ++x){
			//std::cout << line[x].x << " " << line[x].y << " " << line[x].w << " " << line[x].h << std::endl;
			auto sub = src.sub(line[x]);
			out.paste(sub, x * O_PASTE_W + O_PASTE_W * 0.5f - static_cast<float>(sub.width) * 0.5f, y * O_PASTE_H + O_PASTE_H * 0.5f - static_cast<float>(sub.height) * 0.5f);
		}
	}
	
	
	out.write("./XD.png");
	


	
/* 	for(unsigned j = 0; j < nheight; ++j){
		auto frames = src.find_ghosts(0, j * O_PASTE_H, O_PASTE_H);
		for(unsigned i = 0; i < frames.size(); ++i){
			auto sub = src.sub(frames[i]);
			float out_x = cursor_x + O_PASTE_W * 0.5f - static_cast<float>(sub.width) * 0.5f;
			float out_y = cursor_y + O_PASTE_H * 0.5f - static_cast<float>(sub.height) * 0.5f;
			out.paste(sub, out_x, out_y);
			cursor_x += O_PASTE_W;
		}		
		cursor_x = 0;
		cursor_y += O_PASTE_H;
	}
	 */

	
	

	
	
/* 	Bitmap out;
	out.load("output.png");

	unsigned PASTE_X = 9;
	unsigned PASTE_Y = 0;
	
	

	unsigned COPY_X_OFFSET = 3;
	unsigned COPY_Y_OFFSET = 0;
	
	unsigned COPY_X = 9;
	unsigned COPY_Y = 0;

	
	unsigned O_WIDTH = 90;
	unsigned O_HEIGHT = 90;
	unsigned R_WIDTH = 50;
	unsigned R_HEIGHT = 90;
	
	auto sub = src.sub(COPY_X * R_WIDTH + COPY_X_OFFSET, COPY_Y * R_HEIGHT + COPY_Y_OFFSET, R_WIDTH, R_HEIGHT);
	auto cropped = sub.sub(sub.autocrop());
	
	
	out.paste(			cropped, 
						PASTE_X * O_WIDTH + static_cast<float>(O_WIDTH) * 0.5f - static_cast<float>(cropped.width) * 0.5f,
						PASTE_Y * O_HEIGHT + static_cast<float>(O_HEIGHT) * 0.5f - static_cast<float>(cropped.height) * 0.5f);
	out.write("./output.png");
	 */
	
	
	
}