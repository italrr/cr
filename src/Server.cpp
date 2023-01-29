#include "Server.hpp"

#ifdef CR_ENABLE_DEBUG_BUILD
    #include <stdio.h>
    static const char *WORLD_SIM_AUDIT_LIST = "./AUDITS.txt";
    static FILE *WS_AUDIT_LIST_FILE = NULL;
#endif

static void SV_SEND_CLIENT_JOIN(CR::Server *sv, CR::ClientHandle *client){
    CR::Packet clJoinedPacket;
    clJoinedPacket.setHeader(CR::PacketType::CLIENT_JOIN);  
    clJoinedPacket.write(&client->clientId, sizeof(CR::T_GENERICID));
    clJoinedPacket.write(client->nickname);         
    sv->sendPersPacketForMany(sv->getAllClientsIPs(), clJoinedPacket, sv->getAllClientsACKs());
    // Update world
    auto audit = sv->world->createAudit(CR::AuditType::PLAYER_JOINED);
    audit->msg = client->nickname;
    audit->data.write(&client->clientId, sizeof(CR::T_GENERICID));
    sv->world->auditBacklog.push_back(audit);
}

static void SV_SEND_CLIENT_DISCONNECT(CR::Server *sv, CR::ClientHandle *client, const std::string &reason){
    CR::Packet clJoinedPacket;
    clJoinedPacket.setHeader(CR::PacketType::CLIENT_DISCONNECT);  
    clJoinedPacket.write(&client->clientId, sizeof(CR::T_GENERICID));
    clJoinedPacket.write(reason);         
    sv->sendPersPacketForMany(sv->getAllClientsIPs(), clJoinedPacket, sv->getAllClientsACKs());
    // Update world
    auto audit = sv->world->createAudit(CR::AuditType::PLAYER_LEFT);
    audit->msg = reason;
    audit->data.write(&client->clientId, sizeof(CR::T_GENERICID));
    sv->world->auditBacklog.push_back(audit);    
}

static void SV_SEND_CLIENT_DROP(CR::Server *sv, CR::ClientHandle *client, const std::string &reason){
    CR::Packet clJoinedPacket;
    clJoinedPacket.setHeader(CR::PacketType::CLIENT_DROP);  
    clJoinedPacket.write(&client->clientId, sizeof(CR::T_GENERICID));
    clJoinedPacket.write(reason);         
    sv->sendPersPacketForMany(sv->getAllClientsIPs(), clJoinedPacket, sv->getAllClientsACKs());
    // Update world
    auto audit = sv->world->createAudit(CR::AuditType::PLAYER_LEFT);
    audit->msg = reason;
    audit->data.write(&client->clientId, sizeof(CR::T_GENERICID));
    sv->world->auditBacklog.push_back(audit);      
}


static void SV_CHECK_SKIP_TICK(CR::Server *sv, CR::ClientHandle *client, const std::unordered_map<CR::T_AUDITORD, std::vector<std::shared_ptr<CR::Audit>>> &history){
    auto it = history.find(client->lastFrame);
    if(it == history.end()){
        client->lastAudit = 0;        
        CR::log("[SERVER] Failed to find tick/frame needed for client %i: %i. Is the client from the future??? [SV_CHECK_SKIP_TICK]\n", client->clientId, client->lastFrame);
        // TODO: This shouldn't happen, but if it does, probably the best course of action is to kick them
        return;
    }

    if(client->lastAudit >= it->second.size()-1){
        client->lastAudit = 0;
        ++client->lastFrame;
    }
}


