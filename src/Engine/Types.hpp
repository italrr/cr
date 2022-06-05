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

			std::string str() const{
				return "["+std::to_string(this->x)+", "+std::to_string(this->y)+"]";
			}									
        };
        
        template<typename T>
        struct Vec3 {
            T x, y, z;
            
            Vec3(T x, T y, T z){
                set(x, y, z);
            }
            
            Vec3(T c){
                set(c);
            }
            
			Vec3(const CR::Vec3<T> &vec){
				this->x = vec.x; this->y = vec.y; this->z = vec.z;
			}

            Vec3(){
                set(0);
            }
            
            void set(T x, T y, T z){
                this->x = x; this->y = y; this->z = z;
            }

            void set(T c){
                this->x = c;
                this->y = c;
                this->z = c;
            }

			CR::Vec3<T> normalize() const {
				CR::Vec3<T> normalized = *this;
				float sqr = normalized.dot(normalized);
				if(sqr == 1 || sqr == 0){
					return normalized;
				}
				float invrt = 1.0f / CR::Math::sqrt(sqr);
				normalized.x *= invrt;
				normalized.y *= invrt;
				normalized.z *= invrt;				
				return normalized;
			}	

			CR::Vec3<T> operator*(const CR::Vec3<T> &vec) const{
				return CR::Vec3<T>(this->x * vec.x, this->y * vec.y, this->z * vec.z);
			}

			CR::Vec3<T> operator/(const CR::Vec3<T> &vec) const{
				return CR::Vec3<T>(this->x / vec.x, this->y / vec.y, this->z / vec.z);
			}

			CR::Vec3<T> operator+(const CR::Vec3<T> &vec) const{
				return CR::Vec3<T>(this->x + vec.x, this->y + vec.y, this->z + vec.z);
			}

			CR::Vec3<T> operator-(const CR::Vec3<T> &vec) const{
				return CR::Vec3<T>(this->x - vec.x, this->y - vec.y, this->z - vec.z);
			}						

			T dot(const CR::Vec3<T> &vec) const{
				return this->x * vec.x + this->y * vec.y + this->z * vec.z;
			}

			CR::Vec3<T> cross(const CR::Vec3<T> &vec) const{
				CR::Vec3<T> out;
				out.x = this->y * vec.z - vec.y * this->z;
				out.y = -(this->x * vec.z - vec.x * this->z);
				out.z = this->x * vec.y - vec.x * this->y;
				return out;
			}


        };     


        template<typename T>
        struct Vec4 {
            T x, y, z, w;
            
            Vec4(T x, T y, T z, T w){
                set(x, y, z, w);
            }
            
            Vec4(T c){
                set(c);
            }
            
            Vec4(){
                set(0);
            }

			Vec4(const CR::Vec4<T> &vec){
				this->x = vec.x;
				this->y = vec.y;
				this->z = vec.z;
				this->w = vec.w;
			}			
            
            void set(T x, T y, T z, T w){
                this->x = x; this->y = y; this->z = z; this->w = w;
            }

            void set(T c){
                this->x = c;
                this->y = c;
                this->z = c;
				this->w = c;
            }

			CR::Vec4<T> operator*(const CR::Vec4<T> &vec) const{
				return CR::Vec4<T>(this->x * vec.x, this->y * vec.y, this->z * vec.z, this->w * vec.w);
			}

			CR::Vec4<T> operator/(const CR::Vec4<T> &vec) const{
				return CR::Vec4<T>(this->x / vec.x, this->y / vec.y, this->z / vec.z, this->w / vec.w);
			}

			CR::Vec4<T> operator+(const CR::Vec4<T> &vec) const{
				return CR::Vec4<T>(this->x + vec.x, this->y + vec.y, this->z + vec.z, this->w + vec.w);
			}

			CR::Vec4<T> operator-(const CR::Vec4<T> &vec) const{
				return CR::Vec4<T>(this->x - vec.x, this->y - vec.y, this->z - vec.z, this->w - vec.w);
			}									

			CR::Vec4<T> normalize() const{
				CR::Vec4<T> normalized = *this;
				float sqr = normalized.x * normalized.x + normalized.y * normalized.y + normalized.z * normalized.z;
				if(sqr == 1 || sqr == 0){
					return normalized;
				}
				float invrt = 1.0f/CR::Math::sqrt(sqr);
				normalized.x *= invrt;
				normalized.y *= invrt;
				normalized.z *= invrt;				
				return normalized;
			}

			float dot(const CR::Vec4<T> &vec) const{
				return this->x * vec.x + this->y * vec.y + this->z * vec.z + this->w * vec.w;
			}

			CR::Vec4<T> cross(const CR::Vec4<T> &vec) const{
				CR::Vec4<T> out;
				out.x = this->y*vec.z - this->z*vec.y;
				out.y = this->z*vec.x - this->x*vec.z;
				out.z = this->x*vec.y - this->y*vec.x;
				return out;
			}

        };  

        // copy pasted vec4
        template<typename T>
        struct Rect {
            T x, y, w, h;
            
            Rect(T x, T y, T w, T h){
                set(x, y, w, h);
            }
            
            Rect(T c){
                set(c);
            }
            
            Rect(){
                set(0);
            }

			Rect(const CR::Rect<T> &vec){
				this->x = vec.x;
				this->y = vec.y;
				this->w = vec.w;
				this->h = vec.h;
			}			
            
            void set(T x, T y, T w, T h){
                this->x = x; this->y = y; this->w = w; this->h = h;
            }

            void set(T c){
                this->x = c;
                this->y = c;
                this->w = w;
				this->h = h;
            }

			CR::Rect<T> operator*(const CR::Rect<T> &vec) const{
				return CR::Rect<T>(this->x * vec.x, this->y * vec.y, this->w * vec.w, this->h * vec.h);
			}

			CR::Rect<T> operator/(const CR::Rect<T> &vec) const{
				return CR::Rect<T>(this->x / vec.x, this->y / vec.y, this->w / vec.w, this->h / vec.h);
			}

			CR::Rect<T> operator+(const CR::Rect<T> &vec) const{
				return CR::Rect<T>(this->x + vec.x, this->y + vec.y, this->w + vec.w, this->h + vec.h);
			}

			CR::Rect<T> operator-(const CR::Rect<T> &vec) const{
				return CR::Rect<T>(this->x - vec.x, this->y - vec.y, this->w - vec.w, this->h - vec.h);
			}									

			CR::Rect<T> normalize() const{
				CR::Rect<T> normalized = *this;
				float sqr = normalized.x * normalized.x + normalized.y * normalized.y + normalized.w * normalized.w;
				if(sqr == 1 || sqr == 0){
					return normalized;
				}
				float invrt = 1.0f/CR::Math::sqrt(sqr);
				normalized.x *= invrt;
				normalized.y *= invrt;
				normalized.w *= invrt;				
				return normalized;
			}

			float dot(const CR::Rect<T> &vec) const{
				return this->x * vec.x + this->y * vec.y + this->w * vec.w + this->h * vec.h;
			}

			CR::Rect<T> cross(const CR::Rect<T> &vec) const{
				CR::Rect<T> out;
				out.x = this->y*vec.w - this->z*vec.y;
				out.y = this->z*vec.x - this->x*vec.w;
				out.z = this->x*vec.y - this->y*vec.x;
				return out;
			}

        };         

		static const float MAT4FIdentity[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1		
		};		

		template<unsigned w, unsigned h, typename T>
		struct Mat {
			unsigned size;
			T mat[w * h];
            
			Mat(T c){
				set(c);
            }

			Mat(){
				
            }			

            Mat(const T mat[w*h]){
				set(mat);
            }		

            void set(const T mat[w*h]){
				for(unsigned i = 0; i < w * h; ++i){
					this->mat[i] = mat[i];
				}
            }	

			void set(const T c){
				for(unsigned i = 0; i < w * h; ++i){
					this->mat[i] = c;
				}
			}

			Mat(const Mat<3, 3, T> &mat3){
				if(w == 4 && h == 4){
					mat[0 + 0*4] = mat3.mat[0 + 0*3];// AssimpMatrix.a1;
					mat[0 + 1*4] = mat3.mat[1 + 0*3]; //AssimpMatrix.a2;
					mat[0 + 2*4] = mat3.mat[2 + 0*3]; // AssimpMatrix.a3;
					mat[0 + 3*4] = 0.0f;
					
					mat[1 + 0*4] = mat3.mat[0 + 1*3]; //AssimpMatrix.b1;
					mat[1 + 1*4] = mat3.mat[1 + 1*3]; //AssimpMatrix.b2;
					mat[1 + 2*4] = mat3.mat[2 + 1*3]; // AssimpMatrix.b3;
					mat[1 + 3*4] = 0.0f;

					mat[2 + 0*4] = mat3.mat[0 + 2*3]; //AssimpMatrix.c1;
					mat[2 + 1*4] = mat3.mat[1 + 2*3]; //AssimpMatrix.c2;
					mat[2 + 2*4] = mat3.mat[2 + 2*3]; // AssimpMatrix.c3;
					mat[2 + 3*4] = 0.0f;

					mat[3 + 0*4] = 0.0f;
					mat[3 + 1*4] = 0.0f;
					mat[3 + 2*4] = 0.0f;
					mat[3 + 3*4] = 1.0f;
				}			
			}

			CR::Mat<4, 4, T> operator*(const CR::Mat<4, 4, T> &m2) const{
				CR::Mat<4, 4, T> out;
				// unsigned int row, column, row_offset;

				// for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
				// 	for (column = 0; column < 4; ++column)
				// 		out.mat[row_offset + column] =
				// 			(this->mat[row_offset + 0] * m2.mat[column + 0]) +
				// 			(this->mat[row_offset + 1] * m2.mat[column + 4]) +
				// 			(this->mat[row_offset + 2] * m2.mat[column + 8]) +
				// 			(this->mat[row_offset + 3] * m2.mat[column + 12]);

				for (unsigned int i = 0; i < 4; i++) {
					for (unsigned int j = 0; j < 4; j++) {
						out.mat[i + j*4] = mat[i + 0*4] * m2.mat[0 + j*4] +
							mat[i + 1*4] * m2.mat[1 + j*4] +
							mat[i + 2*4] * m2.mat[2 + j*4] +
							mat[i + 3*4] * m2.mat[3 + j*4];
					}
				}

				return out;
			}

			CR::Mat<4, 4, T> operator*(T s) const{
				Mat<4, 4, T> out(*this);


				for(int i = 0; i < 4*4; ++i){
					out.mat[i] *= s;
				}


				return out;
			}			

			CR::Mat<4, 4, T> translate(const CR::Vec3<T> &vec) const{
				Mat<4, 4, T> out(*this);
				
				out.mat[3*4 + 0] = this->mat[0*4 + 0] * vec.x + this->mat[1*4 + 0] * vec.y + this->mat[2*4 + 0] * vec.z + this->mat[3*4 + 0];
				out.mat[3*4 + 1] = this->mat[0*4 + 1] * vec.x + this->mat[1*4 + 1] * vec.y + this->mat[2*4 + 1] * vec.z + this->mat[3*4 + 1];
				out.mat[3*4 + 2] = this->mat[0*4 + 2] * vec.x + this->mat[1*4 + 2] * vec.y + this->mat[2*4 + 2] * vec.z + this->mat[3*4 + 2];
				out.mat[3*4 + 3] = this->mat[0*4 + 3] * vec.x + this->mat[1*4 + 3] * vec.y + this->mat[2*4 + 3] * vec.z + this->mat[3*4 + 3];

				// out[2*4 + 0] = this->mat[0*4 + 0] * vec.x;
				// out[2*4 + 1] = this->mat[0*4 + 1] * vec.x;
				// out[2*4 + 2] = this->mat[0*4 + 2] * vec.x;
				// out[2*4 + 3] = this->mat[0*4 + 3] * vec.x;

				// out[2*4 + 0] = this->mat[0*4 + 0] * vec.x;
				// out[2*4 + 1] = this->mat[0*4 + 1] * vec.x;
				// out[2*4 + 2] = this->mat[0*4 + 2] * vec.x;
				// out[2*4 + 3] = this->mat[0*4 + 3] * vec.x;				


				// out.mat[12] = vec.x;
				// out.mat[13] = vec.y;
				// out.mat[14] = vec.z;


				// out[3]

				return out;
			}

			CR::Mat<4, 4, T> scale(const CR::Vec3<T> &vec) const{
				Mat<4, 4, T> out(*this);
				
				out.mat[0 + 0] = this->mat[0* 4+ 0] * vec.x;
				out.mat[0*4 + 1] = this->mat[0* 4+ 1] * vec.x;
				out.mat[0*4 + 2] = this->mat[0* 4+ 2] * vec.x;
				out.mat[0*4 + 3] = this->mat[0* 4+ 3] * vec.x;

				out.mat[1*4 + 0] = this->mat[1* 4+ 0] * vec.y;
				out.mat[1*4 + 1] = this->mat[1* 4+ 1] * vec.y;
				out.mat[1*4 + 2] = this->mat[1* 4+ 2] * vec.y;
				out.mat[1*4 + 3] = this->mat[1* 4+ 3] * vec.y;

				out.mat[2*4 + 0] = this->mat[2* 4+ 0] * vec.z;
				out.mat[2*4 + 1] = this->mat[2* 4+ 1] * vec.z;
				out.mat[2*4 + 2] = this->mat[2* 4+ 2] * vec.z;
				out.mat[2*4 + 3] = this->mat[2* 4+ 3] * vec.z;				

				out.mat[3*4 + 0] = this->mat[3* 4+ 0];
				out.mat[3*4 + 1] = this->mat[3* 4+ 1];
				out.mat[3*4 + 2] = this->mat[3*4 + 2];
				out.mat[3*4 + 3] = this->mat[3*4 + 3];

				return out;
			}

			CR::Mat<4, 4, T> transpose(){
				CR::Mat<4, 4, T> result;

				result.mat[0 + 4 * 0] = this->mat[0 + 4 * 0];
				result.mat[0 + 4 * 1] = this->mat[1 + 4 * 0];
				result.mat[0 + 4 * 2] = this->mat[2 + 4 * 0];
				result.mat[0 + 4 * 3] = this->mat[3 + 4 * 0];

				result.mat[1 + 4 * 0] = this->mat[0 + 4 * 1];
				result.mat[1 + 4 * 1] = this->mat[1 + 4 * 1];
				result.mat[1 + 4 * 2] = this->mat[2 + 4 * 1];
				result.mat[1 + 4 * 3] = this->mat[3 + 4 * 1];

				result.mat[2 + 4 * 0] = this->mat[0 + 4 * 2];
				result.mat[2 + 4 * 1] = this->mat[1 + 4 * 2];
				result.mat[2 + 4 * 2] = this->mat[2 + 4 * 2];
				result.mat[2 + 4 * 3] = this->mat[3 + 4 * 2];

				result.mat[3 + 4 * 0] = this->mat[0 + 4 * 3];
				result.mat[3 + 4 * 1] = this->mat[1 + 4 * 3];
				result.mat[3 + 4 * 2] = this->mat[2 + 4 * 3];
				result.mat[3 + 4 * 3] = this->mat[3 + 4 * 3];

				return result;				
			}

			T determinant(){
				if(w == 4 && h == 4){
				return this->mat[0 + 4*0] * this->mat[1 + 4*1] * this->mat[2 + 4*2] * this->mat[3 + 4*3] - this->mat[0 + 4*0] * this->mat[1 + 4*1] * this->mat[2 + 4*3] * this->mat[3 + 4*2] + this->mat[0 + 4*0] * this->mat[1 + 4*2] * this->mat[2 + 4*3] * this->mat[3 + 4*1] - this->mat[0 + 4*0] * this->mat[1 + 4*2] * this->mat[2 + 4*1] * this->mat[3 + 4*3]
						+ this->mat[0 + 4*0] * this->mat[1 + 4*3] * this->mat[2 + 4*1] * this->mat[3 + 4*2] - this->mat[0 + 4*0] * this->mat[1 + 4*3] * this->mat[2 + 4*2] * this->mat[3 + 4*1] - this->mat[0 + 4*1] * this->mat[1 + 4*2] * this->mat[2 + 4*3] * this->mat[3 + 4*0] + this->mat[0 + 4*1] * this->mat[1 + 4*2] * this->mat[2 + 4*0] * this->mat[3 + 4*3]
						- this->mat[0 + 4*1] * this->mat[1 + 4*3] * this->mat[2 + 4*0] * this->mat[3 + 4*2] + this->mat[0 + 4*1] * this->mat[1 + 4*3] * this->mat[2 + 4*2] * this->mat[3 + 4*0] - this->mat[0 + 4*1] * this->mat[1 + 4*0] * this->mat[2 + 4*2] * this->mat[3 + 4*3] + this->mat[0 + 4*1] * this->mat[1 + 4*0] * this->mat[2 + 4*3] * this->mat[3 + 4*2]
						+ this->mat[0 + 4*2] * this->mat[1 + 4*3] * this->mat[2 + 4*0] * this->mat[3 + 4*1] - this->mat[0 + 4*2] * this->mat[1 + 4*3] * this->mat[2 + 4*1] * this->mat[3 + 4*0] + this->mat[0 + 4*2] * this->mat[1 + 4*0] * this->mat[2 + 4*1] * this->mat[3 + 4*3] - this->mat[0 + 4*2] * this->mat[1 + 4*0] * this->mat[2 + 4*3] * this->mat[3 + 4*1]
						+ this->mat[0 + 4*2] * this->mat[1 + 4*1] * this->mat[2 + 4*3] * this->mat[3 + 4*0] - this->mat[0 + 4*2] * this->mat[1 + 4*1] * this->mat[2 + 4*0] * this->mat[3 + 4*3] - this->mat[0 + 4*3] * this->mat[1 + 4*0] * this->mat[2 + 4*1] * this->mat[3 + 4*2] + this->mat[0 + 4*3] * this->mat[1 + 4*0] * this->mat[2 + 4*2] * this->mat[3 + 4*1]
						- this->mat[0 + 4*3] * this->mat[1 + 4*1] * this->mat[2 + 4*2] * this->mat[3 + 4*0] + this->mat[0 + 4*3] * this->mat[1 + 4*1] * this->mat[2 + 4*0] * this->mat[3 + 4*2] - this->mat[0 + 4*3] * this->mat[1 + 4*2] * this->mat[2 + 4*0] * this->mat[3 + 4*1] + this->mat[0 + 4*3] * this->mat[1 + 4*2] * this->mat[2 + 4*1] * this->mat[3 + 4*0];
				}
				return static_cast<T>(0);
			}

			CR::Mat<4, 4, T> inverse(){
				CR::Mat<4, 4, float> reversed;
				float det = determinant();
				if(det == 0.0f){
					return reversed;
				}

				float invdet = 1.0f / det;


				reversed.mat[0 + 4*0] = invdet  * (this->mat[1 + 4*1] * (this->mat[2 + 4*2] * this->mat[3 + 4*3] - this->mat[2 + 4*3] * this->mat[3 + 4*2]) + this->mat[1 + 4*2] * (this->mat[2 + 4*3] * this->mat[3 + 4*1] - this->mat[2 + 4*1] * this->mat[3 + 4*3]) + this->mat[1 + 4*3] * (this->mat[2 + 4*1] * this->mat[3 + 4*2] - this->mat[2 + 4*2] * this->mat[3 + 4*1]));
				reversed.mat[0 + 4*1] = -invdet * (this->mat[0 + 4*1] * (this->mat[2 + 4*2] * this->mat[3 + 4*3] - this->mat[2 + 4*3] * this->mat[3 + 4*2]) + this->mat[0 + 4*2] * (this->mat[2 + 4*3] * this->mat[3 + 4*1] - this->mat[2 + 4*1] * this->mat[3 + 4*3]) + this->mat[0 + 4*3] * (this->mat[2 + 4*1] * this->mat[3 + 4*2] - this->mat[2 + 4*2] * this->mat[3 + 4*1]));
				reversed.mat[0 + 4*2] = invdet  * (this->mat[0 + 4*1] * (this->mat[1 + 4*2] * this->mat[3 + 4*3] - this->mat[1 + 4*3] * this->mat[3 + 4*2]) + this->mat[0 + 4*2] * (this->mat[1 + 4*3] * this->mat[3 + 4*1] - this->mat[1 + 4*1] * this->mat[3 + 4*3]) + this->mat[0 + 4*3] * (this->mat[1 + 4*1] * this->mat[3 + 4*2] - this->mat[1 + 4*2] * this->mat[3 + 4*1]));
				reversed.mat[0 + 4*3] = -invdet * (this->mat[0 + 4*1] * (this->mat[1 + 4*2] * this->mat[2 + 4*3] - this->mat[1 + 4*3] * this->mat[2 + 4*2]) + this->mat[0 + 4*2] * (this->mat[1 + 4*3] * this->mat[2 + 4*1] - this->mat[1 + 4*1] * this->mat[2 + 4*3]) + this->mat[0 + 4*3] * (this->mat[1 + 4*1] * this->mat[2 + 4*2] - this->mat[1 + 4*2] * this->mat[2 + 4*1]));
				reversed.mat[1 + 4*0] = -invdet * (this->mat[1 + 4*0] * (this->mat[2 + 4*2] * this->mat[3 + 4*3] - this->mat[2 + 4*3] * this->mat[3 + 4*2]) + this->mat[1 + 4*2] * (this->mat[2 + 4*3] * this->mat[3 + 4*0] - this->mat[2 + 4*0] * this->mat[3 + 4*3]) + this->mat[1 + 4*3] * (this->mat[2 + 4*0] * this->mat[3 + 4*2] - this->mat[2 + 4*2] * this->mat[3 + 4*0]));
				reversed.mat[1 + 4*1] = invdet  * (this->mat[0 + 4*0] * (this->mat[2 + 4*2] * this->mat[3 + 4*3] - this->mat[2 + 4*3] * this->mat[3 + 4*2]) + this->mat[0 + 4*2] * (this->mat[2 + 4*3] * this->mat[3 + 4*0] - this->mat[2 + 4*0] * this->mat[3 + 4*3]) + this->mat[0 + 4*3] * (this->mat[2 + 4*0] * this->mat[3 + 4*2] - this->mat[2 + 4*2] * this->mat[3 + 4*0]));
				reversed.mat[1 + 4*2] = -invdet * (this->mat[0 + 4*0] * (this->mat[1 + 4*2] * this->mat[3 + 4*3] - this->mat[1 + 4*3] * this->mat[3 + 4*2]) + this->mat[0 + 4*2] * (this->mat[1 + 4*3] * this->mat[3 + 4*0] - this->mat[1 + 4*0] * this->mat[3 + 4*3]) + this->mat[0 + 4*3] * (this->mat[1 + 4*0] * this->mat[3 + 4*2] - this->mat[1 + 4*2] * this->mat[3 + 4*0]));
				reversed.mat[1 + 4*3] = invdet  * (this->mat[0 + 4*0] * (this->mat[1 + 4*2] * this->mat[2 + 4*3] - this->mat[1 + 4*3] * this->mat[2 + 4*2]) + this->mat[0 + 4*2] * (this->mat[1 + 4*3] * this->mat[2 + 4*0] - this->mat[1 + 4*0] * this->mat[2 + 4*3]) + this->mat[0 + 4*3] * (this->mat[1 + 4*0] * this->mat[2 + 4*2] - this->mat[1 + 4*2] * this->mat[2 + 4*0]));
				reversed.mat[2 + 4*0] = invdet  * (this->mat[1 + 4*0] * (this->mat[2 + 4*1] * this->mat[3 + 4*3] - this->mat[2 + 4*3] * this->mat[3 + 4*1]) + this->mat[1 + 4*1] * (this->mat[2 + 4*3] * this->mat[3 + 4*0] - this->mat[2 + 4*0] * this->mat[3 + 4*3]) + this->mat[1 + 4*3] * (this->mat[2 + 4*0] * this->mat[3 + 4*1] - this->mat[2 + 4*1] * this->mat[3 + 4*0]));
				reversed.mat[2 + 4*1] = -invdet * (this->mat[0 + 4*0] * (this->mat[2 + 4*1] * this->mat[3 + 4*3] - this->mat[2 + 4*3] * this->mat[3 + 4*1]) + this->mat[0 + 4*1] * (this->mat[2 + 4*3] * this->mat[3 + 4*0] - this->mat[2 + 4*0] * this->mat[3 + 4*3]) + this->mat[0 + 4*3] * (this->mat[2 + 4*0] * this->mat[3 + 4*1] - this->mat[2 + 4*1] * this->mat[3 + 4*0]));
				reversed.mat[2 + 4*2] = invdet  * (this->mat[0 + 4*0] * (this->mat[1 + 4*1] * this->mat[3 + 4*3] - this->mat[1 + 4*3] * this->mat[3 + 4*1]) + this->mat[0 + 4*1] * (this->mat[1 + 4*3] * this->mat[3 + 4*0] - this->mat[1 + 4*0] * this->mat[3 + 4*3]) + this->mat[0 + 4*3] * (this->mat[1 + 4*0] * this->mat[3 + 4*1] - this->mat[1 + 4*1] * this->mat[3 + 4*0]));
				reversed.mat[2 + 4*3] = -invdet * (this->mat[0 + 4*0] * (this->mat[1 + 4*1] * this->mat[2 + 4*3] - this->mat[1 + 4*3] * this->mat[2 + 4*1]) + this->mat[0 + 4*1] * (this->mat[1 + 4*3] * this->mat[2 + 4*0] - this->mat[1 + 4*0] * this->mat[2 + 4*3]) + this->mat[0 + 4*3] * (this->mat[1 + 4*0] * this->mat[2 + 4*1] - this->mat[1 + 4*1] * this->mat[2 + 4*0]));
				reversed.mat[3 + 4*0] = -invdet * (this->mat[1 + 4*0] * (this->mat[2 + 4*1] * this->mat[3 + 4*2] - this->mat[2 + 4*2] * this->mat[3 + 4*1]) + this->mat[1 + 4*1] * (this->mat[2 + 4*2] * this->mat[3 + 4*0] - this->mat[2 + 4*0] * this->mat[3 + 4*2]) + this->mat[1 + 4*2] * (this->mat[2 + 4*0] * this->mat[3 + 4*1] - this->mat[2 + 4*1] * this->mat[3 + 4*0]));
				reversed.mat[3 + 4*1] = invdet  * (this->mat[0 + 4*0] * (this->mat[2 + 4*1] * this->mat[3 + 4*2] - this->mat[2 + 4*2] * this->mat[3 + 4*1]) + this->mat[0 + 4*1] * (this->mat[2 + 4*2] * this->mat[3 + 4*0] - this->mat[2 + 4*0] * this->mat[3 + 4*2]) + this->mat[0 + 4*2] * (this->mat[2 + 4*0] * this->mat[3 + 4*1] - this->mat[2 + 4*1] * this->mat[3 + 4*0]));
				reversed.mat[3 + 4*2] = -invdet * (this->mat[0 + 4*0] * (this->mat[1 + 4*1] * this->mat[3 + 4*2] - this->mat[1 + 4*2] * this->mat[3 + 4*1]) + this->mat[0 + 4*1] * (this->mat[1 + 4*2] * this->mat[3 + 4*0] - this->mat[1 + 4*0] * this->mat[3 + 4*2]) + this->mat[0 + 4*2] * (this->mat[1 + 4*0] * this->mat[3 + 4*1] - this->mat[1 + 4*1] * this->mat[3 + 4*0]));
				reversed.mat[3 + 4*3] = invdet  * (this->mat[0 + 4*0] * (this->mat[1 + 4*1] * this->mat[2 + 4*2] - this->mat[1 + 4*2] * this->mat[2 + 4*1]) + this->mat[0 + 4*1] * (this->mat[1 + 4*2] * this->mat[2 + 4*0] - this->mat[1 + 4*0] * this->mat[2 + 4*2]) + this->mat[0 + 4*2] * (this->mat[1 + 4*0] * this->mat[2 + 4*1] - this->mat[1 + 4*1] * this->mat[2 + 4*0]));

				return reversed;
			}

			CR::Mat<4, 4, T> rotate(T angle, const CR::Vec3<T> &vec) const{
				T const a = angle;
				T const c = Math::cos(a);
				T const s = Math::sin(a);

				CR::Vec3<T> axis = vec.normalize();
				float cdiff = (T)1 - c;
				CR::Vec3<T> temp(axis.x * cdiff, axis.y * cdiff, axis.z * cdiff);

				Mat<4, 4, T> rotate;
				
				rotate.mat[0*4 + 0] = c + temp.x * axis.x;
				rotate.mat[0*4 + 1] = temp.x * axis.y + s * axis.z;
				rotate.mat[0*4 + 2] = temp.x * axis.z - s * axis.y;

				rotate.mat[1*4 + 0] = temp.y * axis.x - s * axis.z;
				rotate.mat[1*4 + 1] = c + temp.y * axis.y;
				rotate.mat[1*4 + 2] = temp.y * axis.z + s * axis.x;

				rotate.mat[2*4 + 0] = temp.z * axis.x + s * axis.y;
				rotate.mat[2*4 + 1] = temp.z * axis.y - s * axis.x;
				rotate.mat[2*4 + 2] = c + temp.z * axis.z;

				Mat<4, 4, T> out;

				Mat<4, 4, T> mout(*this);
				auto &m = mout.mat;
				
				out.mat[0*4 + 0] = m[0*4 + 0] * rotate.mat[0*4 + 0] + m[1*4 + 0] * rotate.mat[0*4 + 1] + m[2*4 + 0] * rotate.mat[0*4 + 2];
				out.mat[0*4 + 1] = m[0*4 + 1] * rotate.mat[0*4 + 0] + m[1*4 + 1] * rotate.mat[0*4 + 1] + m[2*4 + 1] * rotate.mat[0*4 + 2];
				out.mat[0*4 + 2] = m[0*4 + 2] * rotate.mat[0*4 + 0] + m[1*4 + 2] * rotate.mat[0*4 + 1] + m[2*4 + 2] * rotate.mat[0*4 + 2];

				out.mat[1*4 + 0] = m[0*4 + 0] * rotate.mat[1*4 + 0] + m[1*4 + 0] * rotate.mat[1*4 + 1] + m[2*4 + 0] * rotate.mat[1*4 + 2];
				out.mat[1*4 + 1] = m[0*4 + 1] * rotate.mat[1*4 + 0] + m[1*4 + 1] * rotate.mat[1*4 + 1] + m[2*4 + 1] * rotate.mat[1*4 + 2];
				out.mat[1*4 + 2] = m[0*4 + 2] * rotate.mat[1*4 + 0] + m[1*4 + 2] * rotate.mat[1*4 + 1] + m[2*4 + 2] * rotate.mat[1*4 + 2];

				out.mat[2*4 + 0] = m[0*4 + 0] * rotate.mat[2*4 + 0] + m[1*4 + 0] * rotate.mat[2*4 + 1] + m[2*4 + 0] * rotate.mat[2*4 + 2];
				out.mat[2*4 + 1] = m[0*4 + 1] * rotate.mat[2*4 + 0] + m[1*4 + 1] * rotate.mat[2*4 + 1] + m[2*4 + 1] * rotate.mat[2*4 + 2];
				out.mat[2*4 + 2] = m[0*4 + 2] * rotate.mat[2*4 + 0] + m[1*4 + 2] * rotate.mat[2*4 + 1] + m[2*4 + 2] * rotate.mat[2*4 + 2];
		
				out.mat[3*4 + 0] = m[3*4 + 0];
				out.mat[3*4 + 1] = m[3*4 + 1];
				out.mat[3*4 + 2] = m[3*4 + 2];
				out.mat[3*4 + 3] = m[3*4 + 3];
				return out;
			}

			CR::Vec4<T> operator*(const CR::Vec4<T> &vec) const{
				CR::Vec4<T> out;
				for(int i = 0; i < 4; ++i) {
					switch(i){
						case 0: {
							out.x = (vec.x * this->mat[i + 0]) + (vec.y * this->mat[i + 4]) + (vec.z * this->mat[i + 8]) + (vec.w * this->mat[i + 12]);
						} break;
						case 1: {
							out.y = (vec.x * this->mat[i + 0]) + (vec.y * this->mat[i + 4]) + (vec.z * this->mat[i + 8]) + (vec.w * this->mat[i + 12]);
						} break;
						case 2: {
							out.z = (vec.x * this->mat[i + 0]) + (vec.y * this->mat[i + 4]) + (vec.z * this->mat[i + 8]) + (vec.w * this->mat[i + 12]);
						} break;
						case 3: {
							out.w = (vec.x * this->mat[i + 0]) + (vec.y * this->mat[i + 4]) + (vec.z * this->mat[i + 8]) + (vec.w * this->mat[i + 12]);
						} break;																		
					}
				}

				return out;
			}
		};

		static const CR::Mat<4, 4, float>MAT4Identity(MAT4FIdentity);

		struct Color {
			void operator=(float c);
			bool operator<(const CR::Color &v);
			bool operator>(const CR::Color &v);
			bool operator<=(const CR::Color &v);
			bool operator>=(const CR::Color &v);
			bool operator<(float c);
			bool operator>(float c);
			bool operator<=(float c);
			bool operator>=(float c);
			bool operator==(float c);
			bool operator==(const CR::Color &v);
			bool operator!=(float c);
			bool operator!=(const Color &v);
			CR::Color operator/(float c);
			CR::Color operator/(const CR::Color &v);
			CR::Color operator*(float c);
			CR::Color operator*(const CR::Color &v);
			CR::Color operator+(float c);
			CR::Color operator+(const CR::Color &v);
			CR::Color operator-(float c);
			CR::Color operator-(const CR::Color &v);
			CR::Color invert();
			Color(const std::string &hex);
			Color();
			Color(const CR::Color &color);
			Color(float r, float g, float b);
			Color(float r, float g, float b, float a);
			Color(float c);
			Color(float c, float a);
			void set(float c, float a);			
			void set(float c);
			void set(const std::string &hex);
			void set(float r, float g, float b, float a);
			void set(const CR::Color &color);
			float r;
			float g;
			float b;
			float a;		
			operator std::string();
			std::string str();
			std::string hex();
		};

        namespace Colors {

            static CR::Color Black(0.0f, 0.0f, 0.0f, 1.0f);
            static CR::Color White(1.0f, 1.0f, 1.0f, 1.0f);
            
            static CR::Color Red(1.0f, 0.0f, 0.0f, 1.0f);
            static CR::Color Green(0.0f, 1.0f, 0.0f, 1.0f);
            static CR::Color Blue(0.0f, 0.0f, 1.0f, 1.0f);

            static CR::Color Purple(1.0f, 0.0f, 1.0f, 1.0f);
            static CR::Color Cyan(0.0f, 1.0f, 1.0f, 1.0f);
            static CR::Color Yellow(1.0f, 1.0f, 0.0f, 1.0f);

        }

    }


#endif    
