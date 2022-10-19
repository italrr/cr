#include "Client.hpp"



static void CL_SEND_CONNECT(CR::Client* cl){
    CR::Packet packet;
    packet.setOrder(0);
    packet.setHeader(CR::PacketType::CONNECT_REQUEST);
    packet.write(&CR::GAME_VERSION, sizeof(CR::T_VERSION_SEG) * 3);
    packet.write("PLAYER");
    packet.write("#");
    packet.write("#");
    cl->socket.send(cl->svAddress, packet);
    cl->connReqAt = CR::ticks();
    ++cl->connRet;
};

static void CL_SEND_DISCONNECT(CR::Client *cl, const std::string &reason){
    CR::Packet packet;
    packet.setOrder(++cl->sentOrder);
    packet.setHeader(CR::PacketType::CLIENT_DISCONNECT);
    packet.write(reason);
    cl->socket.send(cl->svAddress, packet);
}

static void CL_THREAD(void *handle){
    auto cl = static_cast<CR::Client*>(handle);
    CR::Packet packet;
    CR::IP_Port sender;        
    while(cl->netState != CR::NetHandleState::CONNECTED) CR::sleep(16);
    while(CR::NetHandleState::CONNECTED){
        int nb = cl->socket.recv(sender, packet);
        if(nb > 0){
            std::unique_lock<std::mutex> lock(cl->netCon);
                cl->rcvPackets.push_back(packet);
            lock.unlock();
        }        
    }
}

CR::Client::Client(){
    this->isServer = false;
    this->netState = CR::NetHandleState::IDLE;
    cleanUp();
}

bool CR::Client::connect(const std::string &ip, unsigned port, CR::World *world){
    if(netState != CR::NetHandleState::IDLE){
        CR::log("[CLIENT] Failed to connect %s: Client is not in the IDLE state: disconnect from server\n", ip.c_str(), port);
        return false;
    }
    this->svAddress = CR::IP_Port(ip, port);
    this->world = world;
    this->connRet = 0;
    auto chosenPort = CR::NetworkDefaultPort + CR::Math::random(1, 200);
    if(!this->socket.open(chosenPort)){
        CR::log("[CLIENT] Failed to open UDP Socket at %i\n", chosenPort);
        return false;
    }
    socket.setNonBlocking(true);
    this->thread = std::thread(&CL_THREAD, this); 
    this->netState = CR::NetHandleState::CONNECTING;
    CR::log("[CLIENT] Connecting to %s...\n", this->svAddress.str().c_str());
    CL_SEND_CONNECT(this);
    return true;
}

void CR::Client::cleanUp(){
    if(this->socket.opened){
        this->socket.close();
    }
    deliveries.clear();
    packetQueue.clear();
    rcvPackets.clear();
    netState = CR::NetHandleState::IDLE;
    rcvOrder = 0;
    sentOrder = 1;
    lastPing = 0;    
    lastSentPing = 0;
}

void CR::Client::disconnect(const std::string &message){
    if(netState != CR::NetHandleState::IDLE){
        CL_SEND_DISCONNECT(this, message);
        netState = CR::NetHandleState::IDLE;
    }
    cleanUp();
}

void CR::Client::processPacket(CR::Packet &packet){
    auto &sender = packet.sender;
    uint64 netId = sender.address + sender.port + socket.sock;
    bool isSv = netId == this->svNetId && netState != CR::NetHandleState::DISCONNECTED;
    bool isLast = isSv && packet.getOrder() > this->rcvOrder;
    if(isSv && isLast){
        lastPacket = packet;
        rcvOrder = packet.getOrder();
    }
    switch(packet.getHeader()){
        /*
            SV_PONG
        */
        case CR::PacketType::PONG: {
            if(!isSv || !isLast){ break; }
            lastPacketTimeout = CR::ticks();
            this->ping = CR::ticks() - this->lastPing;  
        } break;
        /*
            SV_PING
        */
        case CR::PacketType::PING: {
            if(!isSv || !isLast){ break; }
            CR::Packet pong(CR::PacketType::PONG);
            pong.setOrder(++sentOrder);
            socket.send(this->svAddress, pong);
        } break;
        /*
            SV_CLIENT_DROP
        */
        case CR::PacketType::CLIENT_DROP: {
            // TODO: Handle client drops
            // if(!isSv || !isLast){ break; }
            // String reason;
            // handler.read(reason);
            // if(reason.length() == 0){
            //     reason = "no reason";
            // }
            // CR::print("[client] server dropped connection: "+reason);
            // clear();
        } break;               
                              
    }
}

