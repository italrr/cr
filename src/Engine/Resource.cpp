#include "Resource.hpp"
#include "Graphics.hpp"
#include "Log.hpp"

std::shared_ptr<CR::Result> CR::Resource::ResourceManager::load(const std::shared_ptr<CR::Indexing::Index> &file, const std::shared_ptr<Resource> &holder){
    if(file.get() == NULL){
        return  CR::makeResult(CR::ResultType::Failure, "no file provided");
    }
    std::unique_lock<std::mutex> lk(accesMutex);
    holder->setId(++lastId);
    holder->setLoaded(true);
    holder->setFile(file);
    this->resources[holder->rscId] = holder;        
    lk.unlock();
    auto r = holder->load(file);
    r->payload->write(&holder->rscId, sizeof(holder->rscId));

    if(r->done && r->msg.length() > 0){
        CR::log("%s\n", r->msg.c_str());
    }

    return r;
}

std::shared_ptr<CR::Result> CR::Resource::ResourceManager::load(const std::string &path, const std::shared_ptr<Resource> &holder){
    auto indexer = CR::getIndexer();
    auto file = indexer->findByPath(path);
    if(file.get() == NULL){
        return CR::makeResult(CR::ResultType::Failure, "'"+path+"' doesn't exist");
    }
    return load(file, holder);
}

std::shared_ptr<CR::Resource::Resource> CR::Resource::ResourceManager::findByPath(const std::string &hash){
    auto resource = std::shared_ptr<CR::Resource::Resource>(NULL);
    std::unique_lock<std::mutex> lk(accesMutex);
    for(auto &it : resources){
        if(it.second->file->hash == hash){
            resource = it.second;
            break;
        }
    }
    lk.unlock();
    return resource;
}

std::shared_ptr<CR::Resource::Resource> CR::Resource::ResourceManager::findByHash(const std::string &hash){
    auto resource = std::shared_ptr<CR::Resource::Resource>(NULL);
    std::unique_lock<std::mutex> lk(accesMutex);
    for(auto &it : resources){
        if(it.second->file->hash == hash){
            resource = it.second;
            break;
        }
    }
    lk.unlock();
    return resource;
}


std::shared_ptr<CR::Resource::Resource> CR::Resource::ResourceManager::findById(int id){
    auto resource = std::shared_ptr<CR::Resource::Resource>(NULL);
    std::unique_lock<std::mutex> lk(accesMutex);
    auto it = resources.find(id);
    if(it != resources.end()){
        resource = it->second;
    }
    lk.unlock();
    return resource;
}

CR::Resource::ResourceManager::ResourceManager(){
    lastId = CR::Math::random(101, 126);
}