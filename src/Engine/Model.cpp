#include <string.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Graphics.hpp"
#include "Log.hpp"
#include "Model.hpp"


static inline CR::Mat<4,4, float> aiMat2CR(const aiMatrix4x4 &mat){
    CR::Mat<4,4, float> result;

    result.mat[0 + 0*4] = mat.a1; result.mat[0 + 1*4] = mat.a2; result.mat[0 + 2*4] = mat.a3; result.mat[0 + 3*4] = mat.a4;
    result.mat[1 + 0*4] = mat.b1; result.mat[1 + 1*4] = mat.b2; result.mat[1 + 2*4] = mat.b3; result.mat[1 + 3*4] = mat.b4;
    result.mat[2 + 0*4] = mat.c1; result.mat[2 + 1*4] = mat.c2; result.mat[2 + 2*4] = mat.c3; result.mat[2 + 3*4] = mat.c4;
    result.mat[3 + 0*4] = mat.d1; result.mat[3 + 1*4] = mat.d2; result.mat[3 + 2*4] = mat.d3; result.mat[3 + 3*4] = mat.d4;

    return result;
}

static CR::Mat<4,4, float> aiMat2CR(const aiMatrix3x3 &mat){
    CR::Mat<4,4, float> result;

    result.mat[0 + 0*4] = mat.a1; result.mat[0 + 1*4] = mat.a2; result.mat[0 + 2*4] = mat.a3; result.mat[0 + 3*4] = 0.0f;
    result.mat[1 + 0*4] = mat.b1; result.mat[1 + 1*4] = mat.b2; result.mat[1 + 2*4] = mat.b3; result.mat[1 + 3*4] = 0.0f;
    result.mat[2 + 0*4] = mat.c1; result.mat[2 + 1*4] = mat.c2; result.mat[2 + 2*4] = mat.c3; result.mat[2 + 3*4] = 0.0f;
    result.mat[3 + 0*4] = 0.0f; result.mat[3 + 1*4] = 0.0f; result.mat[3 + 2*4] = 0.0f; result.mat[3 + 3*4] = 1.0f;

    return result;
}


static void initMesh(   const unsigned int index,
                        const aiScene* pScene,
                        const aiMesh *mesh,
                        CR::Gfx::Mesh *amesh,
                        CR::Gfx::ModelResource *model,
                        std::vector<CR::Gfx::Vertex> &verts,
                        std::vector<unsigned int> &ind,
                        std::vector<CR::Gfx::TextureDependency> &textures);
static void initScene(const aiScene* pScene, CR::Gfx::ModelResource *model);
static void initBones(const unsigned int index, const aiMesh *mesh, CR::Gfx::Mesh *amesh, CR::Gfx::ModelResource *model);
static void initTexture(const aiScene* scene, const aiMesh *mesh, std::vector<CR::Gfx::TextureDependency> &textures);
static void initAnimation(const aiScene* scene, CR::Gfx::ModelResource *model);

static void initScene(const aiScene* scene, CR::Gfx::ModelResource *model){
    int nmeshes = scene->mNumMeshes;
    model->meshes.resize(nmeshes);

	unsigned int nVert = 0;
	unsigned int nIndices = 0;

    model->gInvTrans = aiMat2CR(scene->mRootNode->mTransformation).inverse();  

    // setup
	for (unsigned int i = 0; i < nmeshes; ++i) {
        // create meshes
        auto mesh = std::make_shared<CR::Gfx::Mesh>(CR::Gfx::Mesh());
        model->meshes[i] = mesh;
		model->meshes[i]->mIndex = scene->mMeshes[i]->mMaterialIndex;
        // metadata
		model->meshes[i]->nIndices = scene->mMeshes[i]->mNumFaces * 3;		
 		model->meshes[i]->bVertex = nVert;
		model->meshes[i]->bIndex = nIndices;
		nVert += scene->mMeshes[i]->mNumVertices;
		nIndices += model->meshes[i]->nIndices;
	}
    // init everything
    for(unsigned int i = 0; i < nmeshes; ++i){
        auto &mesh = model->meshes[i];
        initMesh(i, scene, scene->mMeshes[i], mesh.get(), model, mesh->vertices, mesh->indices, model->texDeps);
    }
    // init animation
    initAnimation(scene, model);
}