static void SV_SEND_AUDITS(CR::Server *sv, CR::T_AUDITORD fromAudit, CR::T_AUDITORD tick, CR::ClientHandle *client, const std::unordered_map<CR::T_AUDITORD, std::vector<std::shared_ptr<CR::Audit>>> &history){
    auto it = history.find(tick);
    if(it == history.end()){
        // CR::log("[SERVER] Failed to find tick needed for client %i: %i. Is the client from the future??? [SV_SEND_AUDITS]\n", client->clientId, tick);
        // We probably shouldn't check this here, but the same as in SV_CHECK_SKIP_TICK, we should probably just kick the player 
        return;
    }

    auto copy = it->second;

    #if CR_ENABLE_DEBUG_BUILD == 1
        if(WS_AUDIT_LIST_FILE == NULL){
            WS_AUDIT_LIST_FILE = fopen(WORLD_SIM_AUDIT_LIST, "w");
            fprintf(WS_AUDIT_LIST_FILE, "--- NEW RUN ---\n");
        }
        // fprintf(fp, "This is the line #%d\n", i + 1);
        fprintf(WS_AUDIT_LIST_FILE, "[FRAME %i START]\n", tick);
        for(unsigned i = 0; i < copy.size(); ++i){
            if(copy[i]->msg.length() > 0){
                fprintf(WS_AUDIT_LIST_FILE, "%i: '%s'\n", copy[i]->type, copy[i]->msg.c_str());
            }else{
                fprintf(WS_AUDIT_LIST_FILE, "%i\n", copy[i]->type);
            }
        }
        fprintf(WS_AUDIT_LIST_FILE, "[FRAME %i END]\n", tick);
    #endif

    if(fromAudit > 0){ // we delete the ones before
        copy.erase(copy.begin(), copy.begin() + fromAudit);
    }

    if(copy.size() == 0){ return; }    

    std::vector<CR::Packet> deliveries;
    bool deliveredAll = false;
    uint8 nAudits = copy.size();
    while(!deliveredAll){
        CR::Packet frameUpd;
        frameUpd.setHeader(CR::PacketType::SIMULATION_FRAME_STATE);  
        frameUpd.write(&copy[0]->tick, sizeof(copy[0]->tick));
        frameUpd.write(&copy[0]->time, sizeof(copy[0]->time));
        frameUpd.write(&fromAudit, sizeof(fromAudit));
        frameUpd.write(&nAudits, sizeof(nAudits));
        uint16 nAudInTP = frameUpd.index;
        uint8 audCount = 0;
        frameUpd.write(&nAudits, sizeof(nAudits));

        while(copy.size() > 0){     
            auto &dt = copy[0];
            dt->data.reset();
            CR::SmallPacket buffer;
            buffer.write(&dt->type, sizeof(dt->type));
            buffer.write(&dt->state, sizeof(dt->state));
            buffer.write(dt->msg);
            uint8 affEnt = dt->affEnt.size();
            buffer.write(&affEnt, sizeof(affEnt));
            for(unsigned j = 0; j < dt->affEnt.size(); ++j){
                buffer.write(&dt->affEnt[j], sizeof(dt->affEnt[j])); 
            }
            uint8 plSize = dt->data.size;
            buffer.write(&plSize, sizeof(plSize)); 
            buffer.write(dt->data.data, plSize); 
            if(buffer.size + frameUpd.maxSize > CR::NetworkMaxPacketSize){
                break; 
            }else{
                frameUpd.write(buffer.data, buffer.size);
                copy.erase(copy.begin());
                ++audCount;
            }
        }

        frameUpd.setIndex(nAudInTP);
        frameUpd.write(&audCount, sizeof(uint8));


        deliveries.push_back(frameUpd);
        deliveredAll = copy.size() == 0;
    } 
    
    for(unsigned i = 0; i < deliveries.size(); ++i){
        // sv->sendPersPacketForMany(sv->getAllClientsIPs(), deltaDeliveries[i], sv->getAllClientsACKs());
        sv->sendPacketFor(client->ip, deliveries[i]);
    }

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
            // TODO: some validation here
            // Add client
            auto nclient = sv->addClient(++sv->lastClientId, nickname, sender);
            CR::log("[SERVER] Accepted client %s as Client ID %i | Nickname '%s'\n", sender.str().c_str(), nclient->clientId, nickname.c_str());
            // Send ACCEPT
            CR::Packet acptPacket;
            acptPacket.setOrder(0);
            acptPacket.setHeader(CR::PacketType::CONNECT_ACCEPT);  
            acptPacket.write(sv->sessionName);
            acptPacket.write(&sv->sessionId, sizeof(CR::T_GENERICID));
            acptPacket.write(&nclient->clientId, sizeof(CR::T_GENERICID));
            acptPacket.write(&sv->world->wId, sizeof(CR::T_OBJID));
            sv->socket.send(sender, acptPacket);
            // Notify client joined
            SV_SEND_CLIENT_JOIN(sv, nclient.get());
        } break;
        /*
            SV_CLIENT_DISCONNECT
        */             
        case CR::PacketType::CLIENT_DISCONNECT: {
            if(!client || !isLast){
                if(!client) CR::log("[SERVER] Fatal error: Disconnected Client '%s' does not exist in the server\n", sender.str().c_str());
                break;
            }
            CR::T_GENERICID clientId;
            std::string reason;
            packet.read(&clientId, sizeof(CR::T_GENERICID));
            packet.read(reason);
            if(reason.length() == 0) reason = "Disconnected by the user";
            CR::log("[SERVER] Player %s left the game: %s\n", client->nickname.c_str(), reason.c_str());
            sv->removeClient(clientId);
            // Notify others
            SV_SEND_CLIENT_DISCONNECT(sv, client, reason); 
            // TODO: Send frame to world
        } break;
        /*
            ACK_SIM_FRAME_STATE
        */    
        case CR::PacketType::ACK_SIM_FRAME_STATE: {
            if(!client || !isLast){
                break;
            }
            CR::T_AUDITORD lastFrame, lastAudit;
            packet.read(&lastFrame, sizeof(CR::T_AUDITORD));
            packet.read(&lastAudit, sizeof(CR::T_AUDITORD));
            client->setReadyForFrame(true, lastAudit, lastFrame);
            SV_CHECK_SKIP_TICK(sv, client, sv->world->auditHistory);
            #if CR_ENABLE_DEBUG_BUILD == 1
                fprintf(WS_AUDIT_LIST_FILE, "[ACK] AUDIT %i |  FRAME %i\n", lastAudit, lastFrame);
            #endif            
           
        } break;
        /* 
            CLIENT_INPUT_STATUS
        */
       case CR::PacketType::CLIENT_INPUT_STATUS: {
            if(!client || !isLast){
                break;
            }
            CR::T_AUDITORD tick;
            uint16 keys;
            uint16 mouseX;
            uint16 mouseY;
            packet.read(&tick, sizeof(tick));
            packet.read(&keys, sizeof(keys));
            packet.read(&mouseX, sizeof(mouseX));
            packet.read(&mouseY, sizeof(mouseY));
            client->input.setAll(keys);
            client->input.mpos.set(mouseX, mouseY);
       } break;

        default: {
            // TODO: this can be abused
            CR::log("[SERVER] Unhandled Packet ID: %i from %s\n", packet.getHeader(), sender.str().c_str());
        } break;

    }   
}

