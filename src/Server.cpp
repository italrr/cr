#include "Server.hpp"


static void SV_SEND_CLIENT_JOIN(CR::Server *sv, CR::T_GENERICID clientId, const std::string &nickname){

}

static void SV_PROCESS_PACKET(CR::Server *sv, CR::Packet &packet, bool ignoreOrder){
    auto sender = packet.sender;
    auto client = sv->getClientByIP(sender);
    bool isLast = client && packet.getOrder() > client->lastRecvOrder;
    if(client && isLast && !ignoreOrder){
        client->lastPacketTimeout = CR::ticks();
        client->lastPacket = packet;
        client->lastRecvOrder = packet.getOrder();
    }                  
    if(ignoreOrder){
        isLast = true;
    }
    switch(packet.getHeader()){
        /*
            SV_MULTI_PART_PACKET    
        */
        case CR::PacketType::MULTI_PART_PACKET: {
            if(!client){
                break;
            }                    
            std::vector<uint16> sizes;
            uint8 total;
            packet.read(&total, sizeof(total));
            for(int i = 0; i < total; ++i){
                uint16 size;
                packet.read(&size, sizeof(size));
                sizes.push_back(size);
            }
            for(int i = 0; i < total; ++i){
                CR::Packet holder;
                packet.read(holder.data, sizes[i]);
                holder.sender = client->ip;
                SV_PROCESS_PACKET(sv, holder, true);
            }
        } break;       
        case CR::PacketType::ACK: {
            sv->ack(packet);
        } break;              
        /*
            SV_PONG
        */      
        case CR::PacketType::PONG: {
            if(!client || !isLast){
                break;
            }
            client->ping = CR::ticks() - client->lastPing; 
        } break; 
        /*
            SV_PING
        */             
        case CR::PacketType::PING: {
            if(!client || !isLast){
                break;
            }
            CR::Packet pong;
            pong.setHeader(CR::PacketType::PONG);
            pong.setOrder(++client->lastSentOrder);
            sv->socket.send(client->ip, pong);    
        } break;
        /*
            SV_CONNECT_REQUEST
        */
        case CR::PacketType::CONNECT_REQUEST : {
            if(client){
                CR::Packet acptPacket;
                acptPacket.setHeader(CR::PacketType::CONNECT_ACCEPT);  
                acptPacket.write(sv->sessionName);
                acptPacket.write(&sv->sessionId, sizeof(CR::T_GENERICID));
                acptPacket.write(&client->clientId, sizeof(CR::T_GENERICID));
                sv->socket.send(sender, acptPacket); 
                break;
            }
            CR::log("[SERVER] Incoming connection from %s...\n", sender.str().c_str());
            CR::T_VERSION_SEG clVersion[3];
            std::string nickname;
            std::string password;
            std::string token;
            packet.read(clVersion, sizeof(CR::T_VERSION_SEG) * 3);
            packet.read(nickname);
            packet.read(password);
            packet.read(token);
            // Add client
            auto nclient = std::make_shared<CR::ClientHandle>(CR::ClientHandle());
            nclient->nickname = nickname;
            nclient->clientId = ++sv->lastClientId;
            nclient->ip = sender;
            sv->clients[nclient->clientId] = nclient;
            CR::log("[SERVER] Accepted client %s as Client ID %i | Nickname '%s'\n", sender.str().c_str(), nclient->clientId, nickname.c_str());
            // Send ACCEPT
            CR::Packet acptPacket;
            acptPacket.setOrder(0);
            acptPacket.setHeader(CR::PacketType::CONNECT_ACCEPT);  
            acptPacket.write(sv->sessionName);
            acptPacket.write(&sv->sessionId, sizeof(CR::T_GENERICID));
            acptPacket.write(&nclient->clientId, sizeof(CR::T_GENERICID));
            sv->socket.send(sender, acptPacket);
            // Notify client joined
            CR::Packet clJoinedPacket;
            clJoinedPacket.setHeader(CR::PacketType::CLIENT_JOIN);  
            clJoinedPacket.write(&nclient->clientId, sizeof(CR::T_GENERICID));
            clJoinedPacket.write(nickname);         
            sv->sendPersPacketForMany(sv->getAllClientsIPs(), clJoinedPacket, sv->getAllClientsACKs());
        } break;
        /*
            SV_CLIENT_DISCONNECT
        */             
        case CR::PacketType::CLIENT_DISCONNECT: {
            if(!client){
                break;
            }
        } break;

    }   
}