static void initBones(const unsigned int index, const aiMesh *mesh, CR::Gfx::Mesh *amesh, CR::Gfx::ModelResource *model){

    // for(int i = 0; i < mesh->mNumBones; ++i){
    //     auto aiBone = mesh->mBones[i];
    //     std::string name(aiBone->mName.data);
    //     int boneIndex = model->boneInfo.size();
        
    //     if(model->boneMapping.find(name) == model->boneMapping.end()){
    //         // map index
    //         model->boneMapping[name] = boneIndex;
    //         // add bone info
    //         CR::Gfx::BoneInfo boneInfo;
    //         model->boneInfo.push_back(boneInfo);   
    //     }else{
    //         boneIndex = model->boneMapping[name];      
    //     }       

    //     model->boneInfo[boneIndex].offset = aiMat2CR(aiBone->mOffsetMatrix);
    //     model->boneInfo[boneIndex].name = name;    

    //     // add weights
    //     for(int j = 0; j < aiBone->mNumWeights; ++j){
    //         unsigned int vertId = aiBone->mWeights[j].mVertexId;
    //         float weight = aiBone->mWeights[j].mWeight;
    //         amesh->vertices[vertId].setBoneData(boneIndex, weight);
    //     }
    // }
  
}

static void initTexture(const aiScene* pScene, const aiMesh *mesh, std::vector<CR::Gfx::TextureDependency> &textures){
    
    static auto process = [&](aiMaterial *mat, aiTextureType type, int rtypeName){
        for(unsigned int i = 0; i < mat->GetTextureCount(type); ++i){
            aiString str;
            mat->GetTexture(type, i, &str);
            CR::Gfx::TextureDependency dep;
            dep.role = rtypeName;
            if(!dep.texture->load(str.C_Str())){
                CR::log("[GFX] Model::load: failed to load texture dependency '%s'", str.C_Str());
            }
            textures.push_back(dep);
        }            
    };  

    aiMaterial* material = pScene->mMaterials[mesh->mMaterialIndex];  
    process(material, aiTextureType_DIFFUSE, CR::Gfx::TextureRole::DIFFUSE);
    process(material, aiTextureType_SPECULAR, CR::Gfx::TextureRole::SPECULAR);
    process(material, aiTextureType_HEIGHT, CR::Gfx::TextureRole::NORMAL);
    process(material, aiTextureType_AMBIENT, CR::Gfx::TextureRole::HEIGHT); 
};


