#ifndef CR_RESULT_HPP
    #define CR_RESULT_HPP

    #include <memory>
    #include <functional>
    #include "Types.hpp"

    namespace CR {
        
        namespace ResultType {
            enum ResultType : int {
                noop = 0,
                Success,
                Failure,
                Waiting
            };
            static std::string name(int type){
                switch(type){
                    case ResultType::Success:
                        return "Success";
                    case ResultType::Failure:
                        return "Failure";
                    case ResultType::Waiting:
                        return "Waiting";
                    default:
                        return "Undefined";
                }
            }
        }

        struct Job;
        struct SmallPacket;
        struct  Result {
            std::shared_ptr<Result> self;
            bool done;
            int val;
            std::string msg;
            std::shared_ptr<CR::SmallPacket> payload;
            std::shared_ptr<CR::Job> job;
            std::function<void(const std::shared_ptr<CR::Result> &result)> onSuccess;
            std::function<void(const std::shared_ptr<CR::Result> &result)> onFailure;
            void *ref;
            Result(const CR::Result &result);
            Result();
            Result(int val, const std::string &msg);
            Result(int val, const std::shared_ptr<CR::SmallPacket> &payload);
            Result(int val, const std::shared_ptr<CR::Job> &job);                          
            Result(int val);
            void set(int val, const std::string &msg = "");
            void set(void *ref);
            void set(const std::shared_ptr<CR::Job> &job);
            void set(const std::shared_ptr<CR::SmallPacket> &payload);
            void setFailure(const std::string &msg = "");
            void setSuccess(const std::string &msg = "");
            void initDefault();
            void setOnSuccess(const std::function<void(const std::shared_ptr<CR::Result> &result)> &lambda);
            void setOnFailure(const std::function<void(const std::shared_ptr<CR::Result> &result)> &lambda);
            
            bool isSuccessful();      
            std::string str() const;
            operator std::string() const;
            operator bool() const;
        };

        std::shared_ptr<CR::Result> makeResult();

        std::shared_ptr<CR::Result> makeResult(int val, const std::string &msg = "");
        std::shared_ptr<CR::Result> makeResult(int val, const std::shared_ptr<CR::Job> &job);

        std::shared_ptr<CR::Result> makeResult(int val, const std::shared_ptr<CR::SmallPacket> &payload);

    }

#endif