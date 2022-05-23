#include "Result.hpp"
#include "Job.hpp"
#include "Tools.hpp"

CR::Result::Result(const CR::Result &result){
    initDefault();
    this->val = result.val;
    this->msg = result.msg;
    this->payload = result.payload;
    this->job = result.job;
    this->done = result.done;
}

CR::Result::Result(){
    initDefault();    
    this->val = ResultType::noop;
    this->msg = "";
    this->job = std::shared_ptr<CR::Job>(NULL);
    this->payload = std::make_shared<CR::SmallPacket>(CR::SmallPacket());
    this->done = false;
}

CR::Result::Result(int val, const std::string &msg){
    initDefault();
    this->val = val;
    this->msg = msg;
    this->done = val != ResultType::Waiting;
    this->job = std::shared_ptr<CR::Job>(NULL);
    this->done = val != ResultType::Waiting;
    this->payload = std::make_shared<CR::SmallPacket>(SmallPacket());
}  

CR::Result::Result(int val, const std::shared_ptr<CR::SmallPacket> &payload){
    initDefault();
    this->val = val;
    this->msg = msg;
    this->done = val != ResultType::Waiting;
    this->job = std::shared_ptr<CR::Job>(NULL);
    this->payload = payload;
    this->done = val != ResultType::Waiting;
}  

CR::Result::Result(int val, const std::shared_ptr<CR::Job> &job){
    initDefault();
    this->val = val;
    this->msg = msg;
    this->done = val != ResultType::Waiting;
    this->job = job;
    this->payload = std::shared_ptr<CR::SmallPacket>(NULL);
    this->done = val != ResultType::Waiting;
    this->payload = std::make_shared<CR::SmallPacket>(SmallPacket());
}   

CR::Result::Result(int val){
    initDefault();
    this->val = val;
    this->msg = msg;
    this->done = val != ResultType::Waiting;
    this->job = std::shared_ptr<CR::Job>(NULL);
    this->done = val != ResultType::Waiting;
}   

void CR::Result::set(int val, const std::string &msg){
    this->val = val;
    this->msg = msg;
    this->done = val != ResultType::Waiting;
    switch(val){
        case ResultType::Success: {
            this->onSuccess(self);
        } break;
        case ResultType::Failure: {
            this->onFailure(self);
        } break;        
    }
}    

void CR::Result::set(void *ref){
    this->ref = ref;
}

void CR::Result::set(const std::shared_ptr<CR::Job> &job){
    this->job = job;
}

void CR::Result::set(const std::shared_ptr<CR::SmallPacket> &payload){
    this->payload = payload;
}

void CR::Result::setFailure(const std::string &msg){
    this->val = ResultType::Failure;
    this->msg = msg;
    this->done = true;
    this->onFailure(self);
}

void CR::Result::setSuccess(const std::string &msg){
    this->val = ResultType::Success;
    this->msg = msg;
    this->done = true;
    this->onSuccess(self);
}   

bool CR::Result::isSuccessful(){
    return this->val == ResultType::Success;
}         

std::string CR::Result::str() const {
    return ResultType::name(val) + (this->msg.length() > 0 ? " "+this->msg : "");
}

CR::Result::operator std::string() const {
    return str();
}

CR::Result::operator bool() const {
    return val == ResultType::Success;
}


void CR::Result::initDefault(){
    onSuccess = [](const std::shared_ptr<CR::Result> &lambda){
        return;
    };
    onFailure = [](const std::shared_ptr<CR::Result> &lambda){
        return;
    };    
}

void CR::Result::setOnSuccess(const std::function<void(const std::shared_ptr<CR::Result> &result)> &lambda){
    this->onSuccess = lambda;
}

void CR::Result::setOnFailure(const std::function<void(const std::shared_ptr<CR::Result> &result)> &lambda){
    this->onFailure = lambda;
}

std::shared_ptr<CR::Result> CR::makeResult(){
    auto result = std::make_shared<CR::Result>(CR::Result());
    result->self = result;
    return result;
}

std::shared_ptr<CR::Result> CR::makeResult(int val, const std::string &msg){
    auto result = std::make_shared<CR::Result>(CR::Result(val, msg));
    result->self = result;
    return result;
}

std::shared_ptr<CR::Result> CR::makeResult(int val, const std::shared_ptr<CR::Job> &job){
    auto r = std::make_shared<CR::Result>(CR::Result(val, ""));
    r->self = r;
    r->job = job;
    return r;
}     

std::shared_ptr<CR::Result> CR::makeResult(int val, const std::shared_ptr<CR::SmallPacket> &payload){
    auto r = std::make_shared<CR::Result>(CR::Result(val, ""));
    r->payload = payload;
    r->self = r;
    return r;
}                