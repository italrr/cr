#ifndef CR_GAME_WORLD_HPP
    #define CR_GAME_WORLD_HPP

    #include "Game.hpp"
    #include "Map.hpp"

    namespace CR {

        namespace WorldState {
            enum WorldState : T_STATE {
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
            T_STATE state;
            T_STATE prevState;
            T_TIME lastState;


            std::vector<T_OBJID> toRemObjs;
            std::vector<std::shared_ptr<Object>> objects;
            std::vector<std::shared_ptr<Frame>> auditBacklog;
            std::vector<std::shared_ptr<Frame>> auditHistory;        


            void setState(T_STATE nstate);
            bool apply(const std::shared_ptr<Frame> &audit);
            World();
            void start();
            void reqEnd();
            void run(unsigned ticks);
            bool exists(CR::T_OBJID id);
            std::shared_ptr<CR::Object> get(CR::T_OBJID id);
            std::shared_ptr<Frame> createFrame(T_FRAME type);
            std::shared_ptr<Frame> createFrame(const std::string &msg);
            CR::T_OBJID add(const std::shared_ptr<CR::Object> &obj);
            bool destroy(T_OBJID id);
            void render(const CR::Vec2<unsigned> &offset);


        };


    }

#endif