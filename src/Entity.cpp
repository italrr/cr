#include "Engine/Log.hpp"
#include "Engine/Tools.hpp"
#include "Entity.hpp"


CR::AnimFrame::AnimFrame(){
    vflip = false;
    hflip = false;
}

CR::EntityAnim::EntityAnim(){
    for(unsigned i = 0; i < CR::AnimType::TOTAL; ++i){
        this->anims.push_back(CR::AnimFrame());
    }
}

bool CR::EntityAnim::load(const std::string &path){
    if(!CR::File::exists(path)){
        CR::log("EntityAnim::load: failed to load %s: It doesn't exist\n", path.c_str());
        return false;
    }

    Jzon::Parser parser;
    auto node = parser.parseFile(path);

    if(!node.isValid()){
        CR::log("EntityAnim::load: failed to load %s: %s\n", path.c_str(), parser.getError().c_str());
        return false;
    }    

    CR::Vec2<float> atlaSize;

    this->frameSize.set(node.get("frame_size").get("x").toFloat(), node.get("frame_size").get("y").toFloat(0));
    atlaSize.set(node.get("atlas").get("width").toFloat(), node.get("atlas").get("height").toFloat(0));
    this->framerate = node.get("frame_rate").toFloat(0.0f);

    float inCoorsWidth = this->frameSize.x / atlaSize.x;
    float inCoorsHeight = this->frameSize.y / atlaSize.y;


    for(auto &it : node.get("frames")){
        auto &frame = it.second;
        unsigned type = CR::AnimType::fkeyToVal(it.first);
        auto &target = this->anims[type];
        float iX = frame.get("x").toInt();
        float iY = frame.get("y").toInt();
        unsigned n = frame.get("n").toInt();
        bool hflip = frame.get("hf").toInt();
        bool vflip = frame.get("vf").toInt();
        unsigned dir = frame.get("dir").toInt();
        target.name = it.first;
        target.vflip = vflip;
        target.hflip = hflip;
        target.dir = dir;
        target.frames.clear();
        float xCursor = 0;
        float yCursor = 0;
        for(unsigned i = 0; i < n; ++i){
            float x = xCursor   + (hflip ? iX * inCoorsWidth + inCoorsWidth : iX * inCoorsWidth);
            float y = yCursor   + (vflip ? iY * inCoorsHeight + inCoorsHeight : iY * inCoorsHeight);
            float wx = xCursor  + (hflip ? iX * inCoorsWidth : iX * inCoorsWidth + inCoorsWidth);
            float hy = yCursor  + (vflip ? iY * inCoorsHeight : iY * inCoorsHeight + inCoorsHeight);
            target.frames.push_back(CR::Rect<float>(x, y, wx, hy));
            xCursor += dir == 0 || dir == 3 ? inCoorsWidth : 0;
            yCursor += dir == 1 || dir == 3 ? inCoorsHeight : 0;
        }

    }
    CR::log("Loaded EntityAnim  %s\n", path.c_str());
    return true;
}


CR::Entity::Entity(){
    this->controlType = CR::EntityControlType::IDLE;
    this->entType = CR::EntityType::INERT;
}

void CR::Entity::build(CR::T_GENERICTYPE entType){
    // reset animations
    for(unsigned i = 0; i < 3; ++i){
        this->entStateTarget[i] = CR::EntityState::IDLE;
        this->entStateStart[i] = 0;
        this->entStateTarget[i] = 0;
    }
    this->controlType = CR::EntityControlType::IDLE;
    this->controllerId = 0;
}

void CR::Entity::setControlType(CR::T_GENERICTYPE ctype, CR::T_GENERICID cId){
    this->controllerId = cId;
    this->controlType = ctype;
}