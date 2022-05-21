#include <cstdlib>
#include <chrono>
#include <thread>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
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

		std::string dirSep(){
			return Core::PLATFORM == Core::SupportedPlatform::LINUX ? "/" : "\\";
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
			std::vector<std::string> formats = String::split(format, "|");
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
				std::string fpath = path + dirSep() + filename;
				struct stat ft;
				stat(fpath.c_str(), &ft);
				/* Directory */
				if (S_ISDIR(ft.st_mode)) {
					if (type == ListType::Directory || anyf){
						output.push_back(fpath);
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
						output.push_back(fpath);
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
std::vector<std::string> CR::String::split(const std::string &str, const std::string &sep){
    std::vector<std::string> tokens;
    char *token = strtok((char*)str.c_str(), sep.c_str()); 
    while(token != NULL){ 
        tokens.push_back(std::string(token));
        token = strtok(NULL, "-"); 
    } 
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



/* 
	MATH
*/

namespace CR {
	namespace Math {
		int random(int min, int max){
			static std::random_device rd;
			std::mt19937 rng(rd());
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
		float atan(float y, float x){
			return std::atan2(y, x);
		}
		float rads(float deg){
			return deg * 0.01745329251994329576923690768489;
		}
		float degs(float rads){
			return rads * (180.0f/PI);	
		}

		// Mat<4, 4, float> perspective(float fovy, float aspect, float zNear, float zFar){
		// 	float const tanHalfFovy = CR::Math::tan(fovy / 2.0f);
		// 	CR::Mat<4, 4, float> out(0.0f);
		// 	out.mat[0] = 1.0f / (aspect * tanHalfFovy);
		// 	out.mat[5] = 1.0f / (tanHalfFovy);
		// 	out.mat[10] = - (zFar + zNear) / (zFar - zNear);
		// 	out.mat[11] = - 1.0f;
		// 	out.mat[14] = - (2.0f * zFar * zNear) / (zFar - zNear);
		// 	return out;
		// }
		// Mat<4, 4, float> orthogonal(float left, float right, float bottom, float top){
		// 	auto out = MAT4Identity;
		// 	out.mat[0] = 2 / (right - left);
		// 	out.mat[5] = 2 / (top - bottom);
		// 	out.mat[10] = -1;
		// 	out.mat[12] = - (right + left) / (right - left);
		// 	out.mat[13] = - (top + bottom) / (top - bottom);
		// 	return out;
		// }
		// Mat<4, 4, float> lookAt(const CR::Vec3<float> &pos, const CR::Vec3<float> &dir, const Vec3<float> &up){
		// 	CR::Vec3<float> const f((dir - pos).normalize());
		// 	CR::Vec3<float> const s(up.cross(f).normalize());
		// 	CR::Vec3<float> const u(f.cross(s));
		// 	CR::Mat<4, 4, float> out = MAT4Identity;
		// 	out.mat[0] = s.x;
		// 	out.mat[4] = s.y;
		// 	out.mat[8] = s.z;

		// 	out.mat[1] = u.x;
		// 	out.mat[5] = u.y;
		// 	out.mat[9] = u.z;

		// 	out.mat[2] = -f.x;
		// 	out.mat[6] = -f.y;
		// 	out.mat[10] = -f.z;
			
		// 	out.mat[12] = -s.dot(pos);
		// 	out.mat[13] = -u.dot(pos);
		// 	out.mat[14] = f.dot(pos);
		// 	return out;
		// }
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
		memcpy(this->data, other.data, CR_SMALLPACKET_SIZE);
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
		lk.unlock();
	}

	void SmallPacket::reset(){
		std::unique_lock<std::mutex> lk(accesMutex);
		index = 0; 
		lk.unlock();
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
		for(size_t i = index; i < CR_SMALLPACKET_SIZE; ++i){
			if (this->data[i] == '\0'){
				size_t size = i - index; // don't include the nullterminated
				char buff[size];
				memcpy(buff, data + index, size);
				str = std::string(buff, size);
				index = i + 1;
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
		size_t sl = str.length() + 1;
		memcpy(data + index, str.c_str(), sl);
		index += sl;
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
		lk.unlock();
		return r;
	}
}