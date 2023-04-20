#ifndef CR_GAME_BASE_HPP
    #define CR_GAME_BASE_HPP

    #include <unordered_map>

    #include "Engine/Types.hpp"
    #include "Engine/Tools.hpp"
    #include "Engine/Input.hpp"
    #include "Engine/Graphics.hpp"


    namespace CR {
        
        typedef uint16      T_GENERICATTR;
        typedef uint8       T_OBJSIG;
        typedef uint8       T_ENTTYPE;
        typedef uint8       T_GENERICTYPE;
        typedef uint8       T_STATE;
        typedef uint16      T_OBJID;
        typedef uint16      T_AUDIT;
        typedef uint32      T_GENERICID;
        typedef uint32      T_AUDITORD;
        typedef int32       T_WORLDPOS;
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
                OBJECT_MOVING,
                OBJECT_MOVED,

                ENTITY_STATE_CHANGED,
                ENTITY_STATUS_CHANGED,
                ENTITY_DAMAGED,
                ENTITY_KILLED
            };
            static std::string str(CR::T_AUDIT type){
                switch(type){
                    
                    default:
                        return "UNDEFINED";
                }
            }
        }

        namespace Direction {
            enum Direction : unsigned {
                SOUTH = 0,
                SOUTH_WEST,
                WEST,
                NORTH_WEST,
                NORTH,
                NORTH_EAST,
                EAST,
                SOUTH_EAST
            };
            static std::string inline str(unsigned f){
                switch(f){
                    case Direction::SOUTH: {
                        return "SOUTH";
                    } break;
                    case Direction::SOUTH_WEST: {
                        return "SOUTH_WEST";
                    } break;
                    case Direction::WEST: {
                        return "WEST";
                    } break;
                    case Direction::NORTH_WEST: {
                        return "NORTH_WEST";
                    } break;
                    case Direction::NORTH: {
                        return "NORTH";
                    } break;
                    case Direction::NORTH_EAST: {
                        return "NORTH_EAST";
                    } break;
                    case Direction::EAST: {
                        return "EAST";
                    } break;     
                    case Direction::SOUTH_EAST: {
                        return "SOUTH_EAST";
                    } break;    
                    default: {
                        return "[NO_FACE]";
                    }                                                                            
                }
            }
            static unsigned inline faceFromAngle(float deg){
                if(deg <= 22.5 && deg > 337.5 && deg >= 0){
                    return Direction::EAST;
                }else
                if(deg > 22.5 && deg <= 67.5){
                    return Direction::NORTH_EAST;
                }else
                if(deg > 67.5 && deg <= 112.5){
                    return Direction::NORTH;
                }else
                if(deg > 112.5 && deg <= 157.5){
                    return Direction::NORTH_WEST;
                }else
                if(deg > 157.5 && deg <= 202.5){
                    return Direction::WEST;
                }else
                if(deg > 202.5 && deg <= 247.5){
                    return Direction::SOUTH_WEST;
                }else
                if(deg > 247.5 && deg <= 292.5){
                    return Direction::SOUTH;
                }else
                if(deg > 292.5 && deg <= 337.5){
                    return Direction::SOUTH_EAST;
                }else{
                    // DEFAULT
                    return Direction::SOUTH;
                }                    
            }
        }
        

        namespace PlayerKey { 
            enum PlayerKey : uint8 {
                UP = 0,
                DOWN,
                RIGHT,
                LEFT,
                k1,
                k2,
                k3,
                k4,
                k5,
                k6,
                k7,
                k8
            };
            static const uint8 total = 12;
        }

        namespace PlayerPressType {
            enum PlayerPressType : uint8 {
                None = 0,
                Pressed,
                Released
            };
        }

        static const std::unordered_map<int, uint8> PlayerInputDefaultMapping = { 
            { CR::Input::Key::W,            PlayerKey::UP },
            { CR::Input::Key::S,            PlayerKey::DOWN },
            { CR::Input::Key::A,            PlayerKey::LEFT },
            { CR::Input::Key::D,            PlayerKey::RIGHT },
            { CR::Input::Key::ONE,          PlayerKey::k1 },
            { CR::Input::Key::TWO,          PlayerKey::k2 },
            { CR::Input::Key::THREE,        PlayerKey::k3 },
            { CR::Input::Key::FOUR,         PlayerKey::k4 },
            { CR::Input::Key::FIVE,         PlayerKey::k5 },
            { CR::Input::Key::SIX,          PlayerKey::k6 },
            { CR::Input::Key::SEVEN,        PlayerKey::k7 },
            { CR::Input::Key::EIGHT,        PlayerKey::k8 }
        };

        struct PlayerInputCompacter {
            CR::Vec2<int> mpos;
            uint8 states[PlayerKey::total];
            bool changed;
            PlayerInputCompacter(){
                for(int i = 0; i < PlayerKey::total; ++i){
                    states[i] = CR::PlayerPressType::Released;
                }                
            }            
            bool set(uint8 key, uint16 v){
                if(key >= CR::PlayerKey::total){
                    return false;
                }
                states[key] = v;
                return true;
            }
            void setAll(uint16 v){
                for(int i = 0; i < CR::PlayerKey::total; ++i){
                    states[i] = v;
                }
            }
            bool isKeyPress(uint8 key){
                if(key >= CR::PlayerKey::total){
                    return false;
                }
                return states[key] == CR::PlayerPressType::Pressed;                
            }
            uint16 getCompat(){
                uint16 compat = 0;
                for(int i = 0; i < CR::PlayerKey::total; ++i){
                    compat =  compat | ((states[i] == CR::PlayerPressType::Pressed) ? (1 << (i+1)) : 0);
                }
                return compat;
            }
            void loadCompat(uint16 v){
                for(int i = 0; i < CR::PlayerKey::total; ++i){
                    states[i] = v & (1 << (i+1)) ? CR::PlayerPressType::Pressed : CR::PlayerPressType::Released;
                }
            }
        };

        struct PlayerInput : PlayerInputCompacter {
            CR::Vec2<int> mpos;
            uint64 lastChange;
            uint64 lastCheck;
            std::unordered_map<int, uint8> mapping; // CR key -> game key
            PlayerInput(){
                this->mapping = CR::PlayerInputDefaultMapping;
            }
            void update(bool ignore){
                auto nmp = CR::Input::mousePosition();
                for(auto key : this->mapping){ 
                    bool check = !ignore && (key.first > 200 ? CR::Input::mouseCheck(key.first) : CR::Input::keyboardCheck(key.first));
                    auto nv = check ? CR::PlayerPressType::Pressed : CR::PlayerPressType::Released;
                    if(states[key.second] != nv){
                        states[key.second] = nv;
                        lastChange = CR::ticks();
                        changed = true;
                    }
                }
                if(nmp.x != mpos.x || nmp.y != mpos.y){
                    mpos.set(nmp.x, nmp.y);
                    lastChange = CR::ticks();
                }
            }
        };

        struct GridLoc {
            T_WORLDPOS index; // x+y*h
            T_WORLDPOS level; // height/altitude
            CR::Vec2<T_WORLDPOS> coords; //x,y
            CR::Vec3<float> currentPos; //real 3d position
            CR::Vec3<float> nextPos;
            GridLoc(T_WORLDPOS index, T_WORLDPOS level){
                this->index = index;
                this->level = level;
            }
            GridLoc(){
                index = 0;
                level = 0;
                coords.set(0);
                currentPos.set(0.0f);
                nextPos.set(0.0f);
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

            void setPosition(const CR::GridLoc &npos){

            }

            virtual void onCreate(){

            }

            virtual void onStep(){

            }

            virtual void onDestroy(){

            }

            virtual void draw(CR::Gfx::RenderLayer *layer){
                
            }            
        };        

        struct Audit { 
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