static void initMesh(   const unsigned int index,
                        const aiScene* pScene,
                        const aiMesh *mesh,
                        CR::Gfx::Mesh *amesh,
                        CR::Gfx::ModelResource *model,
                        std::vector<CR::Gfx::Vertex> &verts,
                        std::vector<unsigned int> &ind,
                        std::vector<CR::Gfx::TextureDependency> &textures){




    // TODO: this can be parallelized

    // vertex data
    for(unsigned int i = 0; i < mesh->mNumVertices; ++i){
        CR::Gfx::Vertex vertex;
        // positions
        vertex.position.set(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        // normals
        if (mesh->HasNormals()){
            vertex.normal.set(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
        // texture coordinates
        if(mesh->HasTextureCoords(0)){
            vertex.texCoords.set(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y); 
        }else{
            vertex.texCoords.set(0.0f, 0.0f);
        }
        // tangent and bittangent
        if(mesh->HasTangentsAndBitangents()){
            // tangent
            vertex.tangent.set(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            // bitangent
            vertex.bitangent.set(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }else{
            // TODO: maybe generate these if not available?
            vertex.tangent.set(0);
            vertex.bitangent.set(0);
        }
        verts.push_back(vertex);
    }

    // indices
    for(unsigned int i = 0; i < mesh->mNumFaces; ++i){
        aiFace face = mesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; ++j){
            ind.push_back(face.mIndices[j]);        
        }
    }    

    // bones
    if(mesh->HasBones()){
        // initBones(index, mesh, amesh, model);
    }

    // textures
    initTexture(pScene, mesh, textures);

}

static void initAnimation(const aiScene* scene, CR::Gfx::ModelResource *model){
    // auto nanim = scene->mNumAnimations;

    // std::unordered_map<std::string, std::shared_ptr<CR::Gfx::SkeletalHierarchy>> skelHierarchy;
    // typedef std::shared_ptr<CR::Gfx::SkeletalAnimation> asSkAnim;
    // typedef std::shared_ptr<CR::Gfx::SkeletalHierarchy> asSkHier;

    // std::function<asSkHier(const aiNode *node, const asSkHier &parent)> readHierarchy = [&](const aiNode *node, const asSkHier &parent){
    //     std::string nodeName(node->mName.data);
    //     // add to hierarchy
    //     auto current = std::make_shared<CR::Gfx::SkeletalHierarchy>(CR::Gfx::SkeletalHierarchy());
    //     current->mat = aiMat2CR(node->mTransformation);
    //     current->parent = parent;
    //     current->name = nodeName;
    //     skelHierarchy[nodeName] = current;
    //     // iterate children
    //     for(int i = 0; i < node->mNumChildren; ++i){
    //         current->children.push_back(readHierarchy(node->mChildren[i], current));
    //     }
    //     return current;
    // };

    // std::function<void(const aiNode *node, const aiAnimation *aiAnim, asSkAnim &anim)> readAnimations = [&](const aiNode *node, const aiAnimation *aiAnim, asSkAnim &anim){
    //     std::string nodeName(node->mName.data);
    //     // find channels
    //     for(int i = 0; i < aiAnim->mNumChannels; ++i){
    //         const aiNodeAnim* pNodeAnim = aiAnim->mChannels[i]; 
    //         if(nodeName != std::string(pNodeAnim->mNodeName.data)){
    //             continue;
    //         }
    //         auto fbone = anim->bones.find(nodeName);
    //         if(fbone == anim->bones.end()){
    //             auto animbone = std::make_shared<CR::Gfx::Bone>(CR::Gfx::Bone());
    //             animbone->name = nodeName;           
    //             anim->bones[nodeName] = animbone; 
    //             fbone = anim->bones.find(nodeName);

    //         }
    //         if(fbone != anim->bones.end()){
    //             // rotation
    //             for(unsigned int j = 0; j < pNodeAnim->mNumRotationKeys; ++j){
    //                 CR::Gfx::SkeletalFrameRotation rot; 
    //                 auto airot = pNodeAnim->mRotationKeys[j];
    //                 rot.time = (float)airot.mTime;
    //                 rot.rotation = CR::Vec4<float>(airot.mValue.x, airot.mValue.y, airot.mValue.z, airot.mValue.w);
    //                 fbone->second->rotations.push_back(rot);
    //             }            
    //             // scaling
    //             for (unsigned int j = 0; j < pNodeAnim->mNumScalingKeys; ++j) {
    //                 CR::Gfx::SkeletalFrameScaling scaling;
    //                 auto aiscaling = pNodeAnim->mScalingKeys[j];
    //                 scaling.time = (float)aiscaling.mTime;
    //                 scaling.scaling = CR::Vec3<float>(aiscaling.mValue.x, aiscaling.mValue.y, aiscaling.mValue.z);
    //                 fbone->second->scalings.push_back(scaling);
    //             }            
    //             // translation 
    //             for (unsigned int j = 0; j < pNodeAnim->mNumPositionKeys; ++j) {
    //                 CR::Gfx::SkeletalFrameTranslation trans;
    //                 auto aitrans = pNodeAnim->mPositionKeys[j];
    //                 trans.time = (float)aitrans.mTime;
    //                 trans.translation = CR::Vec3<float>(aitrans.mValue.x, aitrans.mValue.y, aitrans.mValue.z);
    //                 fbone->second->translations.push_back(trans);
    //             }                 
    //         }
    //     }
    //     // iterate children
    //     for(int i = 0; i < node->mNumChildren; ++i){
    //         readAnimations(node->mChildren[i], aiAnim, anim);
    //     }
    // };

    // build hierarchy
    // model->skeletonRoot = readHierarchy(scene->mRootNode, asSkHier(NULL));
    // model->skeleton = skelHierarchy;

    // read animations
    // for(unsigned int i = 0; i < nanim; ++i){
    //     auto anim = std::make_shared<CR::Gfx::SkeletalAnimation>(CR::Gfx::SkeletalAnimation());
    //     anim->name = std::string(scene->mAnimations[i]->mName.data);
    //     anim->ticksPerSecond = scene->mAnimations[i]->mTicksPerSecond;
    //     anim->duration = scene->mAnimations[i]->mDuration;
    //     // init bones for this animation
    //     for(int i = 0; i < model->boneInfo.size(); ++i){
    //         auto &bi = model->boneInfo[i];
    //         auto bone = std::make_shared<CR::Gfx::Bone>(CR::Gfx::Bone());
    //         bone->name = bi.name;        
    //         anim->bones[bi.name] = bone; 
    //     }
    //     // read anim keys
    //     readAnimations(scene->mRootNode, scene->mAnimations[i], anim);
    //     model->animations[anim->name] = anim;
    // }

    // asign default animation (if any)
    // if(nanim > 0){
    //     model->currentAnim = model->animations.begin()->second;
    // }
}










void CR::Gfx::ModelResource::unload(){
    // if(Gfx::deleteMesh(this->textureId)){
    //     this->textureId = 0;
    //     this->rscLoaded = false;
    // }else{
    //     CR::log("[GFX] Failed to unload texture %i\n", this->textureId);
    // }
}


CR::Gfx::Model::Model(const std::string &path){

}

CR::Gfx::Model::Model(){

}

bool CR::Gfx::Model::load(const std::string &path){
    
    auto result = this->findAllocByPath(path);
    if(result == CR::Rsc::AllocationResult::PROXY){
        return true;
    }

    if(!CR::File::exists(path)){
        CR::log("[GFX] Model::load: failed to load model '%s': It doesn't exist\n", path.c_str());
        return false;        
    }

    auto rscModel = std::make_shared<CR::Gfx::ModelResource>(CR::Gfx::ModelResource());
    auto rsc = std::static_pointer_cast<CR::Gfx::ModelResource>(rscModel);

    allocate(rscModel);

    rsc->file->read(path);

    // Load model using Assimp
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path.c_str(),
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType | 
            aiProcess_Triangulate | 
            aiProcess_GenSmoothNormals | 
            aiProcess_FlipUVs |
            aiProcess_LimitBoneWeights);	

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        CR::log("[GFX] Model::load: failed to load model '%s': %s\n", path.c_str(), import.GetErrorString());
        return false;
    }

    // Load model into CR's internal
    initScene(scene, rsc.get());

    // Load mesh data into GPU
    for(int i = 0; i < rsc->meshes.size(); ++i){
        auto md = CR::Gfx::createMesh(rsc->meshes[i]->vertices, rsc->meshes[i]->indices);
        rsc->meshes[i]->md = md;
        // auto meshres = ren->generateMesh(meshes[i]->vertices, meshes[i]->indices, this->transform->useBones);
        // meshres->payload->reset();
        // meshres->payload->read(&meshes[i]->vao, sizeof(meshes[i]->vao));
        // meshres->payload->read(&meshes[i]->vbo, sizeof(meshes[i]->vbo));
        // meshres->payload->read(&meshes[i]->ebo, sizeof(meshes[i]->ebo));
    }


    return true;
}

void CR::Gfx::Model::unload(){

}