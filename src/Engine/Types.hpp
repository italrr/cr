#ifndef CR_TYPES_HPP
    #define CR_TYPES_HPP

    #include <string>
    #include <vector>
    #include <mutex>
    #include <memory>
	#include <sys/types.h>
    
	#define int8 int8_t
	#define uint8 uint8_t
	#define int16 int16_t
	#define uint16 uint16_t
	#define int32 int32_t
	#define uint32 uint32_t
	#define int64 int64_t
	#define uint64 uint64_t   

    namespace CR {

		namespace Math {
			float sqrt(float n);
			float sin(float n);
			float cos(float n);
		}

        template<typename T>
        struct Vec2 {
            T x, y;
            
            Vec2(T x, T y){
                set(x, y);
            }
            
            Vec2(T c){
                set(c);
            }

			Vec2(const CR::Vec2<T> &vec){
				this->x = vec.x; this->y = vec.y;
			}
            
            Vec2(){
                set(0);
            }
            
            void set(T x, T y){
                this->x = x; this->y = y;
            }

            void set(T c){
                this->x = c;
                this->y = c;
            }

			CR::Vec2<T> operator*(const CR::Vec2<T> &vec) const{
				return CR::Vec2<T>(this->x * vec.x, this->y * vec.y);
			}

			CR::Vec2<T> operator/(const CR::Vec2<T> &vec) const{
				return CR::Vec2<T>(this->x / vec.x, this->y / vec.y);
			}

			CR::Vec2<T> operator+(const CR::Vec2<T> &vec) const{
				return CR::Vec2<T>(this->x + vec.x, this->y + vec.y);
			}

			CR::Vec2<T> operator-(const CR::Vec2<T> &vec) const{
				return CR::Vec2<T>(this->x - vec.x, this->y - vec.y);
			}			

			CR::Vec2<T> normalize() const{
				CR::Vec2<T> normalized = *this;
				T sqr = normalized.x * normalized.x + normalized.y * normalized.y;
				if(sqr == 1 || sqr == 0){
					return normalized;
				}
				T invrt = 1.0f/CR::Math::sqrt(sqr);
				normalized.x *= invrt;
				normalized.y *= invrt;		
				return normalized;
			}	

			T dot(const CR::Vec2<T> &vec) const{
				T product = 0; 
				product += x * vec.x; 
				product += y * vec.y; 
				return product; 
			}

			CR::Vec2<T> cross(const CR::Vec2<T> &vec) const{
				return this->x * vec.y - this->y * vec.x;
			}

			CR::Vec2<T> cross(T sc) {
				return CR::Vec2<T>(sc * this->y, -sc * this->x);
			}										
        };
        

    }


#endif    