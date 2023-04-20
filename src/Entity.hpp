#ifndef CR_GAME_ENTITY_HPP
    #define CR_GAME_ENTITY_HPP

    #include "Engine/Types.hpp"
    #include "Engine/Graphics.hpp"
    #include "Engine/Texture.hpp"
    #include "Engine/Shader.hpp"
    #include "Engine/Model.hpp"

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

        namespace EntityControlType {
            enum EntityControlType : T_GENERICTYPE {
                IDLE,
                PLAYER,
                AI
            };
        }

        namespace EntityType {
            enum EntityControlType : T_GENERICTYPE {
                INERT,
                HUMANOID,
                ASSAULT_MECH,
                DRONE
            };
        }        

        namespace EntityState {
            enum EntityState : uint8 {
                IDLE,
                WALKING,
                RUNNING,
                SHOOTING_RANGED_WEAPON,
                IDLE_RANGED_WEAPON,
                WAVING_MELEE_WEAPON,
                IDLE_MELEE_WEAPON,
                WAVING_MELEE_FIST,
                USING_CONSUMABLE,
                SWAPPING_EQUIPMENT,
                IDLE_FIST,
                UHCI_WIRELESS,     // Universal Human Computer Interface 
                UHCI_WIRED,
                FACE_BLINKING,
                FACE_SPEAKING,
                FACE_GROANING,
                FACE_FAINTING
            };
        }

        namespace EntityStateSlot {
            enum EntityStateSlot : uint8 {
                HEAD,
                BODY,
                LEGS
            };
        }

        struct EntityStatus {
            bool dead;
            CR::T_GENERICATTR health;
            CR::T_GENERICATTR maxHealth;
            CR::T_GENERICATTR lv;
        };        

        struct EntityAttr {
            CR::T_GENERICATTR str;
            CR::T_GENERICATTR bstr;

            CR::T_GENERICATTR agi;
            CR::T_GENERICATTR bagi;      

            CR::T_GENERICATTR dex;
            CR::T_GENERICATTR bdex;             

            CR::T_GENERICATTR pers;
            CR::T_GENERICATTR bpers;            

            CR::T_GENERICATTR intel;
            CR::T_GENERICATTR bintel;   

            CR::T_GENERICATTR statPoints;                  
        };

        struct EntityComplexAttr {

            CR::T_GENERICATTR carry;
            CR::T_GENERICATTR maxCarry;
            
        };

        struct EntityAnim {
            CR::Gfx::Model model;
            CR::Gfx::Transform trans;
        };

        struct Entity : CR::Object {
            
            std::shared_ptr<CR::EntityAnim> anim;

            CR::T_GENERICTYPE entState[3];         // Entity can hold up 8 different states
            CR::T_AUDITORD entStateTarget[3];
            CR::T_AUDITORD entStateStart[3];

            CR::T_GENERICTYPE entType;
            CR::T_GENERICTYPE controlType;
            CR::T_GENERICID controllerId;

            Entity();
            void build(CR::T_GENERICTYPE entType);
            void setControlType(CR::T_GENERICTYPE ctype, CR::T_GENERICID cId);
            void walk(CR::T_WORLDPOS x, CR::T_WORLDPOS y);
            void draw(CR::Gfx::RenderLayer *layer);              
        };

        






    }


#endif