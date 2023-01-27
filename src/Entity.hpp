#ifndef CR_GAME_ENTITY_HPP
    #define CR_GAME_ENTITY_HPP

    #include "Engine/Types.hpp"
    #include "Engine/Graphics.hpp"
    #include "Engine/Texture.hpp"
    #include "Engine/Shader.hpp"

    #include "Game.hpp"

    namespace CR {
        
        namespace AnimType {
            enum AnimType : uint8 {
                STAND_SOUTH = 0,
                STAND_SOUTH_WEST,
                STAND_WEST,
                STAND_NORTH_WEST,
                STAND_NORTH,
                STAND_NORTH_EAST,
                STAND_EAST,
                STAND_SOUTH_EAST,

                SIT_SOUTH,
                SIT_SOUTH_WEST,
                SIT_WEST,
                SIT_NORTH_WEST,
                SIT_NORTH,
                SIT_NORTH_EAST,
                SIT_EAST,
                SIT_SOUTH_EAST,
                
                WALKING_SOUTH,
                WALKING_SOUTH_WEST,
                WALKING_WEST,
                WALKING_NORTH_WEST,
                WALKING_NORTH,
                WALKING_NORTH_EAST,
                WALKING_EAST,
                WALKING_SOUTH_EAST,

            };
            static const unsigned TOTAL = 24;
            static uint8 fkeyToVal(const std::string &key){
                if(key == "st_s"){
                    return CR::AnimType::STAND_SOUTH;
                }else
                if(key == "st_sw"){
                    return CR::AnimType::STAND_SOUTH_WEST;
                }else
                if(key == "st_w"){
                    return CR::AnimType::STAND_WEST;
                }else
                if(key == "st_nw"){
                    return CR::AnimType::STAND_NORTH_WEST;
                }else
                if(key == "st_n"){
                    return CR::AnimType::STAND_NORTH;
                }else
                if(key == "st_ne"){
                    return CR::AnimType::STAND_NORTH_EAST;
                }else
                if(key == "st_e"){
                    return CR::AnimType::STAND_EAST;
                }else
                if(key == "st_se"){
                    return CR::AnimType::STAND_SOUTH_EAST;
                }else
                if(key == "s_s"){
                    return CR::AnimType::SIT_SOUTH;
                }else
                if(key == "s_sw"){
                    return CR::AnimType::SIT_SOUTH_WEST;
                }else
                if(key == "s_w"){
                    return CR::AnimType::SIT_WEST;
                }else
                if(key == "s_nw"){
                    return CR::AnimType::SIT_NORTH_WEST;
                }else
                if(key == "s_n"){
                    return CR::AnimType::SIT_NORTH;
                }else
                if(key == "s_ne"){
                    return CR::AnimType::SIT_NORTH_EAST;
                }else
                if(key == "s_e"){
                    return CR::AnimType::SIT_EAST;
                }else
                if(key == "s_se"){
                    return CR::AnimType::SIT_SOUTH_EAST;
                }else
                if(key == "w_s"){
                    return CR::AnimType::WALKING_SOUTH;
                }else
                if(key == "w_sw"){
                    return CR::AnimType::WALKING_SOUTH_WEST;
                }else
                if(key == "w_w"){
                    return CR::AnimType::WALKING_WEST;
                }else
                if(key == "w_nw"){
                    return CR::AnimType::WALKING_NORTH_WEST;
                }else
                if(key == "w_n"){
                    return CR::AnimType::WALKING_NORTH;
                }else
                if(key == "w_ne"){
                    return CR::AnimType::WALKING_NORTH_EAST;
                }else
                if(key == "w_e"){
                    return CR::AnimType::WALKING_EAST;
                }else
                if(key == "w_se"){
                    return CR::AnimType::WALKING_SOUTH_EAST;
                }else{
                    return CR::AnimType::STAND_SOUTH;
                }               
            }
        }

        struct AnimFrame {
            std::string name;
            bool vflip;
            bool hflip;
            unsigned dir;
            std::vector<CR::Rect<float>> frames;
            AnimFrame();
        };

        struct EntityAnim {
            float framerate;
            CR::Vec2<float> frameSize;
            std::shared_ptr<CR::Gfx::Texture> atlas;            
            std::vector<CR::AnimFrame> anims;
            EntityAnim();
            bool load(const std::string &path);
        };


        namespace EntityControlType {
            enum EntityControlType : T_GENERICTYPE {
                IDLE,
                PLAYER,
                AI
            };
        }

        struct Entity : CR::Object {

            CR::T_GENERICTYPE controlType;
            CR::T_OBJID controllerId;

            Entity();
            
        };

        






    }


#endif