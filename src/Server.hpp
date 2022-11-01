#ifndef CR_GAME_SERVER_HPP
    #define CR_GAME_SERVER_HPP

    #include "Network.hpp"
    #include "World.hpp"


    namespace CR {

        struct Server : NetHandle {
            std::thread gameThread;

            std::string sessionName;
            CR::T_GENERICID sessionId;
            std::string sessionPassword;

            std::vector<std::shared_ptr<Audit>> auditQueue;
            std::mutex alMutex;
            
            uint32 lastClientId;

            Server();
            ~Server();

            void flushFrameQueue(std::vector<std::shared_ptr<Audit>> &list);
            bool listen(const std::string &name, uint8 maxClients, uint16 port);
            void close();
            void deliverPacketQueue();
            bool dropClient(CR::T_GENERICID clientId, const std::string &reason);

        };



    }

#endif
