#include <cstdlib>
#include <chrono>
#include <thread>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <limits.h>
#include <random>
#include <cmath>
#include <mutex>

#include "Tools.hpp"
#include "3rdparty/MD5.hpp"

/*
	TIME
*/

uint64 CR::ticks(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64 CR::epoch(){
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void CR::sleep(uint64 t){
    std::this_thread::sleep_for(std::chrono::milliseconds(t));
}


/* 
    CORE
*/

namespace CR {
    namespace Core {
            void exit(int code){
                std::exit(code);
            }
            std::vector<std::string> loadParams(int argc, char* argv[]){
                std::vector<std::string> params;
                for(int i = 0; i < argc; ++i){
                    params.push_back(std::string(argv[i]));
                }            
                return params;    
            }
    }
}


/*
	HASH
*/
namespace CR {
	namespace Hash {
        std::string md5(const std::string &path, bool partial){
			if(!File::exists(path)){
				return "";
			}
			
			FILE *f = fopen(path.c_str(), "rb");
			fseek(f, 0, SEEK_END);
			size_t fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *buffer = (char*)malloc(fsize);
			fread(buffer, fsize, 1, f);
			fclose(f);
			std::string hash = ThirdPartyMD5::md5(buffer, fsize);
			free(buffer);
			return hash;
		}

        std::string md5(char *data, size_t size, bool partial){
			return ThirdPartyMD5::md5(data, size);
		}		
	}
}

/* 
	FILE
*/
namespace CR {
	namespace File {

        std::string getCwd(){
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            return std::string(cwd);
        }

		std::string dirSep(){
			return Core::PLATFORM == Core::SupportedPlatform::LINUX ? "/" : "\\";
		}

        std::string fixPath(const std::string &path){
            auto result = path;
            if(Core::PLATFORM == Core::SupportedPlatform::WINDOWS){
                return CR::String::replaceAll(path, "/", "\\");                
            }
            return path;
        }

		bool exists(const std::string &path){
			struct stat tt;
			stat(path.c_str(), &tt);
			return S_ISREG(tt.st_mode);		
		}

		size_t size(const std::string &path){
			if (!exists(path)){
				return 0;
			}
			struct stat tt;
			stat(path.c_str(), &tt);
			return tt.st_size;			
		}

		std::string format(const std::string &filename){
			int k = filename.rfind(".");
			return k != -1 ? filename.substr(k+1, filename.length()-1) : "";
		}

		std::string filename(const std::string &path){
			int k = path.rfind(File::dirSep());
			return k != -1 ? path.substr(k+1, path.length()-1) : path;			
		}

		CR::Result list(const std::string &path, const std::string &format, int type, bool recursively, std::vector<std::string> &output){
			CR::Result result(CR::ResultType::noop);
			DIR *directory;
			struct dirent *ent;
			std::vector<std::string> formats = String::split(format, '|');
			bool anyf = !((int)formats.size());
			directory = opendir(path.c_str());
			if (directory == NULL){
				result.set(ResultType::Failure, "directory '"+path+"' does not exist");
				return result;
			}
			std::string filename;
			while ((ent = readdir(directory))) {
				filename = ent->d_name;
				if (filename == "." || filename == ".."){
					continue;
				}
				std::string fpath = path + filename;
				struct stat ft;
				stat(fpath.c_str(), &ft);
				/* Directory */
				if (S_ISDIR(ft.st_mode)) {
					if (type == ListType::Directory){
						output.push_back(CR::File::fixPath(fpath));
					}
					if (recursively){
						File::list(fpath, format, type, recursively, output);
					}
				}else
				/* File */
				if (S_ISREG(ft.st_mode)){
					if (type != ListType::File && type != ListType::Any){
						continue;
					}
					bool add = anyf;
					if (!anyf){
						for (int i = 0; i< formats.size(); ++i){
							if (File::format(fpath) == formats[i]){
								add = true;
								break;
							}
						}
					}
					if (add){
						output.push_back(CR::File::fixPath(fpath));
					}					
				}
			}
			closedir (directory);
			result.set(output.size() > 0 ? ResultType::Success : ResultType::Failure);
			return result;
		}

	}
}

/*
	STRING
*/
std::string CR::String::toLower(const std::string &str){
	std::string out;
	for(int i = 0; i < str.size(); i++){
		out += tolower(str.at(i));
	}
	return out;
}
std::vector<std::string> CR::String::split(const std::string &str, const char sep){
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while ((end = str.find(sep, start)) != std::string::npos) {
    tokens.push_back(str.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(str.substr(start));
  return tokens;
}

std::string CR::String::format(const std::string &_str, ...){
	va_list arglist;
	char buff[1024];
	va_start(arglist, _str);
	vsnprintf(buff, sizeof(buff), _str.c_str(), arglist);
	va_end( arglist );
	return std::string(buff);
}

std::string CR::String::formatByes(int n){
	int round = 0;
	float _n = n;
	while(_n > 1024){
		_n /= 1024;
		++round;
	}
	std::string size = "B";
	switch(round){
		case 1: 
			size = "KB";
			break;
		case 2: 
			size = "MB";
			break;
		case 3:
			size = "GB";
			break;
		case 4:
			size = "TB";									
			break;
	}
	return String::format("%.2f %s", _n, size.c_str());
}

std::string CR::String::str(float n){
	return std::to_string(n);
}

std::string CR::String::str(double n){
	return std::to_string(n);
}

std::string CR::String::str(uint8 n){
	return std::to_string(n);
}

std::string CR::String::str(int8 n){
	return std::to_string(n);
}

std::string CR::String::str(uint16 n){
	return std::to_string(n);
}

std::string CR::String::str(int16 n){
	return std::to_string(n);
}

std::string CR::String::str(uint32 n){
	return std::to_string(n);
}

std::string CR::String::str(int32 n){
	return std::to_string(n);
}

std::string CR::String::str(uint64 n){
	return std::to_string(n);
}

std::string CR::String::str(int64 n){
	return std::to_string(n);
}


std::string CR::String::replaceAll(std::string subject, const std::string& search, const std::string& replace){
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}


/* 
	MATH
*/

namespace CR {
	namespace Math {
        float round(float n){
			return std::round(n);
		}
		double round(double n){
			return std::round(n);
		}
		int even(int n){
			return n % 2 != 0 ? n + 1 : n;
		}
		int odd(int n){
			return n % 2 == 0 ? n + 1 : n;
		}
		int random(int min, int max){
			// static std::random_device rd;
			static std::mt19937 rng(std::time(nullptr));
			std::uniform_int_distribution<int> uni(min,max);
			return uni(rng);			
		}
		float sqrt(float n){
			return std::sqrt(n);
		}
		float sin(float n){
			return std::sin(n);
		}
		float cos(float n){
			return std::cos(n);
		}
		float tan(float n){
			return std::tan(n);
		}
		float asin(float n){
			return std::asin(n);
		}
		float acos(float n){
			return std::acos(n);
		}		
		float abs(float n){
			return std::abs(n);
		}
		float atan(float y, float x){
			return std::atan2(y, x);
		}
		float rads(float deg){
			return deg * 0.01745329251994329576923690768489;
		}
		float clamp(float num, float min, float max){
			return CR::Math::min(CR::Math::max(num, min), max);
		}
		float min(float a, float b){
			return a < b ? a : b;
		}
		float max(float a, float b){
			return a > b ? a : b;
		}		
		float degs(float rads){
			// return rads * (180.0f/PI);
			return (rads/PI*180) + (rads > 0 ? 0 : 360);

			// )
			// return (-rads * PI) / 180.0f;
			// return std::fmod((rads * 180.0f) / (PI), 360);	
		}
		float lerp(float x1, float x2, float step, float delta, float limit){
			if(x1 == x2) return x1;	
			if(step > 1.0 || step <= 0.0){
				step = 1.0;
			}	
			const float f = (1.0 - std::pow(1.0 - step, delta));
			x1 = (x1 * (1.0 - f)) + (x2 * f);	
                return x1;       
		}
		float lerpUnit(float x1, float x2, float step, float delta, float limit){
			return CR::Math::lerp(x1 * 100.0f, x2 * 100.0f, step, delta, limit) / 100.0f;                                			
		}

		Mat<4, 4, float> perspective(float fovy, float aspect, float zNear, float zFar){
			float const tanHalfFovy = CR::Math::tan(fovy / 2.0f);
			CR::Mat<4, 4, float> out(0.0f);

			// out.mat[0*4 + 0] = 1.0f / (aspect * tanHalfFovy);
			// out.mat[1*4 + 1] = 1.0f / tanHalfFovy;
			// out.mat[2*4 + 2] = zFar / (zNear - zFar);
			// out.mat[2*4 + 3] = -1.0f;
			// out.mat[3*4 + 2] = -(zFar * zNear) / (zFar - zNear);

				
			out.mat[0*4+0] = static_cast<float>(1) / (aspect * tanHalfFovy);
			out.mat[1*4+1] = static_cast<float>(1) / (tanHalfFovy);
			out.mat[2*4+2] = - (zFar + zNear) / (zFar - zNear);
			out.mat[2*4+3] = - static_cast<float>(1);
			out.mat[3*4+2] = - (static_cast<float>(2) * zFar * zNear) / (zFar - zNear);

			return out;
		}
		Mat<4, 4, float> orthogonal(float left, float right, float bottom, float top, float zNear, float zFar){
			auto out = MAT4Identity;
			// out.mat[0] = 2 / (right - left);
			// out.mat[5] = 2 / (top - bottom);
			// out.mat[10] = -1;
			// out.mat[12] = - (right + left) / (right - left);
			// out.mat[13] = - (top + bottom) / (top - bottom);


			// static float zFar = -1.0f;
			// static float zNear = 1.0f;


			// out.mat[0*4 + 0] = static_cast<float>(2) / (right - left);
			// out.mat[1*4 + 1] = static_cast<float>(2) / (top - bottom);
			// out.mat[2*4 + 2] = static_cast<float>(1) / (zFar - zNear);
			// out.mat[3*4 + 0] = - (right + left) / (right - left);
			// out.mat[3*4 + 1] = - (top + bottom) / (top - bottom);
			// out.mat[3*4 + 2] = - zNear / (zFar - zNear);

			out.mat[0*4+0] = static_cast<float>(2) / (right - left);
			out.mat[1*4+1] = static_cast<float>(2) / (top - bottom);
			out.mat[2*4+2] = static_cast<float>(2) / (zFar - zNear);
			out.mat[3*4+0] = - (right + left) / (right - left);
			out.mat[3*4+1] = - (top + bottom) / (top - bottom);
			out.mat[3*4+2] = - (zFar + zNear) / (zFar - zNear);


			return out;
		}
		Mat<4, 4, float> lookAt(const CR::Vec3<float> &pos, const CR::Vec3<float> &dir, const Vec3<float> &up){
			CR::Vec3<float> const f((dir - pos).normalize());
			CR::Vec3<float> const s(f.cross(up).normalize());
			CR::Vec3<float> const u(s.cross(f));

			CR::Mat<4, 4, float> out = MAT4Identity;
			out.mat[0*4 + 0] = s.x;
			out.mat[1*4 + 0] = s.y;
			out.mat[2*4 + 0] = s.z;

			out.mat[0*4 + 1] = u.x;
			out.mat[1*4 + 1] = u.y;
			out.mat[2*4 + 1] = u.z;

			out.mat[0*4 + 2] = -f.x;
			out.mat[1*4 + 2] = -f.y;
			out.mat[2*4 + 2] = -f.z;
			
			out.mat[3*4 + 0] = -s.dot(pos);
			out.mat[3*4 + 1] = -u.dot(pos);
			out.mat[3*4 + 2] = f.dot(pos);

			return out;
		}
	}
}

/* 
	SMALLPACKET
*/
namespace CR {

	SmallPacket::SmallPacket(const CR::SmallPacket &other){
		copy(other);
	}

	SmallPacket::SmallPacket(){
		this->data = NULL;
		clear();
	}

	SmallPacket::~SmallPacket(){
		std::unique_lock<std::mutex> lk(accesMutex);
		if(this->data != NULL){
			delete this->data;
		}
		lk.unlock();
	}

	void SmallPacket::copy(const SmallPacket &other){
		std::unique_lock<std::mutex> lk(accesMutex);
		if(this->data == NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}
		if(other.data == NULL){
			lk.unlock();
			clear();
			return;
		}
		memcpy(this->data, other.data, other.size);
		this->size = other.size; // actual size
		lk.unlock();
		reset();
	}

	SmallPacket& SmallPacket::operator= (const SmallPacket &other){
		copy(other);
		return *this;
	}

	void SmallPacket::clear(){
		std::unique_lock<std::mutex> lk(accesMutex);
		if(this->data != NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}
		index = 0; 
		size = 0;
		lk.unlock();
	}

	void SmallPacket::reset(){
		std::unique_lock<std::mutex> lk(accesMutex);
		index = 0; 
		lk.unlock();
	}

	void  SmallPacket::allocate(){
		if(this->data == NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}	
	}

	void SmallPacket::setIndex(size_t index){
		std::unique_lock<std::mutex> lk(accesMutex);
		this->index = index;
		lk.unlock();
	}

	std::shared_ptr<CR::Result> SmallPacket::read(std::string &str){
		std::unique_lock<std::mutex> lk(accesMutex);
		auto r = CR::makeResult(CR::ResultType::Success);
		if(this->data == NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}	
		if(index >= CR_SMALLPACKET_SIZE){
			r->setFailure("packet full");
			lk.unlock();
			return r;
		}
		auto sread = index;
		for(size_t i = index; i < CR_SMALLPACKET_SIZE; ++i){
			if(this->data[i] == '\0' && i == sread){
				// add one byte to index
				index += 1;
				lk.unlock();
				return r;
			}
			if (this->data[i] == '\0'){
				size_t size = i - index; // don't include the nullterminated
				char buff[size];
				memcpy(buff, data + index, size);
				str = std::string(buff, size);
				index = i + 1;
				this->size += i + 1;
				lk.unlock();
				return r;
			}
		}
		lk.unlock();
		return r;
	}

	std::shared_ptr<CR::Result> SmallPacket::read(void *data, size_t size){
		std::unique_lock<std::mutex> lk(accesMutex);
		auto r = CR::makeResult(CR::ResultType::Success);
		if(this->data == NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}	
		if((index >= CR_SMALLPACKET_SIZE) || (index + size > CR_SMALLPACKET_SIZE)){
			r->setFailure("packet full or won't fit");
			lk.unlock();
			return r;
		}
		memcpy(data, this->data + index, size);
		index += size;
		this->size += size;
		lk.unlock();
		return r;
	}		

	std::shared_ptr<CR::Result> SmallPacket::write(const std::string str){
		std::unique_lock<std::mutex> lk(accesMutex);
		auto r = CR::makeResult(CR::ResultType::Success);
		if(this->data == NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}	
		if((index >= CR_SMALLPACKET_SIZE) || (index + str.length() + 1 > CR_SMALLPACKET_SIZE)){
			r->setFailure("packet full or won't fit");
			lk.unlock();
			return r;
		}
		// just write \0 if the string is empty
		if(str.size() == 0){
			data[index] = '\0';
			// add one byte to index
			index += 1;
			this->size += 1;
			lk.unlock();
			return r;
		}
		size_t sl = str.length() + 1;
		memcpy(data + index, str.c_str(), sl);
		index += sl;
		this->size += sl;
		lk.unlock();
		return r;
	}

	std::shared_ptr<CR::Result> SmallPacket::write(const void *data, size_t size){
		std::unique_lock<std::mutex> lk(accesMutex);
		auto r = CR::makeResult(CR::ResultType::Success);
		if(this->data == NULL){
			this->data = (char*)malloc(CR_SMALLPACKET_SIZE);
		}	
		if((index >= CR_SMALLPACKET_SIZE) || (index + size > CR_SMALLPACKET_SIZE)){
			r->setFailure("packet full or won't fit");
			lk.unlock();
			return r;
		}
		memcpy(this->data + index, data, size);
		index += size;
		this->size += size;
		lk.unlock();
		return r;
	}
}