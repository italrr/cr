#include <cmath>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.hpp"

static CR::Mat<4,4, float> aiMat2CR(const aiMatrix4x4 &mat){
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

struct TextureDep {
    unsigned tex;
    int role;

};

static void initMesh(   const unsigned int index,
                        const aiScene* pScene,
                        const aiMesh *mesh,
                        CR::Gfx::Model *model);

static void initScene(const aiScene* pScene, CR::Gfx::Model *model);
static void initTexture(const unsigned int index, const aiScene* scene, const aiMesh *mesh, CR::Gfx::Mesh *crmesh);



static void initBones(const unsigned int index, const aiMesh *mesh, CR::Gfx::Mesh *amesh, CR::Gfx::Model *model);
static void initAnimation(const aiScene* scene, CR::Gfx::Model *model);





static void initScene(const aiScene* scene, CR::Gfx::Model *model){
    auto rsc = model->getRsc();
    int nmeshes = scene->mNumMeshes;
    rsc->mesh.resize(nmeshes);

	unsigned int nVert = 0;
	unsigned int nIndices = 0;

    rsc->gInvTrans = aiMat2CR(scene->mRootNode->mTransformation).inverse();  

    // setup
	for (unsigned int i = 0; i < nmeshes; ++i) {
        // create meshes
        auto mesh = std::make_shared<CR::Gfx::Mesh>(CR::Gfx::Mesh());
        rsc->mesh[i] = mesh;
		rsc->mesh[i]->mIndex = scene->mMeshes[i]->mMaterialIndex;
        // metadata
		rsc->mesh[i]->nIndices = scene->mMeshes[i]->mNumFaces * 3;		
 		rsc->mesh[i]->bVertex = nVert;
		rsc->mesh[i]->bIndex = nIndices;
		nVert += scene->mMeshes[i]->mNumVertices;
		nIndices += rsc->mesh[i]->nIndices;
	}
    // init everything
    for(unsigned int i = 0; i < nmeshes; ++i){
        auto &mesh = rsc->mesh[i];
        initMesh(i, scene, scene->mMeshes[i], model);
        initTexture(i, scene, scene->mMeshes[i], mesh.get());
    }
    // init animation
    // initAnimation(scene, model);
}

static void initBones(const unsigned int index, const aiMesh *mesh, CR::Gfx::Mesh *amesh, CR::Gfx::Model *model){

    // auto rsc = model->getRsc();

    // for(int i = 0; i < mesh->mNumBones; ++i){
    //     auto aiBone = mesh->mBones[i];
    //     std::string name(aiBone->mName.data);

    //     int boneIndex = rsc->boneInfo.size();
        
    //     if(rsc->boneMapping.find(name) == rsc->boneMapping.end()){
    //         // map index
    //         rsc->boneMapping[name] = boneIndex;
    //         // add bone info
    //         CR::Gfx::BoneInfo boneInfo;
    //         rsc->boneInfo.push_back(boneInfo);   
    //     }else{
    //         boneIndex = rsc->boneMapping[name];      
    //     }       

    //     rsc->boneInfo[boneIndex].offset = aiMat2CR(aiBone->mOffsetMatrix);
    //     rsc->boneInfo[boneIndex].name = name;    

    //     // add weights
    //     for(int j = 0; j < aiBone->mNumWeights; ++j){
    //         unsigned int vertId = aiBone->mWeights[j].mVertexId;
    //         float weight = aiBone->mWeights[j].mWeight;
    //         amesh->vertices[vertId].setBoneData(boneIndex, weight);
    //     }
    // }
  
}

static void initTexture(const unsigned int index, const aiScene* pScene, const aiMesh *mesh, CR::Gfx::Mesh *crmesh){

    static auto process = [&](aiMaterial *mat, aiTextureType type, int rtypeName){
        for(unsigned int i = 0; i < mat->GetTextureCount(type); ++i){
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string fpath = CR::String::format("data/texture/%s", str.C_Str());
            if(CR::File::exists(fpath)){
                crmesh->textures[rtypeName].load(fpath);
            }
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
                        CR::Gfx::Model *model){



    std::vector<float> vpos;
    std::vector<float> vnorm;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;

    // vertex data
    for(unsigned int i = 0; i < mesh->mNumVertices; ++i){
        // positions
        vpos.push_back(mesh->mVertices[i].x);
        vpos.push_back(mesh->mVertices[i].y);
        vpos.push_back(mesh->mVertices[i].z);
        // normals
        if (mesh->HasNormals()){
            vnorm.push_back(mesh->mNormals[i].x);
            vnorm.push_back(mesh->mNormals[i].y);
            vnorm.push_back(mesh->mNormals[i].z);
        }
        // texture coordinates
        if(mesh->HasTextureCoords(0)){
            texCoords.push_back(mesh->mTextureCoords[0][i].x);          
            texCoords.push_back(mesh->mTextureCoords[0][i].y);          
        }else{
            texCoords.push_back(0.0f);  
            texCoords.push_back(0.0f);  
        }
    }

    // indices
    for(unsigned int i = 0; i < mesh->mNumFaces; ++i){
        aiFace face = mesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; ++j){
            indices.push_back(face.mIndices[j]);        
        }
    }    




    // bones
    // if(mesh->HasBones()){
    //     initBones(index, mesh, amesh, model);
    // }

    // // textures
    // initTexture(pScene, mesh, textures);

    auto rsc = model->getRsc();
    auto md = CR::Gfx::createMesh(vpos, texCoords, indices, CR::Gfx::VertexStoreType::STATIC, CR::Gfx::VertexStoreType::STATIC);
    rsc->mesh[index]->md = md;
}








void CR::Gfx::ModelResource::unload(){

}




CR::Gfx::Model::Model(const std::string &path){

}

CR::Gfx::Model::Model(){

}

bool CR::Gfx::Model::load(const std::string &path){

    // auto result = this->findAllocByPath(path);
    // if(result == CR::Rsc::AllocationResult::PROXY){
    //     return true;
    // }

    if(!CR::File::exists(path)){
        CR::log("[GFX] Model::load: failed to load model '%s': file doesn't exist\n", path.c_str());
        return false;  
    }

    auto rscModel = std::make_shared<CR::Gfx::ModelResource>(CR::Gfx::ModelResource());
    auto rsc = std::static_pointer_cast<CR::Gfx::ModelResource>(rscModel);

    allocate(rscModel);

    rscModel->file->read(path);

    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path.c_str(),
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType | 
            aiProcess_Triangulate | 
            aiProcess_GenSmoothNormals | 
            aiProcess_FlipUVs |
            aiProcess_LimitBoneWeights);	

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        CR::log("Model::load: failed to load model '%s': %s\n", CR::File::filename(path).c_str(), import.GetErrorString());
        return false;
    }
    

    initScene(scene, this);

    rscModel->rscLoaded = true;
    CR::log("[GFX] Loaded Model %s\n", path.c_str());

    return true;
}

void CR::Gfx::Model::unload(){

}