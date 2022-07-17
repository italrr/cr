#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"
#include "Log.hpp"
#include "Graphics.hpp"
#include "Texture.hpp"


void CR::Gfx::TextureResource::unload(){
    if(Gfx::deleteTexture2D(this->textureId)){
        this->textureId = 0;
        this->rscLoaded = false;
    }else{
        CR::log("[GFX] Failed to unload texture %i\n", this->textureId);
    }
}

bool CR::Gfx::Texture::load(const std::string &path, const std::string &mask){
    return false;
}

bool CR::Gfx::Texture::load(const std::string &path){
    auto result = this->findAllocByPath(path);
    if(result == CR::Rsc::AllocationResult::PROXY){
        return true;
    }

    if(!CR::File::exists(path)){
        CR::log("[GFX] Texture::load: failed to load texture '%s': It doesn't exist\n", path.c_str());
        return false;        
    }

    auto rscTexture = std::make_shared<CR::Gfx::TextureResource>(CR::Gfx::TextureResource());
    auto rsc = std::static_pointer_cast<CR::Gfx::TextureResource>(rscTexture);

    allocate(rscTexture);

    rscTexture->file->read(path);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0); 

    if(!data){
        CR::log("[GFX] Texture::load: failed to load texture '%s': incompatible format?\n", path.c_str());
        return false;
    }

    int format;
    if (nrChannels == 1)
        format = ImageFormat::RED;
    else if (nrChannels == 3)
        format = ImageFormat::RGB;
    else if (nrChannels == 4){
        format = ImageFormat::RGBA;
    }

    rscTexture->size.set(width, height);
    rscTexture->channels = nrChannels;

    auto r = CR::Gfx::createTexture2D(data, width, height, format);

    rscTexture->textureId = r;
    rscTexture->rscLoaded = true;
    CR::log("[GFX] Loaded Texture %s | Size %ix%i | Channels %i\n", path.c_str(), rscTexture->size.x, rscTexture->size.y, nrChannels);

    return true;    

}

CR::Gfx::Texture::Texture(const std::string &path){
    load(path);
}

CR::Gfx::Texture::Texture(){

}

void CR::Gfx::Texture::unload(){
    this->rsc->unload();
}