static bool ssTrig = false;

void CR::Server::step(){
    if(!ssTrig && CR::ticks()-startTime > 2000){
        auto entId = this->world->createEntity("PLAYER", CR::EntityType::PLAYER, CR::GridLoc(0, 0));    
        auto audit = this->world->createAudit(CR::AuditType::PLAYER_GRANT_ENTITY_CONTROL);
        audit->affEnt.push_back(entId);
        audit->data.write(&this->clients.begin()->second->clientId, sizeof(CR::T_GENERICID));
        this->world->auditBacklog.push_back(audit);
        ssTrig = true;
    }
    // update world
    if(world->state != CR::WorldState::IDLE && world->state != CR::WorldState::STOPPED){
        if(CR::ticks()-lastWorldTick >= world->tickRate){
            world->run(1);
        }
    }
    // run server controller
    CR::Packet packet;
    CR::IP_Port sender;        
    if(netState == CR::NetHandleState::LISTENING){
        int nb = socket.recv(sender, packet);
        if(nb > 0){
            SV_PROCESS_PACKET(this, packet, false);            
        }
        // Send delta to clients
        for(auto &it : clients){
            auto cl = it.second.get();
            if(CR::ticks()-cl->lastFrameACK > 1000 || cl->readyNextFrame){
                SV_SEND_AUDITS(this, cl->lastAudit, cl->lastFrame, cl, this->world->auditHistory);
                cl->setReadyForFrame(false);
            }
        }
        // Update deliveries
        deliverPacketQueue();               
        // Ping clients
        for(auto &cl : this->clients){
            auto &client = cl.second;
            if(CR::ticks()-client->lastPing > 1000){
                client->lastPing = CR::ticks();
                CR::Packet ping(CR::PacketType::PING);
                ping.setOrder(++client->lastSentOrder);
                this->socket.send(client->ip, ping);
            }
        }          
        // Drop unresponsive clients
        std::vector<CR::ClientHandle*> timedoutClients;
        for(auto cl : this->clients){
            if(CR::ticks()-cl.second->lastPacketTimeout > CR::Net::CLIENT_UNRESPONSIVE_TIMEOUT){
                timedoutClients.push_back(cl.second.get());
            }
        }
        for(int i = 0; i < timedoutClients.size(); ++i){
            auto &client = timedoutClients[i];
            this->dropClient(client->clientId, "Timed out");
        }       
    }    

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

    this->world = std::make_shared<CR::World>(CR::World());
    world->start(); // TODO: add clear to allow reusing world
    socket.setNonBlocking(true);
    CR::log("[SERVER] Started server. Listening at %i | Max Players %i\n", port, maxClients);
    this->netState = CR::NetHandleState::LISTENING;
    CR::log("[SERVER] Waiting for clients...\n");

    this->startTime = CR::ticks();
    return true;
}

bool CR::Server::dropClient(CR::T_GENERICID clientId, const std::string &reason){
    auto client = getClient(clientId);
    if(!client) return false;
    CR::log("[server] Dropped client '%s': %s\n", client->nickname.c_str(), reason.c_str());
    removeClient(client->clientId);
    SV_SEND_CLIENT_DROP(this, client, reason);  
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