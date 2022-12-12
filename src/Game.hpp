#ifndef CR_GAME_BASE_HPP
    #define CR_GAME_BASE_HPP

    #include "Engine/Types.hpp"
    #include "Engine/Tools.hpp"


    namespace CR {
        
        typedef uint8       T_OBJSIG;
        typedef uint8       T_ENTTYPE;
        typedef uint8       T_GENERICTYPE;
        typedef uint8       T_STATE;
        typedef uint16      T_OBJID;
        typedef uint16      T_AUDIT;
        typedef uint32      T_GENERICID;
        typedef uint32      T_AUDITORD;
        typedef uint32      T_WORLDPOS;
        typedef uint64      T_TIME;
        typedef uint8       T_VERSION_SEG;

        static const T_OBJID OBJ_ID_INVALID = 0;
        static const T_VERSION_SEG GAME_VERSION[3] =  { 0, 0, 0 }; // Major, Minor, Bugfix


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

        namespace AuditType {
            enum AuditType : T_AUDIT {
                UNDEFINED = 0,

                GAME_CLOCK_TICK,
                GAME_SIM_CREATED,
                GAME_SIM_STARTED,
                GAME_SIM_ENDED,
                GAME_SIM_MESSAGE,
                GAME_SIM_SETTING_CHANGED,
                GAME_SIM_STATE_CHANGED,
                GAME_SIM_WEATHER_STATUS,
                GAME_SIM_START_OF_FRAME,
                GAME_SIM_END_OF_FRAME,
                GAME_SIM_NEXT_FRAME,

                CLIENT_CONNECTED,
                CLIENT_DISCONNECTED,
                CLIENT_DROPPED,

                PLAYER_CHAT_MESSAGE,
                PLAYER_JOINED,
                PLAYER_LEFT,
                PLAYER_GRANT_ENTITY_CONTROL,

                OBJECT_CREATED,
                OBJECT_DESTROYED,
                OBJECT_MOVED,

                ENTITY_STATE_CHANGED,
                ENTITY_STATUS_CHANGED,
                ENTITY_DAMAGED,
                ENTITY_KILLED,
                ENTITY_MOVED
            };
            static std::string str(CR::T_AUDIT type){
                switch(type){
                    
                    default:
                        return "UNDEFINED";
                }
            }
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

        struct Audit { // Audit are terms used interchangeably 
            T_AUDIT type;                   // Type of Audit
            T_AUDITORD tick;                // Which tick it belongs to
            T_AUDITORD order;               // Order in which the Audit was applied during a tick
            T_TIME time;                    // What time it happened. This is probably useless.
            T_STATE state;                  // The state of the world when this Audit was applied
            std::string msg;                // Used to store any text pertinent to the Audit
            std::vector<T_OBJID> affEnt;    // Defines which entities of the world are affected by this Audit. It's usually one
            CR::SmallPacket data;           // A data object used to store specificities of each type of Audit
        };

        static std::shared_ptr<Audit> createAudit(T_AUDIT type){
            auto audit = std::make_shared<Audit>(Audit());
            audit->type = type;
            audit->order = 0;
            audit->time = CR::ticks();
            return audit;
        }

        static std::shared_ptr<Audit> createAudit(const std::string &msg){
            auto audit = std::make_shared<Audit>(Audit());
            audit->type = AuditType::GAME_SIM_MESSAGE;
            audit->order = 0;
            audit->msg = msg;
            audit->time = CR::ticks();
            return audit;
        }        
    }

#endif