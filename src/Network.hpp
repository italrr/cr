#ifndef CR_GAME_NETWORK_HPP
    #define CR_GAME_NETWORK_HPP

    #include "Engine/Network.hpp"


    namespace CR {

        namespace Net {
            static const uint32 CLIENT_UNRESPONSIVE_TIMEOUT = 30 * 1000; // 30 seconds before dropping unresponsive client
            static const uint32 CLIENT_CONNECT_TRY_TIMEOUT_INTERVAL = 3;  // retries 3 times
            static const uint64 CLIENT_CONNECT_TIMEOUT = 1000 * 3;
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

        struct NetHandle {
            bool isServer;
            CR::UDPSocket socket;
            std::vector<std::shared_ptr<CR::PersisentDelivey>> deliveries;
            std::vector<CR::Packet> packetQueue;
            std::vector<CR::Packet> rcvPackets;
            
            NetHandle();
            
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

            SV_MULTI_PART_PACKET = 0,
            /*
                UINT8 N
                { 
                    UINT16 * N
                }
                {
                    PACKETS
                }
            */

            SV_ACK,
            /*
                UINT32 ORDER
            */

            SV_PING,
            /*

            */

            SV_PONG,
            /*

            */           

        };      

    }


#endif