static void SV_THREAD(void *handle){
    auto sv = static_cast<CR::Server*>(handle);
    CR::Packet packet;
    CR::IP_Port sender;        
    while(sv->netState != CR::NetHandleState::LISTENING) CR::sleep(16);
    while(CR::NetHandleState::LISTENING){
        int nb = sv->socket.recv(sender, packet);
        if(nb > 0){
            std::unique_lock<std::mutex> lock(sv->netCon);
                SV_PROCESS_PACKET(sv, packet, false);
            lock.unlock();
        }
        sv->deliverPacketQueue();               
        for(auto &cl : sv->clients){
            auto &client = cl.second;
            if(CR::ticks()-client->lastPing > 1000){
                client->lastPing = CR::ticks();
                CR::Packet ping(CR::PacketType::PING);
                ping.setOrder(++client->lastSentOrder);
                sv->socket.send(client->ip, ping);
            }
        }          
    }
}

static void GAME_THREAD(void *handle){

}

CR::Server::Server(){
    this->isServer = true;
    this->netState = CR::NetHandleState::IDLE;
    this->lastClientId = CR::Math::random(101, 251);
    this->sessionName = "Social Room";
    this->sessionId = CR::Math::random(1001, 2499);
}

CR::Server::~Server(){
    close();
}

bool CR::Server::listen(const std::string &name, uint8 maxClients, uint16 port){
    this->sessionName = name;
    // this->maxClients = maxClients;
    if(!this->socket.open(CR::NetworkDefaultPort)){
        CR::log("[SERVER] Failed to start server: Couldn't open port %i\n", port);
        return false;
    }
    socket.setNonBlocking(true);
    CR::log("[SERVER] Started server. Listening at %i | Max Players %i\n", port, maxClients);
    this->thread = std::thread(&SV_THREAD, this); 
    this->netState = CR::NetHandleState::LISTENING;
    CR::log("[SERVER] Waiting for clients...\n");
    return true;
}

void CR::Server::deliverPacketQueue(){
    updateDeliveries();
    if(packetQueue.size() < 1){
        return;
    }

    // deliver messages
    for(auto &it : clients){
        // packets will be joined together into bigger packets 
        auto &client = it.second;
        std::vector<std::vector<int>> indexes;
        std::vector<std::vector<uint16>> sizes;
        static const size_t indexSize = sizeof(uint16);
        size_t size;
        auto reset = [&](){
            indexes.push_back(std::vector<int>());
            sizes.push_back(std::vector<uint16>());
            size = CR::NetworkMaxHeaderSize;
        };
        reset();
        // 1. calculate how many bigger packets are needed
        for(int i = 0; i < packetQueue.size(); ++i){
            auto &packet = packetQueue[i];
            // reset
            if(size + packet.maxSize > CR::NetworkMaxPacketSize){
                reset();                
            }
            size += packet.maxSize + sizeof(indexSize);
            indexes[indexes.size()-1].push_back(i);
            sizes[sizes.size()-1].push_back(packet.maxSize);
        }
        // 2. pack it up and send
        for(int i = 0; i < indexes.size(); ++i){
            CR::Packet handle(CR::PacketType::MULTI_PART_PACKET);
            auto &index = indexes[i];
            auto &size = sizes[i];
            // write sizes
            uint8 n = size.size();
            handle.write(&n, sizeof(n));
            for(int j = 0; j < size.size(); ++j){
                handle.write(&size[j], sizeof(size[j]));
            }
            // write packets
            for(int j = 0; j < index.size(); ++j){
                auto &packet = packetQueue[index[j]];
                handle.write(packet.data, packet.maxSize);
            }
            handle.setOrder(++client->lastSentOrder);
            this->socket.send(client->ip, handle);
        }
    }
    packetQueue.clear();  
}

void CR::Server::close(){
    // XD
}