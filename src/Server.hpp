#ifndef CR_GAME_SERVER_HPP
    #define CR_GAME_SERVER_HPP

    #include "Network.hpp"
    #include "World.hpp"


    namespace CR {
        
        struct Server : NetHandle {

            CR::World world;

            std::thread gameThread;

            std::string sessionName;
            CR::T_GENERICID sessionId;
            std::string sessionPassword;
            
            uint32 lastClientId;

            Server();
            ~Server();

            bool listen(const std::string &name, uint8 maxClients, uint16 port);
            void close();
            void deliverPacketQueue();
            bool dropClient(CR::T_GENERICID clientId, const std::string &reason);

        };



    }

#endif
