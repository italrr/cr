#ifndef CR_SHADER_HPP
    #define CR_SHADER_HPP

    #include "Types.hpp"
    #include "Resource.hpp"

    namespace CR {

        namespace Gfx {
            namespace ShaderAttrType {
                enum ShaderAttr : int {
                    VEC2,
                    VEC3,
                    COLOR,
                    FLOAT,
                    INT,
                    MAT4, 
                    NONE
                };
            }

            struct ShaderAttr {
                int type;
                std::string name;
                ShaderAttr(){
                    this->type = ShaderAttrType::NONE;
                }
                ShaderAttr(const std::string &name){
                    this->type = ShaderAttrType::NONE;
                    this->name = name;
                }                
                void set(const std::string &name){
                    this->name = name;
                }
            };

            struct ShaderAttrVec2 : ShaderAttr {
                CR::Vec2<float> vec;
                ShaderAttrVec2(){
                    this->type = ShaderAttrType::VEC2;
                }
                ShaderAttrVec2(const CR::Vec2<float> &vec, const std::string &name = ""){
                    this->type = ShaderAttrType::VEC2;
                    this->vec = vec;
                    if(name.length() > 0) this->name = name;
                }                
                void set(const CR::Vec2<float> &vec){
                    this->vec = vec;
                }
            };

            struct ShaderAttrVec3 : ShaderAttr {
                CR::Vec3<float> vec;
                ShaderAttrVec3(){
                    this->type = ShaderAttrType::VEC3;
                }
                ShaderAttrVec3(const CR::Vec3<float> &vec, const std::string &name = ""){
                    this->type = ShaderAttrType::VEC3;
                    this->vec = vec;
                    if(name.length() > 0) this->name = name;
                }                
                void set(const CR::Vec3<float> &vec){
                    this->vec = vec;
                }
            };                    


            struct ShaderAttrFloat : ShaderAttr {
                float n;
                ShaderAttrFloat(){
                    this->type = ShaderAttrType::FLOAT;
                }
                ShaderAttrFloat(float n, const std::string &name = ""){
                    this->type = ShaderAttrType::FLOAT;
                    this->n = n;
                    if(name.length() > 0) this->name = name;
                }                
                void set(float n){
                    this->n = n;
                }
            };    

            struct ShaderAttrInt : ShaderAttr {
                int n;
                ShaderAttrInt(){
                    this->type = ShaderAttrType::INT;
                }
                ShaderAttrInt(int n, const std::string &name = ""){
                    this->type = ShaderAttrType::INT;
                    this->n = n;
                    if(name.length() > 0) this->name = name;
                }                
                void set(int n){
                    this->n = n;
                }
            };   

            struct ShaderAttrColor : ShaderAttr {
                CR::Color color;
                ShaderAttrColor(){
                    this->type = ShaderAttrType::COLOR;
                }
                ShaderAttrColor(const CR::Color &color, const std::string &name = ""){
                    this->type = ShaderAttrType::COLOR;
                    this->color = color;
                    if(name.length() > 0) this->name = name;
                }                
                void set(const CR::Color &color){
                    this->color = color;
                }
            };   

            struct ShaderAttrMat4 : ShaderAttr {
                CR::Mat<4, 4, float> mat;
                int n;
                ShaderAttrMat4(){
                    this->type = ShaderAttrType::MAT4;
                }
                ShaderAttrMat4(const CR::Mat<4, 4, float> &mat, const std::string &name = ""){
                    this->type = ShaderAttrType::MAT4;
                    this->mat = mat;
                    if(name.length() > 0) this->name = name;
                }                
                void set(const CR::Mat<4, 4, float> &mat){
                    this->mat = mat;
                }
            };                                    

            struct Shader : CR::Resource::Resource {
                std::string vertSrc;
                std::string fragSrc;
                int shaderId;
                Shader(){
                    rscType = CR::Resource::ResourceType::SHADER;
                    shaderId = 0;
                }
                std::shared_ptr<CR::Result> unload();
                std::shared_ptr<CR::Result> load(const std::shared_ptr<CR::Indexing::Index> &file);
            };              
        }

    }


#endif