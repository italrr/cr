#ifndef CR_GAME_CLIENT_HPP
    #define CR_GAME_CLIENT_HPP

    #include "Network.hpp"
    #include "World.hpp"


    namespace CR {


        
        struct Client : NetHandle {
            CR::T_GENERICID clientId;
            CR::IP_Port svAddress;
            uint64 connReqAt;
            unsigned connRet;
            uint64 svNetId;
            uint64 lastPacketTimeout;
            CR::Packet lastPacket;
            uint64 ping;
            uint64 lastPing;
            uint64 lastSentPing;
            // Packet delivery
            uint32 rcvOrder;
            uint32 sentOrder;

            CR::T_AUDITORD lastFrame;
            CR::T_AUDITORD lastAudit;
            
            Client();
            void processPacket(CR::Packet &packet, bool ignoreOrder = false);
            bool connect(const std::string &ip, unsigned port);
            void disconnect(const std::string &message = "");
            void cleanUp();
            void step();
            void deliverPacketQueue();

        };



    }

#endif
