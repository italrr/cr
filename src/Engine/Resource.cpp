#include "Resource.hpp"
#include "Graphics.hpp"
#include "Log.hpp"

void CR::Rsc::Index::read(const std::string &path){
    this->path = path;
    this->size = CR::File::size(path);
    this->fname = CR::File::filename(path);
    this->loaded = true;
    this->autotag();
}

void CR::Rsc::Index::autotag(){
    tags.clear();
    if(!CR::File::exists(path)){
        return;
    }
    auto format = CR::String::toLower(CR::File::format(path));
    if(format == "png" || format == "jpg" || format == "jpeg" || format == "bmp"){
        tags.push_back("image");
    }
    if(format == "mp3" || format == "wav" || format == "ogg" || format == "flac"){
        tags.push_back("sound");
    }
    if(format == "json"){
        tags.push_back("json");
    } 
    if(format == "xml"){
        tags.push_back("xml");
    }     
    if(format == "glsl"){
        tags.push_back("shader");
    }       
    if(format == "txt"){
        tags.push_back("text");
    }    
    if(format == "obj" || format == "dae" || format == "cr3"){
        tags.push_back("model");
    }
    tags.push_back(format);
}

bool CR::Rsc::Index::isIt(const std::string &tag){
    for(int i = 0; i < tags.size(); ++i){
        if(tags[i] == tag){
            return true;
        }
    }
    return false;
}

static std::unordered_map<unsigned, std::shared_ptr<CR::Rsc::Resource>> resources;

static unsigned genId(){
    static unsigned lastId = CR::Math::random(101, 199);
    return ++lastId;
}

unsigned CR::Rsc::Proxy::findAllocByPath(const std::string &path){
    for(auto &it : resources){
        if(it.second->file->path == path){
            this->rsc = it.second;
            return CR::Rsc::AllocationResult::PROXY;
        }
    }
    return CR::Rsc::AllocationResult::ANEW;
}

bool CR::Rsc::Proxy::allocate(const std::shared_ptr<CR::Rsc::Resource> &rsc){
    this->rsc = rsc;
    this->rsc->rscId = genId();
    resources[this->rsc->rscId] = this->rsc;
    return true;
}













