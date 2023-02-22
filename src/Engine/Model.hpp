#ifndef CR_MODEL_HPP
    #define CR_MODEL_HPP

    #include "Types.hpp"
    #include "Resource.hpp"
    #include "Graphics.hpp"
    #include "Texture.hpp"

    namespace CR {
        namespace Gfx {

            struct SkeletalFrameRotation {
                CR::Vec4<float> rotation;
                float time;
            };

            struct SkeletalFrameTranslation {
                CR::Vec3<float> translation;
                float time;
            };

            struct SkeletalFrameScaling {
                CR::Vec3<float> scaling;
                float time;
            };                        

            struct Bone {
                std::string name;
                unsigned int index;
                std::vector<CR::Gfx::SkeletalFrameRotation> rotations;
                std::vector<CR::Gfx::SkeletalFrameTranslation> translations;
                std::vector<CR::Gfx::SkeletalFrameScaling> scalings;   
            };

            struct SkeletalHierarchy {
                std::string name;
                CR::Mat<4, 4, float> mat;
                std::shared_ptr<CR::Gfx::SkeletalHierarchy> parent;
                std::vector<std::shared_ptr<CR::Gfx::SkeletalHierarchy>> children;

                CR::Mat<4, 4, float> getTransform(){
                    auto b = this->parent.get();
                    std::vector<CR::Mat<4, 4, float>> mats;
                    while(b != NULL){
                        mats.push_back(b->mat);
                        b = b->parent.get();
                    }
                    CR::Mat<4, 4, float> total = MAT4Identity;
                    for(int i = 0; i < mats.size(); ++i){
                        total = total * mats[mats.size() - 1 - i];
                    }
                    return total;
                }
                SkeletalHierarchy(){
                    parent = std::shared_ptr<CR::Gfx::SkeletalHierarchy>(NULL);
                }
            };

            struct BoneInfo {
                std::shared_ptr<CR::Gfx::SkeletalHierarchy> skeleton;
                CR::Mat<4,4, float> transf;
                CR::Mat<4,4, float> offset;
                std::string name;
            };     

            struct SkeletalAnimation {
                std::unordered_map<std::string, std::shared_ptr<CR::Gfx::Bone>> bones;
                std::vector<CR::Mat<4, 4, float>> hierarchy;
                std::string name;
                float ticksPerSecond;
                float duration;
            };

            struct ModelResource : CR::Rsc::Resource {
                std::vector<std::shared_ptr<CR::Gfx::Mesh>> mesh;
                CR::Mat<4,4, float> gInvTrans;         
                std::vector<BoneInfo> boneInfo;        
                ModelResource() : Resource() {
                    rscType = CR::Rsc::ResourceType::MODEL;
                    srcType = CR::Rsc::SourceType::FILE;
                }
                void unload();
            };         

            struct Model : CR::Rsc::Proxy {
                Model(const std::string &path);
                Model();                
                bool load(const std::string &path);
                void unload();
                CR::Gfx::ModelResource *getRsc(){
                    return static_cast<CR::Gfx::ModelResource*>(rsc.get());
                }                
            }; 
        }
    }
#endif