#include "Network.hpp"



void CR::ClientHandle::setReadyForFrame(bool ready){
    std::unique_lock<std::mutex> lock(nextFrameMutex);
        this->lastFrameACK = CR::ticks();
        this->readyNextFrame = ready;
    lock.unlock();
}

void CR::ClientHandle::setReadyForFrame(bool ready, CR::T_AUDITORD lastAudit, CR::T_AUDITORD lastFrame){
    std::unique_lock<std::mutex> lock(nextFrameMutex);
        this->lastFrameACK = CR::ticks();
        this->readyNextFrame = ready;
        this->lastAudit = lastAudit;
        this->lastFrame = lastFrame;
    lock.unlock();
}        

CR::ClientHandle::ClientHandle(const CR::ClientHandle &other){
    lastRecvOrder = 0;
    lastSentOrder = 1;
    svACK = 0;       
    lastFrame = 0;
    lastAudit = 0;
    readyNextFrame = true;
    lastPacketTimeout = CR::ticks();
    lastFrameACK = CR::ticks();
}

CR::ClientHandle::ClientHandle(){
    lastRecvOrder = 0;
    lastSentOrder = 1;
    svACK = 0;       
    lastFrame = 0;
    lastAudit = 0;
    readyNextFrame = true;
    lastPacketTimeout = CR::ticks();
    lastFrameACK = CR::ticks();
}


CR::ClientHandle *CR::NetHandle::getClientByIP(const CR::IP_Port &ip){
    for(auto _cl : clients){
        if(_cl.second->ip.isSame(ip)){
            return clients[_cl.first].get();
        }
    }
    return NULL;
}

CR::ClientHandle *CR::NetHandle::getClient(CR::T_GENERICID playerId){
    return clients.count(playerId) > 0 ? clients[playerId].get() : NULL;
}


std::vector<CR::IP_Port> CR::NetHandle::getAllClientsIPs(){
    std::vector<CR::IP_Port> ips;
    for(auto &it : clients){
        ips.push_back(it.second->ip);
    }
    return ips;    
}

std::vector<uint32> CR::NetHandle::getAllClientsACKs(){
    std::vector<uint32> acks;
    for(auto &it : clients){
        acks.push_back(++it.second->svACK);
    }
    return acks;
}


bool CR::NetHandle::removeClient(CR::T_GENERICID clientId){
    auto client = getClient(clientId);
    if(!client) return false;
    this->clients.erase(clientId);
    return true;
}

std::shared_ptr<CR::ClientHandle> CR::NetHandle::addClient(CR::T_GENERICID clientId, const std::string &nickname, const CR::IP_Port &ip){
    auto nclient = std::make_shared<CR::ClientHandle>(CR::ClientHandle());
    nclient->nickname = nickname;
    nclient->clientId = clientId;
    nclient->ip = ip;
    clients[nclient->clientId] = nclient;
    return nclient;
}


CR::NetHandle::NetHandle(){
    packetQueue.reserve(1024);
    rcvPackets.reserve(1024);
    netState = NetHandleState::IDLE;
    isServer = false;
}

void CR::NetHandle::sendPacketFor(const CR::IP_Port &ip, CR::Packet &packet){
    packet.receiver = ip;
    packetQueue.push_back(packet);
}

void CR::NetHandle::sendPacketForMany(const std::vector<CR::IP_Port> &ips, CR::Packet &packet){
    for(int i = 0; i < ips.size(); ++i){
        packet.receiver = ips[i];
        packetQueue.push_back(packet);
    }
}

void CR::NetHandle::sendPersPacketFor(const CR::IP_Port &ip, CR::Packet &packet, uint32 ack){
    packet.receiver = ip;
    packet.setAck(ack);
    packetQueue.push_back(packet);  
    auto pd = std::make_shared<CR::PersisentDelivey>(CR::PersisentDelivey());
    pd->ip = ip;
    pd->ack = ack;
    pd->header = packet.getHeader();
    pd->packet = packet;
    pd->lastRetry = CR::ticks();
    deliveries.push_back(pd);    
}

void CR::NetHandle::sendPersPacketForMany(const std::vector<CR::IP_Port> &ips, CR::Packet &packet, const std::vector<uint32> acks){
    for(int i = 0; i < ips.size(); ++i){
        auto &ip = ips[i];
        auto &ack = acks[i];
        packet.receiver = ip;
        packet.setAck(ack);
        packetQueue.push_back(packet);        
        auto pd = std::make_shared<CR::PersisentDelivey>(CR::PersisentDelivey());
        pd->ip = ip;
        pd->header = packet.getHeader();
        pd->ack = ack;
        pd->packet = packet;
        pd->lastRetry = CR::ticks();
        deliveries.push_back(pd);  
    }
}

void CR::NetHandle::updateDeliveries(){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(CR::ticks()-del->lastRetry > 1000){
            del->lastRetry = CR::ticks();
            sendPacketFor(del->ip, del->packet);
        }
    }  
}

void CR::NetHandle::dropPersFor(const CR::IP_Port &ip){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->ip.isSame(ip)){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void CR::NetHandle::dropPersForHeader(const CR::IP_Port &ip, uint16 header){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->ip.isSame(ip) && del->packet.getHeader() == header){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void CR::NetHandle::ack(CR::Packet &packet){
    uint32 ack = packet.getAck();
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack && pck->ip.isSame(packet.sender)){
            pck->onAck(pck->onAckPayload, packet.sender);
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void CR::NetHandle::sendAck(CR::IP_Port &ip, uint32 ackId){
    CR::Packet ack(CR::PacketType::ACK);
    ack.setAck(ackId);
    sendPacketFor(ip, ack);
}

void CR::NetHandle::bindOnAckFor(uint32 ack, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda, CR::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

void CR::NetHandle::bindOnAckFor(uint32 ack, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack){     
            pck->onAck = lambda;
        }
    }
}

void CR::NetHandle::bindOnAckForHeader(uint16 header, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda, CR::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->header == header){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

void CR::NetHandle::bindOnAckForHeader(uint16 header, std::function<void(CR::SmallPacket &payload, CR::IP_Port &ip)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->header == header){
            pck->onAck = lambda;
        }
    }
}