#include <fstream>

#include "Graphics.hpp"
#include "Log.hpp"
#include "Shader.hpp"

static std::string loadSource(const std::string &path){
    std::string src;
    std::ifstream file;
    std::string buffer;
    file.open(path.c_str());
    while (getline(file,buffer)){
        src += buffer+"\n";
    }
    return src;
}


void CR::Gfx::ShaderResource::unload(){
    if(Gfx::deleteShader(this->shaderId)){
        this->shaderId = 0;
        this->rscLoaded = false;
    }else{
        CR::log("[GFX] Failed to unload texture %i\n", this->shaderId);
    }
}

void CR::Gfx::Shader::findAttrs(const std::vector<std::string> &list){

    auto rsc = std::static_pointer_cast<CR::Gfx::ShaderResource>(this->rsc);
    

    for(unsigned i = 0; i < list.size(); ++i){
        this->shAttrs[list[i]] = CR::Gfx::findShaderAttr(rsc->shaderId, list[i].c_str());
    }
}

bool CR::Gfx::Shader::load(const std::string &frag, const std::string &vert){

    auto result = this->findAllocByPath(frag);
    if(result == CR::Rsc::AllocationResult::PROXY){
        return true;
    }    

    if(!CR::File::exists(frag) || !CR::File::exists(vert)){
        CR::log("[GFX] Shader::load: failed to load Shader '%s': file doesn't exist\n", frag.c_str());
        return false;        
    }    

    auto rscShader = std::make_shared<CR::Gfx::ShaderResource>(CR::Gfx::ShaderResource());
    auto rsc = std::static_pointer_cast<CR::Gfx::ShaderResource>(rscShader);

    allocate(rscShader);


    const std::string &fragPath = frag;
    const std::string &vertPath = vert;

    rsc->fragSrc = loadSource(fragPath);
    rsc->vertSrc = loadSource(vertPath);

    auto r = CR::Gfx::createShader(rsc->vertSrc, rsc->fragSrc);
    if(r == 0){
        CR::log("[GFX] Shader::load: failed to compile shader\n");
        return false;
    }
    rsc->shaderId = r;

    CR::log("[GFX] Loaded Shader | frag %s | vert %s\n", fragPath.c_str(), vertPath.c_str());

    return true;
}

CR::Gfx::Shader::Shader(){

}

void CR::Gfx::Shader::unload(){
    this->rsc->unload();
}