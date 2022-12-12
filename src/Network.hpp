#ifndef CR_GAME_NETWORK_HPP
    #define CR_GAME_NETWORK_HPP

    #include <thread>
    #include <unordered_map>

    #include "Engine/Network.hpp"
    #include "Game.hpp"


    namespace CR {

        /*
            Some definitions:

            A game session can be private or public to join.

            A game session can be Cloud only (Meaning the session is essentially run on the servers even when playing single player story mode).
            Characters and save games when Cloud only are stored in the servers. You may download a copy of the character, but it'll be recognized
            as such (a copy).
            
            So essentially there are 2 types of character info:
            - Cloud Only: People whose character was created online, and therefore play from a server even when playing single player
            - Local: People whose character was created locally/offline

            When creating a game session, the leader can decide whether the server would allow any of the 2 to join.
            Some people would prefer Online only characters to avoid encountering cheaters,
            Some other people wouldn't mind Local characters to join.

            Cloud only should to create some form of trust that other players aren't cheating. Of course this is not guaranteed, but
            it should reduce it a little bit. Local saves are encrypted.


            When a new character is created, it's registered in the backend.

        */

        namespace Net {
            static const uint32 CLIENT_UNRESPONSIVE_TIMEOUT = 30 * 1000; // 30 seconds before dropping unresponsive client
            static const uint32 CLIENT_CONNECT_TRY_TIMEOUT_INTERVAL = 4;  // retries 3 times + 1 initial connection
            static const uint64 CLIENT_CONNECT_TIMEOUT = 1000 * 5;
        }

        namespace ClientOriginType {
            enum ClientOriginType : T_GENERICTYPE {
                CLOUD_ONLY,
                LOCAL,
                ANY
            };            
        }

        namespace SessionType {
            enum SessionType : T_GENERICTYPE {
                FREEROAM = 0,
                BATTLE_ROLAYE,
                TEAM_DEATHMATCH,
                ONE_VS_ONE,
                SEAMLESS_COOP
            };
        }        

        struct PersisentDelivey {
            CR::Packet packet;
            uint32 ack;
            uint16 header;
            CR::IP_Port ip;
            uint64 lastRetry;
            std::function<void(CR::SmallPacket &payload, CR::IP_Port &cl)> onAck;
            CR::SmallPacket onAckPayload;
            PersisentDelivey(){
                this->onAck = [](CR::SmallPacket &payload, CR::IP_Port &cl){

                };
            }
        };

        namespace NetHandleState {
            enum NetHandleState : T_GENERICTYPE {
                IDLE,
                LISTENING,
                CONNECTING,
                CONNECTED,
                DISCONNECTED
            };
        }

        struct ClientHandle {
            CR::T_GENERICID clientId;
            CR::T_AUDITORD lastFrame;
            CR::T_AUDITORD lastAudit;
            CR::T_TIME lastFrameACK;
            bool readyNextFrame;
            std::mutex nextFrameMutex;
            
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

            void setReadyForFrame(bool ready);
            void setReadyForFrame(bool ready, CR::T_AUDITORD lastAudit, CR::T_AUDITORD lastFrame);
            ClientHandle(const ClientHandle &other);
            ClientHandle();
        };

        struct NetHandle {
            std::shared_ptr<CR::World> world;

            T_GENERICTYPE netState;
            std::thread thread;
            std::mutex netCon;
            bool isServer;
            CR::UDPSocket socket;
            std::vector<std::shared_ptr<CR::PersisentDelivey>> deliveries;
            std::vector<CR::Packet> packetQueue;
            std::vector<CR::Packet> rcvPackets;
            
            std::unordered_map<CR::T_GENERICID, std::shared_ptr<CR::ClientHandle>> clients;

            NetHandle();

            bool removeClient(CR::T_GENERICID clientId);
            std::shared_ptr<CR::ClientHandle> addClient(CR::T_GENERICID clientId, const std::string &nickname, const CR::IP_Port &ip = CR::IP_Port());

            CR::ClientHandle *getClient(CR::T_GENERICID playerId);
            CR::ClientHandle *getClientByIP(const CR::IP_Port &ip);            
            
            std::vector<CR::IP_Port> getAllClientsIPs();
            std::vector<uint32> getAllClientsACKs();

            void sendPacketFor(const CR::IP_Port &ip, CR::Packet &packet);
            void sendPacketForMany(const std::vector<CR::IP_Port> &ips, CR::Packet &packet);
            void sendPersPacketFor(const CR::IP_Port &ip, CR::Packet &packet, uint32 ack);
            void sendPersPacketForMany(const std::vector<CR::IP_Port> &ips, CR::Packet &packet, const std::vector<uint32> acks);

            void updateDeliveries();
            void dropPersFor(const CR::IP_Port &ip);
            void dropPersForHeader(const CR::IP_Port &ip, uint16 header);
            void ack(CR::Packet &packet);
            void sendAck(CR::IP_Port &ip, uint32 ackId);
            void bindOnAckFor(uint32 ack, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda, CR::SmallPacket packet);
            void bindOnAckFor(uint32 ack, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda);

            void bindOnAckForHeader(uint16 header, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda, CR::SmallPacket packet);
            void bindOnAckForHeader(uint16 header, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda);                        
        };

        enum PacketType : uint16 {

            MULTI_PART_PACKET = 0,
            /*
                UINT8 N
                { 
                    UINT16 * N
                }
                {
                    PACKETS
                }
            */

            ACK,
            /*
                UINT32 ORDER
            */

            PING,
            /*

            */

            PONG,
            /*

            */           

            REQ_SESSION_INFO,
            /*
                STRING AUTH TOKEN (IF REQUIRED. 64 BYTES MAX. ASCII)
            */

            SESSION_INFO,
            /*
                STRING SESSION MAME (MAX 24 B)
                UINT8 GAME TYPE/SESSION TYPE
                UINT8 CLIENT ORIGIN
                UINT64 RUNTIME
                BOOL PASSWORDREQ
                BOOL STARTED/AT LOBBY
                UINT8 CURRENT PLAYER NUMBER
                UINT8 MAX PLAYER NUMBER

            */

            CONNECT_REQUEST,
            /*
                UINT8[3] CLIENT VERSION
                STRING NICKNAME (MAX 24 B)
                STRING AUTH TOKEN 
                STRING PASSWORD 
            */

            CONNECT_ACCEPT,
            /*
                STRING SESSION MAME (MAX 24 B)
                UINT32 SESSION ID
                UINT32 CLIENT ID
                T_OBJID WORLD ID
            */

            CONNECT_REJECT,
            /*
                STRING REASON (MAX 300 B)
            */

            CLIENT_JOIN,
            /*
                UINT32 CLIENT ID
                STRING NICKNAME
            */

            CLIENT_DROP,
            /*
                UINT32 CLIENT ID
                STRING REASON (MAX 300 B)
            */

            CLIENT_DISCONNECT,
            /*
                UINT32 CLIENT ID
                STRING REASON (MAX 300 B)
            */

            CLIENT_AWAIT_CLIENT_LOAD,
            /*
                STRING MAP ID (HASH)
            */

            CLIENT_LOAD_READY,
            /*
                STRING MAP ID (HASH)
            */

            CLIENT_LIST,
            /*
                UINT8 N
                0: {
                    UINT32 CLIENT ID
                    STRING NICKNAME (MAX 24 B)
                    UINT8 PING
                }
                ...
            */

            BROADCAST_MESSAGE, // From the server
            /*
                STRING MESSAGE (MAX 250 B)
                UINT8[3] COLOR
            */

            CHAT_MESSAGE,
            /*
                UINT32 CLIENT ID
                STRING MESSAGE (MAX 250 B)
            */

            CHARACTER_SHEET,
            /*
                

            */

            SIMULATION_SETUP,
            /*
            
            */

            SIMULATION_FRAME_STATE,
            /*
                T_FRAMEORD  TICK
                T_TIME      EPOCH
                T_FRAMEORD  FROM AUDIT N
                UINT8       NUMBER OF TOTAL FRAMES IN THIS TICK
                UINT8       NUMBER OF FRAMES IN THIS PACKET
                {
                    T_FRAME     TYPE
                    T_STATE     CURRENT STATE     
                    STRING      MESSAGE
                    UINT8       AFFECTED ENTITIES
                    0: {
                        T_OBJID ENTITY ID
                    }
                    ... N
                    UINT8       PAYLOAD SIZE
                    [PAYLOAD DATA]
                } ... N
            */
            ACK_SIM_FRAME_STATE,
            /*
                T_AUDITORD LAST APPLIED FRAME
                T_AUDITORD LAST AUDIT APPLIED
            */





        };      

    }


#endif