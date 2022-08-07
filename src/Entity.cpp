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


    for(auto &it : node.get("frames")){
        auto &frame = it.second;
        unsigned type = CR::AnimType::fkeyToVal(it.first);
        auto &target = this->anims[type];
        
        float iX = frame.get("x").toInt();
        float iY = frame.get("y").toInt();
        unsigned n = frame.get("n").toInt();
        bool flipped = frame.get("f").toInt();

        target.name = it.first;
        target.flip = flipped;

        target.frames.clear();
        for(unsigned i = 0; i < n; ++i){
            float x = iX * inCoorsWidth;
            float y = iY * inCoorsHeight;
            float wx = x + inCoorsWidth;
            float hy = y + inCoorsHeight;
            target.frames.push_back(CR::Vec4<float>(x, y, wx, hy));

            if(it.first == "st_sw"){
                CR::log("%i %f %f %f %f\n", type, x, y, wx, hy);
            }

        }

    }
    CR::log("Loaded EntityAnim  %s\n", path.c_str());
    return true;
}