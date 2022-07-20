#ifndef CR_ENTITY_HPP
    #define CR_ENTITY_HPP

    #include "Engine/Types.hpp"
    #include "Engine/Graphics.hpp"
    #include "Engine/Texture.hpp"
    #include "Engine/Shader.hpp"

    #include "Game.hpp"

    namespace CR {
        
        namespace CharFace {
            enum CharFace : unsigned {
                DOWN = 0,
                DOWN_LEFT,
                LEFT,
                UP_LEFT,
                UP,
                UP_RIGHT,
                RIGHT,
                DOWN_RIGHT
            };
            static std::string inline str(unsigned f){
                switch(f){
                    case CharFace::DOWN: {
                        return "DOWN";
                    } break;
                    case CharFace::DOWN_LEFT: {
                        return "DOWN_LEFT";
                    } break;
                    case CharFace::LEFT: {
                        return "LEFT";
                    } break;
                    case CharFace::UP_LEFT: {
                        return "UP_LEFT";
                    } break;
                    case CharFace::UP: {
                        return "UP";
                    } break;
                    case CharFace::UP_RIGHT: {
                        return "UP_RIGHT";
                    } break;
                    case CharFace::RIGHT: {
                        return "RIGHT";
                    } break;     
                    case CharFace::DOWN_RIGHT: {
                        return "DOWN_RIGHT";
                    } break;    
                    default: {
                        return "[NO_FACE]";
                    }                                                                            
                }
            }
            static unsigned inline faceFromAngle(float deg){
                if(deg <= 22.5 && deg > 337.5 && deg >= 0){
                    return CharFace::RIGHT;
                }else
                if(deg > 22.5 && deg <= 67.5){
                    return CharFace::UP_RIGHT;
                }else
                if(deg > 67.5 && deg <= 112.5){
                    return CharFace::UP;
                }else
                if(deg > 112.5 && deg <= 157.5){
                    return CharFace::UP_LEFT;
                }else
                if(deg > 157.5 && deg <= 202.5){
                    return CharFace::LEFT;
                }else
                if(deg > 202.5 && deg <= 247.5){
                    return CharFace::DOWN_LEFT;
                }else
                if(deg > 247.5 && deg <= 292.5){
                    return CharFace::DOWN;
                }else
                if(deg > 292.5 && deg <= 337.5){
                    return CharFace::DOWN_RIGHT;
                }else{
                    // DEFAULT
                    return CharFace::DOWN;
                }                    
            }
        }


        struct Entity : CR::Object {



        };








    }


#endif