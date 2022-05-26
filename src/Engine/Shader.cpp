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

std::shared_ptr<CR::Result> CR::Gfx::Shader::load(const std::shared_ptr<CR::Indexing::Index> &file){
    auto result = CR::makeResult(CR::ResultType::Waiting);

    // infer other shader filename(have _v ? infere _f; have _f? infere _v)
    const static std::string fprex = "_f";
    const static std::string vprex = "_v";        
    std::string fragPath = file->fname.find(fprex) != -1 ? file->path : "";
    std::string vertPath = file->fname.find(vprex) != -1 ? file->path : "";
    if(fragPath.length() == 0 && vertPath.size() == 0){
        result->setFailure(CR::String::format("Shader::load: failed to find both fragment and vertex files for '%s'", file->fname.c_str()));
        return result;
    }
    if(fragPath.length() == 0){
        fragPath = vertPath;
        fragPath.replace(fragPath.find(vprex), vprex.size(), fprex);
    }
    if(vertPath.length() == 0){
        vertPath = fragPath;
        vertPath.replace(vertPath.find(fprex), fprex.size(), vprex);
    }
    
    this->fragSrc = loadSource(fragPath);
    this->vertSrc = loadSource(vertPath);

    auto r = CR::Gfx::createShader(this->vertSrc, this->fragSrc);

    result->setSuccess(String::format("Loaded Shader %s", file->path.c_str()));

    return result;
}

std::shared_ptr<CR::Result> CR::Gfx::Shader::unload(){
    auto result = CR::makeResult(CR::ResultType::Waiting);
    if(this->shaderId == 0){
        result->setSuccess();
        return result;
    }else{
        if(Gfx::deleteShader(this->shaderId)){
            this->shaderId = 0;
            this->rscLoaded = false;
            result->setSuccess();
        }else{
            result->setFailure();
        }
    }
    return result;
}