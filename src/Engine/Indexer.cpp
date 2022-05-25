#include "Indexer.hpp"
#include "Tools.hpp"
#include "Log.hpp"
#include "Job.hpp"

std::shared_ptr<CR::Result> CR::Indexing::Indexer::scan(const std::string &root){
    auto startTime = CR::ticks();
    auto result = CR::makeResult(CR::ResultType::Waiting);
    result->job = CR::spawn([&, root, result, startTime](CR::Job &ctx){
        std::vector<std::shared_ptr<Index>> toAdd;
        int found = 0;
        int bytes = 0;
        for(int i = 0; i < INDEX_STRUCTURE.size(); ++i){
            std::string path = CR::File::fixPath(root+INDEX_STRUCTURE[i]);
            std::vector<std::string> files;
            CR::File::list(path, "", CR::File::ListType::File, true, files);
            for(int j = 0; j < files.size(); ++j){
                auto res = std::make_shared<CR::Indexing::Index>(CR::Indexing::Index());
                res->read(files[j]);
                toAdd.push_back(res);
                bytes += res->size;
                ++found;
            }
        }
        std::unique_lock<std::mutex> lk(accesMutex);
        for(int i = 0; i < toAdd.size(); ++i){
            this->resources[CR::String::toLower(toAdd[i]->path)] = toAdd[i];
        }
        lk.unlock();        
        result->set(ResultType::Success);
        CR::log("[IND] indexed %i files(s) | total %s | elapsed %.2f secs\n", found, CR::String::formatByes(bytes).c_str(), (float)(CR::ticks()-startTime)/1000);
    }, CR::JobSpec(true, false, true, {"indexing", root}));
    return result;
}

std::shared_ptr<CR::Indexing::Index> CR::Indexing::Indexer::findByHash(const std::string &hash){
    std::unique_lock<std::mutex> lk(accesMutex);
    for(auto &it : this->resources){
        if(it.second->hash == hash){
            auto rs = it.second;
            lk.unlock();
            return rs;
        }
    }
    lk.unlock();
    return std::shared_ptr<CR::Indexing::Index>(NULL);
}

std::shared_ptr<CR::Indexing::Index> CR::Indexing::Indexer::findByPath(const std::string &path){
    std::unique_lock<std::mutex> lk(accesMutex);
    auto it = this->resources.find(path);
    if(it == this->resources.end()){
        lk.unlock();
        return std::shared_ptr<CR::Indexing::Index>(NULL);
    }
    auto rs = it->second;
    lk.unlock();
    return it->second;
}

std::shared_ptr<CR::Result> CR::Indexing::Indexer::asyncFindByHash(const std::string &hash, std::function<void(std::shared_ptr<Index> &file)> callback){
    auto result = CR::makeResult(CR::ResultType::Waiting);
    result->job = CR::spawn([&, hash, result, callback](CR::Job &ctx){    
        auto find = this->findByHash(hash);
        if(find.get() != NULL){
            callback(find);
            result->setSuccess();
            ctx.stop();
        }

    }, true, true, false);
    return result;
}

std::shared_ptr<CR::Result> CR::Indexing::Indexer::asyncFindByPath(const std::string &path, std::function<void(std::shared_ptr<Index> &file)> callback){
    auto result = CR::makeResult(CR::ResultType::Waiting);
    result->job = CR::spawn([&, path, result, callback](CR::Job &ctx){    
        auto find = this->findByPath(path);
        if(find.get() != NULL){
            callback(find);
            result->setSuccess();
            ctx.stop();
        }

    }, true, true, false);
    return result;
}

std::shared_ptr<CR::Result> CR::Indexing::Indexer::asyncFindManyByHash(const std::vector<std::string> &hashes, std::function<void(std::vector<std::shared_ptr<Index>> &files)> callback){
    auto result = CR::makeResult(CR::ResultType::Waiting);
    result->job = CR::spawn([&, hashes, result, callback](CR::Job &ctx){    
        std::vector<std::shared_ptr<Index>> find;
        for(int i = 0; i < hashes.size(); ++i){
            auto c = this->findByHash(hashes[i]);
            if(c.get() == NULL){
                return;
            }
            find.push_back(c);
        }
        callback(find);
        result->setSuccess();
        ctx.stop();

    }, true, true, false);
    return result;
}

std::shared_ptr<CR::Result> CR::Indexing::Indexer::asyncFindManyByPath(const std::vector<std::string> &paths, std::function<void(std::vector<std::shared_ptr<Index>> &files)> callback){
    auto result = CR::makeResult(CR::ResultType::Waiting);
    result->job = CR::spawn([&, paths, result, callback](CR::Job &ctx){    
        std::vector<std::shared_ptr<Index>> find;
        for(int i = 0; i < paths.size(); ++i){
            auto c = this->findByPath(paths[i]);
            if(c.get() == NULL){
                return;
            }
            find.push_back(c);
        }
        callback(find);
        result->setSuccess();
        ctx.stop();

    }, true, true, false);
    return result;
}


void CR::Indexing::Index::read(const std::string &path){
    this->path = path;
    this->size = File::size(path);
    this->fname = File::filename(path);
    this->hash = Hash::md5(path); // TODO: implement partial hashing
    this->autotag();   
}

void CR::Indexing::Index::autotag(){
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
    if(format == "glsl"){
        tags.push_back("shader");
    }       
}

bool CR::Indexing::Index::isIt(const std::string &tag){
    for(int i = 0; i < tags.size(); ++i){
        if(tags[i] == tag){
            return true;
        }
    }
    return false;
}