#include "Client.hpp"



static void CL_SEND_CONNECT(CR::Client* cl){
    CR::Packet connPacket;
    connPacket.setOrder(0);
    connPacket.setHeader(CR::PacketType::CONNECT_REQUEST);
    connPacket.write(&CR::GAME_VERSION, sizeof(CR::T_VERSION_SEG) * 3);
    connPacket.write("PLAYER");
    connPacket.write("#");
    connPacket.write("#");
    cl->socket.send(cl->svAddress, connPacket);
    cl->connReqAt = CR::ticks();
    ++cl->connRet;
};

static void CL_THREAD(void *handle){
    auto cl = static_cast<CR::Client*>(handle);
    CR::Packet packet;
    CR::IP_Port sender;        
    cl->isHandleRunning = true;

    while(CR::NetHandleState::IDLE) CR::sleep(16);
    while(CR::NetHandleState::CONNECTING){
        std::unique_lock<std::mutex> lock(cl->netCon);
            int nb = cl->socket.recv(sender, packet);
            if(nb > 0){
                switch(packet.getHeader()){
                    case CR::PacketType::CONNECT_ACCEPT: {
                        std::string seasonName;
                        uint64 sessionId;
                        CR::T_CLIENTID clientId;
                        packet.read(seasonName);
                        packet.read(&sessionId, sizeof(uint64));
                        packet.read(&clientId, sizeof(CR::T_CLIENTID));
                        CR::log("[CLIENT] Connected to server (%s): Session Name/ID '%s/%i' | Client ID '%i' \n", cl->svAddress.str().c_str(), seasonName.c_str(), sessionId, clientId);
                        cl->netState = CR::NetHandleState::CONNECTED;
                        lock.unlock();
                        cl->socket.setNonBlocking(false); // switch to blocking from now on
                    } break;
                    case CR::PacketType::CONNECT_REJECT: {
                        std::string reason;
                        packet.read(reason);
                        if(reason.length() == 0){
                            reason = "no reason given";
                        }
                        CR::log("[CLIENT] Connection rejected to server (%s): Reason: %s\n", cl->svAddress.str().c_str(), reason.c_str());
                        cl->netState = CR::NetHandleState::IDLE;
                        lock.unlock();
                        return;                                
                    } break;                            
                }
            }else
             // TODO: implement retry
            if(CR::ticks()-cl->connReqAt > CR::Net::CLIENT_CONNECT_TIMEOUT && cl->connRet < CR::Net::CLIENT_CONNECT_TRY_TIMEOUT_INTERVAL){
                CR::log("[CLIENT] Retrying connecting to %s (%i retries)...\n", cl->svAddress.str().c_str(), cl->connRet);   
                CL_SEND_CONNECT(cl);
            }else
            if(CR::ticks()-cl->connReqAt > CR::Net::CLIENT_CONNECT_TIMEOUT && cl->connRet == CR::Net::CLIENT_CONNECT_TRY_TIMEOUT_INTERVAL){
                CR::log("[CLIENT] Failed to connect to server (%s): Server didn't respond (%i retries)\n", cl->svAddress.str().c_str(), cl->connRet);
                cl->netState = CR::NetHandleState::IDLE;
                lock.unlock();
                return;
            }  
        lock.unlock();
    }
    while(CR::NetHandleState::CONNECTED){
        std::unique_lock<std::mutex> lock(cl->netCon);
        int nb = cl->socket.recv(sender, packet);
        if(nb > 0){

        }
        lock.unlock();
    }
    cl->isHandleRunning = false;
}

CR::Client::Client(){
    this->isServer = false;
    this->isHandleRunning = false;
    this->netState = CR::NetHandleState::IDLE;
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
    std::unique_lock<std::mutex> lock(this->netCon);
        if(!this->socket.open(chosenPort)){
            CR::log("[CLIENT] Failed to open UDP Socket at %i\n", chosenPort);
            lock.unlock();
            return false;
        }
        socket.setNonBlocking(true);
        this->thread = std::thread(&CL_THREAD, this); 
        this->netState = CR::NetHandleState::CONNECTING;
        CL_SEND_CONNECT(this);
        CR::log("[CLIENT] Connecting to %s...\n", this->svAddress.str().c_str());
    lock.unlock();
    return true;
}

void CR::Client::end(){
    
}

void CR::Client::step(){

}