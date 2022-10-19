#ifndef CR_GAME_SERVER_HPP
    #define CR_GAME_SERVER_HPP

    #include "Network.hpp"
    #include "World.hpp"


    namespace CR {


        struct ClientHandle {
            CR::T_GENERICID clientId;
            uint64 netId;
            CR::IP_Port ip;
            std::string nickname;
            uint64 ping;
            uint64 lastPing;
            uint32 svACK;
            CR::Packet lastPacket;

            uint32 lastRecvOrder; 
            uint32 lastSentOrder;  
            uint64 lastPacketTimeout;            


            ClientHandle(){
                lastRecvOrder = 0;
                lastSentOrder = 1;
                svACK = 0;       
            }
        };
        
        struct Server : NetHandle {

            CR::World world;

            std::thread gameThread;


            std::string sessionName;
            CR::T_GENERICID sessionId;
            std::unordered_map<CR::T_GENERICID, std::shared_ptr<CR::ClientHandle>> clients;
            uint32 lastClientId;

            Server();
            ~Server();

            CR::ClientHandle *getClient(CR::T_GENERICID playerId);
            CR::ClientHandle *getClientByIP(const CR::IP_Port &ip);
            bool listen(const std::string &name, uint8 maxClients, uint16 port);
            void close();
            void deliverPacketQueue();

        };



    }

#endif
