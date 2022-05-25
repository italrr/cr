#ifndef CR_INDEXING_HPP
    #define CR_INDEXING_HPP

    #include <unordered_map>
    #include <memory>
    #include <mutex>

    #include "Types.hpp"
    #include "Result.hpp"    

    namespace CR {
        
        namespace Indexing {

            struct Index {
                std::string hash;
                std::string fname;
                std::string fpath;
                std::string path; //abs
                size_t size;
                std::vector<std::string> tags;
                void read(const std::string &path);
                bool isIt(const std::string &tag);
                void autotag();
            };

            static const std::vector<std::string> INDEX_STRUCTURE = {
                "texture/", "font/", "model/", "shader/", "cfg/"
            };

            struct Indexer {
                std::string path;
                std::mutex accesMutex;
                std::unordered_map<std::string, std::shared_ptr<Index>> resources;
                std::shared_ptr<CR::Result> scan(const std::string &root);
                
                std::shared_ptr<Index> findByHash(const std::string &hash);
                std::shared_ptr<Index> findByPath(const std::string &path);

                std::shared_ptr<CR::Result> asyncFindByHash(const std::string &hash, std::function<void(std::shared_ptr<Index> &file)> callback);
                std::shared_ptr<CR::Result> asyncFindByPath(const std::string &path, std::function<void(std::shared_ptr<Index> &file)> callback);

                std::shared_ptr<CR::Result> asyncFindManyByHash(const std::vector<std::string> &hashes, std::function<void(std::vector<std::shared_ptr<Index>> &files)> callback);
                std::shared_ptr<CR::Result> asyncFindManyByPath(const std::vector<std::string> &paths, std::function<void(std::vector<std::shared_ptr<Index>> &files)> callback);
            };
        }

    }

#endif