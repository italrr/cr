#ifndef CR_GAME_WORLD_HPP
    #define CR_GAME_WORLD_HPP

    #include "Game.hpp"
    #include "Map.hpp"

    namespace CR {

        namespace WorldState {
            enum WorldState : T_STATUS {
                IDLE = 0,
                PAUSED,
                RUNNING,
                STOPPING,
                STOPPED,
            };
        }


        struct World {
            T_OBJID wId;
            T_TIME timePerTick;
            T_TIME totalSimTime;
            T_FRAMEORD currentTick;
            T_FRAMEORD tickRate;
            T_STATUS state;


            std::vector<T_OBJID> toRemObjs;
            std::vector<std::shared_ptr<Object>> objects;
            std::vector<std::shared_ptr<Frame>> auditBacklog;
            std::vector<std::shared_ptr<Frame>> auditHistory;        

            World();
            void start();
            void reqEnd();
            void run(unsigned ticks);
            bool exists(CR::T_OBJID id);
            std::shared_ptr<CR::Object> get(CR::T_OBJID id);
            CR::T_OBJID add(const std::shared_ptr<CR::Object> &obj);
            bool destroy(T_OBJID id);
            void render(const CR::Vec2<unsigned> &offset);


        };


    }

#endif