void CR::Client::step(){
    CR::Packet packet;
    CR::IP_Port sender;           
    if(netState == CR::NetHandleState::CONNECTING){
        int nb = this->socket.recv(sender, packet);
        if(nb > 0){
            switch(packet.getHeader()){
                case CR::PacketType::CONNECT_ACCEPT: {
                    std::string seasonName;
                    CR::T_GENERICID sessionId;
                    CR::T_GENERICID clientId;
                    packet.read(seasonName);
                    packet.read(&sessionId, sizeof(CR::T_GENERICID));
                    packet.read(&clientId, sizeof(CR::T_GENERICID));
                    CR::log("[CLIENT] Joined server (%s): Session Name/ID '%s/%i' | Client ID '%i' \n", this->svAddress.str().c_str(), seasonName.c_str(), sessionId, clientId);
                    this->netState = CR::NetHandleState::CONNECTED;
                    this->sendAck(sender, packet.getAck()); // make sure to send ACK
                    // this->socket.setNonBlocking(false); // switch to blocking from now on
                    this->svNetId = sender.address + sender.port + socket.sock;
                    return;
                } break;
                case CR::PacketType::CONNECT_REJECT: {
                    std::string reason;
                    packet.read(reason);
                    if(reason.length() == 0) reason = "no reason given";
                    CR::log("[CLIENT] Connection rejected to server (%s): Reason: %s\n", this->svAddress.str().c_str(), reason.c_str());
                    cleanUp();
                    return;                                
                } break;                            
            }
        }else
        if(CR::ticks()-this->connReqAt > CR::Net::CLIENT_CONNECT_TIMEOUT && this->connRet < CR::Net::CLIENT_CONNECT_TRY_TIMEOUT_INTERVAL){
            CR::log("[CLIENT] Retrying connecting to %s (%i retries)...\n", this->svAddress.str().c_str(), this->connRet);   
            CL_SEND_CONNECT(this);
        }else
        if(CR::ticks()-this->connReqAt > CR::Net::CLIENT_CONNECT_TIMEOUT && this->connRet == CR::Net::CLIENT_CONNECT_TRY_TIMEOUT_INTERVAL){
            CR::log("[CLIENT] Failed to connect to server (%s): Server didn't respond (%i retries)\n", this->svAddress.str().c_str(), this->connRet);
            cleanUp();
            return;
        } 
    }else
    if(netState != CR::NetHandleState::CONNECTED){
        return;
    }
    // process packet queue
    std::unique_lock<std::mutex> lock(this->netCon);
        if(rcvPackets.size() > 0){
            for(unsigned i = 0; i < rcvPackets.size(); ++i){
                processPacket(rcvPackets[i]);
            }
            rcvPackets.clear();
        }
    lock.unlock();  
    deliverPacketQueue();
    if(CR::ticks()-lastSentPing > 1000){
        lastSentPing = CR::ticks();
        lastPing = CR::ticks();
        CR::Packet ping(CR::PacketType::PING);
        ping.setOrder(++sentOrder);
        this->socket.send(this->svAddress, ping);
    }    
}


void CR::Client::deliverPacketQueue(){
    updateDeliveries();  
    // deliver messages
    if(packetQueue.size() < 1){
        return;
    }        
    // packets will be joined together into bigger packets 
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
        handle.setOrder(++sentOrder);
        this->socket.send(this->svAddress, handle);
    }
    packetQueue.clear();
}