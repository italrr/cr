#ifndef CR_TOOLS_HPP
    #define CR_TOOLS_HPP

    #include "Types.hpp"
    #include "Result.hpp"
    #include "3rdparty/Jzon.hpp"

    namespace CR {

        uint64 ticks();
        uint64 epoch();
        void sleep(uint64 t);

        namespace Core {


            namespace SupportedPlatform {
                enum SupportedPlatform : int {
                    WINDOWS,
                    LINUX,
                    UNSUPPORTED = -1
                };
                static std::string name(int plat){
                    switch(plat){
                        case SupportedPlatform::WINDOWS: 
                            return "WINDOWS";
                        case SupportedPlatform::LINUX:
                            return "LINUX";
                        default:
                            return "UNKNOWN";
                    }
                }
            }


            namespace SupportedArchitecture {
                enum SupportedArchitecture : int {
                    X86,
                    X86_64,
                    ARM,
                    ARM64,
                    UNKNOWN
                };
                static std::string name(int plat){
                    switch(plat){
                        case SupportedArchitecture::X86: 
                            return "X86";
                        case SupportedArchitecture::X86_64: 
                            return "X86_64";
                        case SupportedArchitecture::ARM: 
                            return "ARM";
                        case SupportedArchitecture::ARM64: 
                            return "ARM64";                                                                                    
                        default:
                            return "UNKNOWN";
                    }
                }
            }

            #if defined(__i386__) || defined(__i686__)
                const static int ARCH = SupportedArchitecture::X86;
            #elif defined(__x86_64__) || defined(__amd64__)
                const static int ARCH = SupportedArchitecture::X86_64;
            #elif defined(__arm__)
                const static int ARCH = SupportedArchitecture::ARM;
            #elif defined(__aarch64__)
                const static int ARCH = SupportedArchitecture::ARM64;
            #else
                const static int ARCH = SupportedArchitecture::UNKNOWN;
            #endif

            #ifdef _WIN32
                const static int PLATFORM = SupportedPlatform::WINDOWS;
            #elif __linux__
                const static int PLATFORM = SupportedPlatform::LINUX;
            #else
                const static int PLATFORM = SupportedPlatform::UNSUPPORTED;
            #endif    


            void exit(int code);
            std::vector<std::string> loadParams(int argc, char* argv[]);
        }


        namespace Hash {
            std::string md5(const std::string &path, bool partial = false);
            std::string md5(char *data, size_t size, bool partial = false);
        }
        
        namespace File {
            namespace ListType {
                enum ListType : int {
                    File,
                    Directory,
                    Any
                }; 
            }           
            std::string getCwd();

            std::string dirSep();
            std::string fixPath(const std::string &path);
            bool exists(const std::string &path);
            std::string format(const std::string &filename);
            std::string filename(const std::string &path);
            size_t size(const std::string &path);
            CR::Result list(const std::string &path, const std::string &format, int type, bool recursively, std::vector<std::string> &output);
        }

        namespace String {
            std::string toLower(const std::string &str);
            std::vector<std::string> split(const std::string &str, const std::string &sep);
            std::string format(const std::string &str, ...);
            std::string formatByes(int n);
            std::string str(float n);
            std::string str(double n);
            std::string str(uint8 n);
            std::string str(int8 n);
            std::string str(uint16 n);
            std::string str(int16 n);
            std::string str(uint32 n);
            std::string str(int32 n);
            std::string str(uint64 n);
            std::string str(int64 n);       
            std::string replaceAll(std::string subject, const std::string &search, const std::string &replace);
            
            template<typename T>
            std::string str(const CR::Mat<4, 4, T> &mat){
                std::string out;
                for(int x = 0; x < 4; ++x){
                    for(int y = 0; y < 4; ++y){
                        int i = x + y * 4;
                        out += str(static_cast<T>(mat.mat[i]))+" ";
                    }    
                }
                return out;
            } 

            template<typename T>
            std::string str(const CR::Mat<3, 3, T> &mat){
                std::string out;
                for(int x = 0; x < 3; ++x){
                    for(int y = 0; y < 3; ++y){
                        int i = x + y * 3;
                        out += str(static_cast<T>(mat.mat[i]));
                        if(x == 2){
                           out += "\n"; 
                        }else
                        if(x < 8){
                            out += ",\t";
                        }
                    }    
                }
                return out;
            }    

            template<typename T>
            std::string str(const CR::Vec2<T> &vec){
                return "(" + str(static_cast<T>(vec.x)) + ", " + str(static_cast<T>(vec.y)) + ")";
            }     

            template<typename T>
            std::string str(const CR::Vec3<T> &vec){
                return "(" + str(static_cast<T>(vec.x)) + ", " + str(static_cast<T>(vec.y)) + ", " + str(static_cast<T>(vec.z)) + ")";
            }      

            template<typename T>
            std::string str(const CR::Vec4<T> &vec){
                return "(" + str(static_cast<T>(vec.x)) + ", " + str(static_cast<T>(vec.y)) + ", " + str(static_cast<T>(vec.z)) + ", " + str(static_cast<T>(vec.w)) + ")";
            }                                   

        }


        namespace Math {
            static const float PI = 3.14159265358979323846;
            int random(int min, int max);
            float sqrt(float n);
            float sin(float n);
            float cos(float n);
            float tan(float n);
            float asin(float n);
            float acos(float n);
            float atan(float y, float x);
            float rads(float deg);
            float degs(float rads);
            CR::Mat<4, 4, float> scale(const CR::Vec3<float> &dir);
            CR::Mat<4, 4, float> perspective(float fovy, float aspRatio, float nearPlane, float farPlane);
            CR::Mat<4, 4, float> orthogonal(float left, float right, float bottom, float top, float zNear, float zFar);
            CR::Mat<4, 4, float> lookAt(const CR::Vec3<float> &to, const CR::Vec3<float> &dir, const Vec3<float> &up);  


            template<typename T>
            CR::Vec4<T> slerp(CR::Vec4<T> pStart, CR::Vec4<T> pEnd, T pFactor){
                // Taken from assimp source code
                // calc cosine theta
                T cosom = pStart.x * pEnd.x + pStart.y * pEnd.y + pStart.z * pEnd.z + pStart.w * pEnd.w;

                // adjust signs (if necessary)
                CR::Vec4<T> end = pEnd;
                if(cosom < static_cast<T>(0.0)){
                    cosom = -cosom;
                    end.x = -end.x;   // Reverse all signs
                    end.y = -end.y;
                    end.z = -end.z;
                    end.w = -end.w;
                }

                // Calculate coefficients
                T sclp, sclq;
                if((static_cast<T>(1.0) - cosom) > static_cast<T>(0.0001)){ // 0.0001 -> some epsillon{
                    // Standard case (slerp)
                    T omega, sinom;
                    omega = acos(cosom); // extract theta from dot product's cos theta
                    sinom = sin( omega);
                    sclp  = sin( (static_cast<T>(1.0) - pFactor) * omega) / sinom;
                    sclq  = sin( pFactor * omega) / sinom;
                }else{
                    // Very close, do linear interp (because it's faster)
                    sclp = static_cast<T>(1.0) - pFactor;
                    sclq = pFactor;
                }

                CR::Vec4<T> out;

                out.x = sclp * pStart.x + sclq * end.x;
                out.y = sclp * pStart.y + sclq * end.y;
                out.z = sclp * pStart.z + sclq * end.z;
                out.w = sclp * pStart.w + sclq * end.w; 

                return out;               
            }
        }

        static const size_t CR_SMALLPACKET_SIZE = 64;
		struct SmallPacket {
            SmallPacket(const CR::SmallPacket &other);
            SmallPacket();
			~SmallPacket();
            std::mutex accesMutex;
			size_t index;
			char *data;				
			void copy(const CR::SmallPacket &other);
			void reset();
			void clear();
			void setIndex(size_t index);
			std::shared_ptr<CR::Result> write(const void *data, size_t Si);
			std::shared_ptr<CR::Result> read(std::string &str);
			std::shared_ptr<CR::Result> read(void *data, size_t Si);
			std::shared_ptr<CR::Result> write(const std::string str);
			CR::SmallPacket& operator= (const CR::SmallPacket &other);		
		};        

    }


#endif