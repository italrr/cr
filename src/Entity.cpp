#include "Engine/Log.hpp"
#include "Engine/Tools.hpp"
#include "Entity.hpp"


CR::AnimFrame::AnimFrame(){

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



    return true;
}