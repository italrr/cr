#ifndef CR_GAME_SERVER_HPP
    #define CR_GAME_SERVER_HPP

    #include "Network.hpp"
    #include "World.hpp"


    namespace CR {

        struct Server : NetHandle {
            std::string sessionName;
            CR::T_GENERICID sessionId;
            std::string sessionPassword;

            std::vector<std::shared_ptr<Audit>> auditQueue;
            std::mutex alMutex;
            uint64 startTime;
            uint32 lastClientId;

            CR::T_TIME lastWorldTick;

            Server();
            ~Server();

            bool listen(const std::string &name, uint8 maxClients, uint16 port);
            void close();
            void step();
            void deliverPacketQueue();
            bool dropClient(CR::T_GENERICID clientId, const std::string &reason);

        };



    }

#endif
