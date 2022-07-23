#ifndef CR_GAME_BASE_HPP
    #define CR_GAME_BASE_HPP

    #include "Engine/Types.hpp"


    namespace CR {

        
        typedef uint8   T_OBJSIG;
        typedef uint8   T_ENTTYPE;
        typedef uint8   T_ENTID;
        typedef uint16  T_FRAME;
        typedef uint16  T_FRAMEORD;
        typedef uint32  T_WORLDPOS;
        typedef uint64  T_TIME;

        static const T_ENTID OBJ_ID_INVALID = 0;


        namespace ObjSigType {
            enum ObjSigType : T_OBJSIG {
                UNDEFINED = 0,
                PROP,
                ENTITY,
                INTERACTABLE,
                EVENT_TRIGGER
            };
        }

        namespace EntityType {
            enum EntityType : T_ENTTYPE {
                UNDEFINED = 0,
                PLAYER,
                NPC,
                MOB
            };
        }

        namespace FrameType {
            enum FrameType : T_FRAME {
                UNDEFINED = 0,
                PLAYER_JOINS,
                PLAYER_DISCONNECT,
                PLAYER_INPUT,
                PLAYER_CHAT_MESSAGE,
                PLAYER_USE_ITEM,
                PLAYER_COMMAND_ACTION,
                PLAYER_KICK,

                OBJECT_CREATE,
                OBJECT_DESTROY,
                OBJECT_MOVE,
                ENTITY_STATE_CHANGE,
                ENTITY_STATUS_CHANGE,
                ENTITY_DAMAGE,
                ENTITY_KILL
            };
        }

        
        struct GridLoc {
            T_WORLDPOS index; // tile
            T_WORLDPOS level; // height/altitude
            CR::Vec2<T_WORLDPOS> position; 
            CR::Vec3<float> lerpPos;
            GridLoc(){
                index = 0;
                level = 0;
                position.set(0);
                lerpPos.set(0);
            }
        };

        struct Object {
            
            T_ENTID id;
            T_OBJSIG sigType;
            std::string name;
            std::shared_ptr<GridLoc> loc;
            bool solid;

            Object(){
                this->id = OBJ_ID_INVALID; 
                this->sigType = ObjSigType::PROP;
                this->loc = std::make_shared<GridLoc>(GridLoc());
                this->name = "NO_NAME";
                this->solid = false;
            }

            virtual void onCreate(){

            }

            virtual void onStep(){

            }

            virtual void onDestroy(){

            }

            virtual void draw(){
                
            }            
        };        

        struct Frame {
            T_FRAME type;
            T_FRAMEORD order;
            T_TIME createdAt;
        };
    }

#endif