#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"
#include "Log.hpp"
#include "Graphics.hpp"
#include "Texture.hpp"

std::shared_ptr<CR::Result> CR::Gfx::Texture::load(const std::shared_ptr<CR::Indexing::Index> &file){
    auto result = CR::makeResult(CR::ResultType::Waiting);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(file->path.c_str(), &width, &height, &nrChannels, 0); 

    if(!data){
        result->setFailure(String::format("Texture::load: failed to load texture '%s': incompatible format?", file->fname.c_str()));
        return result;
    }

    int format;
    if (nrChannels == 1)
        format = ImageFormat::RED;
    else if (nrChannels == 3)
        format = ImageFormat::RGB;
    else if (nrChannels == 4)
        format = ImageFormat::RGBA;

    this->size.set(width, height);
    this->channels = nrChannels;

    auto r = CR::Gfx::createTexture2D(data, width, height, format);

    result->setSuccess(String::format("Loaded Texture %s | Size %ix%i | Channels %i", file->path.c_str(), width, height, nrChannels));

    return result;
}

std::shared_ptr<CR::Result> CR::Gfx::Texture::unload(){
    auto result = CR::makeResult(CR::ResultType::Waiting);

    if(textureId == 0){
        result->setSuccess();
        return result;
    }else{
        if(Gfx::deleteTexture2D(this->textureId)){
            textureId = 0;
            result->setSuccess();
        }else{
            result->setFailure();
        }
    }


    return result;
}