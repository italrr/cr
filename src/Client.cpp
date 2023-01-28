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

static void CL_SEND_CURRENT_SIM_STATE(CR::Client *cl){
    CR::Packet packet;
    packet.setHeader(CR::PacketType::ACK_SIM_FRAME_STATE);
    packet.write(&cl->lastFrame, sizeof(CR::T_AUDITORD));
    packet.write(&cl->lastAudit, sizeof(CR::T_AUDITORD));
    cl->sendPacketFor(cl->svAddress, packet);
}

CR::Client::Client(){
    this->isServer = false;
    this->netState = CR::NetHandleState::IDLE;
    this->lastFrame = 0;
    this->lastAudit = 0;
    cleanUp();
}

bool CR::Client::connect(const std::string &ip, unsigned port){
    if(netState != CR::NetHandleState::IDLE){
        CR::log("[CLIENT] Failed to connect %s: Client is not in the IDLE state: disconnect from server\n", ip.c_str(), port);
        return false;
    }
    this->svAddress = CR::IP_Port(ip, port);
    this->world = std::make_shared<CR::World>(CR::World());
    this->connRet = 0;
    auto chosenPort = CR::NetworkDefaultPort + CR::Math::random(1, 200);
    if(!this->socket.open(chosenPort)){
        CR::log("[CLIENT] Failed to open UDP Socket at %i\n", chosenPort);
        return false;
    }
    socket.setNonBlocking(true);
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

void CR::Client::processPacket(CR::Packet &packet, bool ignoreOrder){
    auto &sender = packet.sender;
    uint64 netId = sender.address + sender.port + socket.sock;
    bool isSv = netId == this->svNetId && netState != CR::NetHandleState::DISCONNECTED;
    bool isLast = isSv && packet.getOrder() > this->rcvOrder;
    if(isSv && isLast){
        lastPacket = packet;
        rcvOrder = packet.getOrder();
    }
    if(ignoreOrder){
        isLast = true;
    }
    switch(packet.getHeader()){
        /*
            SV_MULTI_PART_PACKET    
        */
        case CR::PacketType::MULTI_PART_PACKET: {
            if(!isSv){
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
                holder.sender = svAddress;
                processPacket(holder, true);
            }
        } break;         
        case CR::PacketType::ACK: {
            ack(packet);
        } break;        
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
            CLIENT_JOIN
        */
        case CR::PacketType::CLIENT_JOIN: {
            if(!isSv || !isLast){ break; }
            CR::T_GENERICID clientId;
            std::string nickname;
            packet.read(&clientId, sizeof(CR::T_GENERICID));
            packet.read(nickname);

            auto client = getClient(clientId);
            if(!client){
                auto nclient = std::make_shared<CR::ClientHandle>(CR::ClientHandle());
                addClient(clientId, nickname);                
            }
            CR::log("[CLIENT] Player %s joined the game\n", nickname.c_str());
            sendAck(sender, packet.getAck());
        } break;   
        /*
            CLIENT_DISCONNECT
        */
        case CR::PacketType::CLIENT_DISCONNECT: {
            if(!isSv || !isLast){ break; }
            CR::T_GENERICID clientId;
            std::string reason;
            packet.read(&clientId, sizeof(CR::T_GENERICID));
            packet.read(reason);
            if(reason.length() == 0) reason = "Disconnected by the user";
            auto client = getClient(clientId);
            if(client){
                CR::log("[CLIENT] Player %s left the game: %s\n", client->nickname.c_str(), reason.c_str());
                removeClient(clientId);
            }
            sendAck(sender, packet.getAck());            
        } break;  
        /*
            CLIENT_DROP
        */
        case CR::PacketType::CLIENT_DROP: {
            if(!isSv || !isLast){ break; }
            CR::T_GENERICID clientId;
            std::string reason;
            packet.read(&clientId, sizeof(CR::T_GENERICID));
            packet.read(reason);
            if(reason.length() == 0) reason = "No reason given";
            if(clientId == this->clientId){
                CR::log("[CLIENT] Dropped by the server: %s\n", reason.c_str());
                this->cleanUp();
                return;
            }
            auto client = getClient(clientId);
            if(client){
                CR::log("[CLIENT] Player %s left the game: %s\n", client->nickname.c_str(), reason.c_str());
                removeClient(clientId);
            }
            sendAck(sender, packet.getAck());            
        } break;         
        /* SIMULATION_FRAME_STATE */                               
        case CR::PacketType::SIMULATION_FRAME_STATE: {
            if(!isSv || !isLast){ break; }

            CR::T_AUDITORD tick;
            CR::T_AUDITORD fromAudit;
            CR::T_TIME epoch;
            uint8 nAuditsTick;
            uint8 nAuditsPacket;

            packet.read(&tick, sizeof(tick));
            packet.read(&epoch, sizeof(epoch));
            packet.read(&fromAudit, sizeof(fromAudit));
            packet.read(&nAuditsTick, sizeof(nAuditsTick));
            packet.read(&nAuditsPacket, sizeof(nAuditsPacket));

            // check this state is indeed the next one
            if(tick < this->lastFrame || tick-this->lastFrame > 1 || tick == this->lastFrame && (fromAudit < this->lastAudit || fromAudit-this->lastAudit > 1)){
                CL_SEND_CURRENT_SIM_STATE(this);
                break;
            }
            
            std::vector<std::shared_ptr<Audit>> audits;
            T_AUDITORD readAudits = this->lastAudit + 0;
            for(unsigned i = 0; i < nAuditsPacket; ++i){
                auto audit = std::make_shared<CR::Audit>(CR::Audit());
                packet.read(&audit->type, sizeof(audit->type));
                packet.read(&audit->state, sizeof(audit->state));
                packet.read(audit->msg);
                // packet.index += 1;
                uint8 affEnt;
                packet.read(&affEnt, sizeof(affEnt));
                for(unsigned j = 0; j < affEnt; ++j){
                    CR::T_OBJID objId;
                    packet.read(&objId, sizeof(objId));
                    audit->affEnt.push_back(objId);
                }
                uint8 plSize;
                packet.read(&plSize, sizeof(plSize));
                audit->data.allocate();
                packet.read(audit->data.data, plSize);
                audit->data.size = plSize;
                audit->time = epoch;
                audit->tick = tick;
                audit->order = i + this->lastAudit;
                ++readAudits;
                audits.push_back(audit);
            }
            this->world->run(audits);
    
            this->lastFrame = tick;
            this->lastAudit = readAudits;   

            CL_SEND_CURRENT_SIM_STATE(this);

            // if we got all audits from this frame, reset counter
            if(this->lastAudit >= nAuditsTick){
                this->lastAudit = 0;
            }

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
                    CR::T_OBJID wId;
                    packet.read(seasonName);
                    packet.read(&sessionId, sizeof(CR::T_GENERICID));
                    packet.read(&clientId, sizeof(CR::T_GENERICID));
                    packet.read(&wId, sizeof(CR::T_OBJID));
                    CR::log("[CLIENT] Joined server (%s): Session Name/ID '%s/%i' | Client ID '%i' \n", this->svAddress.str().c_str(), seasonName.c_str(), sessionId, clientId);
                    this->netState = CR::NetHandleState::CONNECTED;
                    this->sendAck(sender, packet.getAck()); // make sure to send ACK
                    // this->socket.setNonBlocking(false); // switch to blocking from now on
                    this->svNetId = sender.address + sender.port + socket.sock;
                    world->setPuppet(true, wId);
                    world->start();
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
    int nb = socket.recv(sender, packet);
    if(nb > 0){
        rcvPackets.push_back(packet);
    }    
            
    // process packet queue
    if(rcvPackets.size() > 0){
        for(unsigned i = 0; i < rcvPackets.size(); ++i){
            processPacket(rcvPackets[i]);
        }
        rcvPackets.clear();
    }
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