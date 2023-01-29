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
            bool puppetMode;

            T_OBJID wId;
            T_TIME totalSimTime;
            T_AUDITORD currentTick;
            T_AUDITORD tickRate;
            T_STATE state;
            T_STATE prevState;
            T_TIME lastState;


            std::vector<T_OBJID> toRemObjs;
            std::vector<std::shared_ptr<Object>> objects;
            std::vector<std::shared_ptr<Audit>> auditBacklog;
            std::unordered_map<T_AUDITORD, std::vector<std::shared_ptr<Audit>>> auditHistory;        


            void setState(T_STATE nstate);
            bool apply(const std::shared_ptr<Audit> &audit);
            World();
            void setPuppet(bool puppetMode, T_OBJID wId);
            void start();
            void reqEnd();
            bool isLocEmpty(T_WORLDPOS x, T_WORLDPOS y);
            bool isLocEmpty(T_WORLDPOS index);
            bool run(unsigned ticks); // authoritative (for server)
            bool run(const std::vector<std::shared_ptr<Audit>> &audits);  // puppet mode (for clients)
            bool exists(CR::T_OBJID id);
            std::shared_ptr<CR::Object> get(CR::T_OBJID id);
            std::shared_ptr<Audit> createAudit(T_AUDIT type);
            std::shared_ptr<Audit> createAudit(const std::string &msg);
            CR::T_OBJID createEntity(const std::string &name, T_ENTTYPE type, const CR::GridLoc &loc);
            bool destroy(T_OBJID id);
            void render(const CR::Vec2<unsigned> &offset);


        };


    }

#endif