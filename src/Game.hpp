#ifndef CR_GAME_BASE_HPP
    #define CR_GAME_BASE_HPP

    #include "Engine/Types.hpp"
    #include "Engine/Tools.hpp"


    namespace CR {
        
        typedef uint8       T_OBJSIG;
        typedef uint8       T_ENTTYPE;
        typedef uint8       T_STATE;
        typedef uint16      T_OBJID;
        typedef uint16      T_FRAME;
        typedef uint32      T_FRAMEORD;
        typedef uint32      T_WORLDPOS;
        typedef uint64      T_TIME;
        typedef uint8       T_VERSION_SEG;

        static const T_OBJID OBJ_ID_INVALID = 0;
        static const T_VERSION_SEG GAME_VERSION[3] =  { 0, 0, 0 };


        namespace ObjSigType {
            enum ObjSigType : T_OBJSIG {
                UNDEFINED = 0,
                PROP,
                ENTITY,
                INTERACTABLE,
                TRIGGER
            };
            static std::string str(T_OBJSIG t){
                switch(t){
                    case PROP:
                        return "PROP";
                    case ENTITY:
                        return "ENTITY";
                    case INTERACTABLE:
                        return "INTERACTABLE";
                    case TRIGGER:
                        return "TRIGGER";   
                    default:
                        return "UNDEFINED";
                }
            }
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

                GAME_CLOCK_TICK,
                GAME_SIM_CREATED,
                GAME_SIM_STARTED,
                GAME_SIM_ENDED,
                GAME_SIM_MESSAGE,
                GAME_SIM_SETTING_CHANGED,
                GAME_SIM_STATE_CHANGED,

                WORLD_WEATHER_STATUS,

                CLIENT_CONNECTED,
                CLIENT_DISCONNECTED,

                PLAYER_CHAT_MESSAGE,
                PLAYER_JOINED,
                PLAYER_KICKED,

                OBJECT_CREATED,
                OBJECT_DESTROYED,
                OBJECT_MOVED,

                ENTITY_STATE_CHANGED,
                ENTITY_STATUS_CHANGED,
                ENTITY_DAMAGED,
                ENTITY_KILLED,
                ENTITY_MOVED
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

        struct World;
        struct Object {
            
            T_OBJID id;
            T_OBJSIG sigType;
            std::string name;
            std::shared_ptr<GridLoc> loc;
            bool solid;
            bool destroyed;
            CR::World *world;            

            Object(){
                this->id = OBJ_ID_INVALID; 
                this->sigType = ObjSigType::PROP;
                this->loc = std::make_shared<GridLoc>(GridLoc());
                this->name = "NO_NAME";
                this->solid = false;
                this->world = NULL;
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
            T_FRAMEORD tick;
            T_FRAMEORD order;
            T_TIME time;
            T_STATE state;
            std::string msg;
            std::vector<T_OBJID> affEnt;
            CR::SmallPacket data;
        };

        static std::shared_ptr<Frame> createFrame(T_FRAME type){
            auto frame = std::make_shared<Frame>(Frame());
            frame->type = type;
            frame->order = 0;
            frame->time = CR::ticks();
            return frame;
        }

        static std::shared_ptr<Frame> createFrame(const std::string &msg){
            auto frame = std::make_shared<Frame>(Frame());
            frame->type = FrameType::GAME_SIM_MESSAGE;
            frame->order = 0;
            frame->msg = msg;
            frame->time = CR::ticks();
            return frame;
        }        
    }

#endif