#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <WS2tcpip.h>
#else
    #include <stdio.h>
	#include <errno.h> 
	#include <unistd.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/types.h> 
	#include <netinet/in.h>
	#include <fcntl.h>
#endif

#include "Network.hpp"
#include "Log.hpp"

CR::IP_Port::IP_Port(const std::string &ip, uint16 port){
	set(ip, port);
}

CR::IP_Port::IP_Port(){
	set("127.0.0.1", NetworkDefaultPort);
}

CR::IP_Port::IP_Port(const CR::IP_Port &ip, uint16 nport){
	set(ip.ip, nport);
}

bool CR::IP_Port::operator== (const IP_Port &other){
	return isSame(other);
}

bool CR::IP_Port::isSame(const IP_Port &other){
	return this->address == other.address && this->port == other.port;
}

void CR::IP_Port::set(const std::string &ip, uint16 port){
	this->port = port;
	this->ip = ip;
	this->address = inet_addr(ip.c_str());
}

bool CR::IP_Port::isBlank(){
	return address == 0 && port == 0;
}

void CR::IP_Port::clear(){
	address = 0;
	port = 0;
}

const std::string CR::IP_Port::str(){
	return (std::string)*this;
}

CR::IP_Port::operator std::string() const {
	return ip +":"+ std::to_string(port);
}

CR::Packet::Packet(uint16 Header){
	clear();
	setHeader(Header);
}

CR::Packet::Packet(){
	clear();
}

CR::Packet::~Packet(){

}

void CR::Packet::copy(const Packet &other){ 
	maxSize = other.maxSize;
	memcpy(this->data, other.data, CR::NetworkMaxPacketSize);
	reset();
}

CR::Packet& CR::Packet::operator= (const CR::Packet &other){
	copy(other);
	return *this;
}

void CR::Packet::clear(){
	memset(data, '0', CR::NetworkMaxPacketSize);
	index = CR::NetworkHeaderSize + CR::NetworkOrderSize + NetworkAckSize; // starts from the send byte mark
	maxSize = index;
}

size_t CR::Packet::getSize(){
	return maxSize - (CR::NetworkHeaderSize + CR::NetworkOrderSize + NetworkAckSize);
}

void CR::Packet::setHeader(uint16 header){
	memcpy(data, &header, CR::NetworkHeaderSize);
}

uint16 CR::Packet::getHeader(){
	uint16 header;
	memcpy(&header, data, CR::NetworkHeaderSize);
	return header;
}

uint32 CR::Packet::getOrder(){
	uint32 order;
	memcpy(&order, data + CR::NetworkHeaderSize, CR::NetworkOrderSize);
	return order;
}

void CR::Packet::setOrder(uint32 order){
	memcpy(data + CR::NetworkHeaderSize, &order, CR::NetworkOrderSize);
}

uint32 CR::Packet::getAck(){
	uint32 ack;
	memcpy(&ack, data + CR::NetworkHeaderSize + CR::NetworkOrderSize, CR::NetworkAckSize);
	return ack;
}

void CR::Packet::setAck(uint32 ack){
	memcpy(data + CR::NetworkHeaderSize + CR::NetworkOrderSize, &ack, CR::NetworkAckSize);
}


void CR::Packet::reset(){
	index = CR::NetworkHeaderSize + CR::NetworkOrderSize + NetworkAckSize; 
}

void CR::Packet::setIndex(size_t index){
	this->index = index;
}

bool CR::Packet::write(const std::string str){
	if((index >= CR::NetworkMaxPacketSize) || (index + str.length() + 1 > CR::NetworkMaxPacketSize)){
        CR::log("failed to write to packet: too big\n");
		return false;
	}
	size_t sl = str.length() + 1;
	memcpy(data + index, str.c_str(), sl);
	maxSize += sl; 
	index += sl;
	return true;
}

bool CR::Packet::read(std::string &str){
	if(index >= CR::NetworkMaxPacketSize){
		return false;
	}
	for(size_t i = index; i < CR::NetworkMaxPacketSize; ++i){
		if (this->data[i] == '\0'){
			size_t size = i - index; // don't include the nullterminated
			char buff[size];
			memcpy(buff, data + index, size);
			str = std::string(buff, size);
			index = i + 1;
			return true;
		}
	}
	return false;
}

bool CR::Packet::write(const void *data, size_t size){
	if(size == 0){
		return false;
	}
	if((index >= CR::NetworkMaxPacketSize) || (index + size > CR::NetworkMaxPacketSize)){
        CR::log("failed to write to packet: too big\n");
		return false;
	}
	memcpy(this->data + index, data, size);
	maxSize += size; 
	index += size;
	return true;
}

bool CR::Packet::read(void *data, size_t size){
	if((index >= CR::NetworkMaxPacketSize) || (index + size > CR::NetworkMaxPacketSize)){
		return false;
	}
	memcpy(data, this->data + index, size);
	index += size;
	return true;
}

bool CR::Packet::read(CR::Vec2<float> &v){
	bool x = false, y = false;
	x = this->read(&v.x, sizeof(v.x));
	y = this->read(&v.y, sizeof(v.y));	
	return x && y;
}

bool CR::Packet::write(const CR::Vec2<float>  &v){
	bool x = false, y = false;
	x = this->write(&v.x, sizeof(v.x));
	y = this->write(&v.y, sizeof(v.y));	
	return x && y;
}


void __CR_init_network(){
	#ifdef _WIN32
		WSADATA WsaData;
		if(WSAStartup(MAKEWORD(2,2), &WsaData) != 0){
			CR::log("failed to initialize Winsock\n"); // TODO: add proper error handling 
		}
	#endif
}

void __CR_end_network(){
	#ifdef _WIN32
		WSACleanup();
	#endif
}













static size_t Rx = 0;
static size_t Sx = 0;

static void close_(int s){
	#ifdef _WIN32
		closesocket(s);
	#else
		close(s);
	#endif
}

CR::UDPSocket::UDPSocket(){
	opened = false;
	nonBlocking = false;
}

CR::UDPSocket::~UDPSocket(){
	if (opened){
		close_(sock);
	}
}


bool CR::UDPSocket::open(uint16 port){
	this->port = port;
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		CR::log("failed to create UDP socket\n"); // TODO: add proper error handling
		return false;
	}
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(sock, (const sockaddr*) &addr, sizeof(sockaddr_in)) > 0){
 		CR::log("failed to bind UDP socket to port %i\n", port); // TODO: add proper error handling
		close_(sock);
		return false;
	}
	opened = true;
	CR::log("UDP socket opened at %i (%i)\n", port, sock);
	return true;
}

void CR::UDPSocket::close(){
	if (opened){
		close_(sock);
		opened = false;
		CR::log("UDP socket closed at %i (%i)\n", port, sock);

	}
}

bool CR::UDPSocket::setNonBlocking(bool m){
	if (m == nonBlocking){
		return false;
	}
	#ifdef _WIN32
		u_long b = m ? 1 : 0;
		if (ioctlsocket(sock, FIONBIO, &b) != 0){
			nonBlocking = m; // TODO: add proper error handling
			return true;
		}
	#else
		if (fcntl(sock, F_SETFL, O_NONBLOCK, m) == -1){
			nonBlocking = m; // TODO: add proper error handling
			return true;
		}
	#endif
	return false;
}

size_t CR::UDPSocket::send(const IP_Port &dest, char *data, size_t size){
	if (!opened){
		return false;
	}
	// handicap for testing
	// if(CR::randomInt(1, 3) == 1){
	// 	return 0;
	// }
	sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dest.port);
	addr.sin_addr.s_addr = dest.address;
	ssize_t s = sendto(sock, data, size, 0, (sockaddr*)&addr, sizeof(sockaddr_in)); // TODO: add proper error handling
	if(s > 0){
		Sx += s;
	}
	return s;
}

size_t CR::UDPSocket::send(const IP_Port &dest, CR::Packet &packet){
	return send(dest, packet.data, packet.maxSize);
}

size_t CR::UDPSocket::recv(IP_Port &sender, char *buffer){
	sockaddr_in from;
	socklen_t as = sizeof(sockaddr);
	ssize_t s = recvfrom(sock, buffer, NetworkMaxPacketSize, 0, (sockaddr*)&from, &as); // TODO: add proper error handling
	sender.address = from.sin_addr.s_addr;
	sender.port = ntohs(from.sin_port);
	sender.ip = inet_ntoa(from.sin_addr);
	if(s > 0){
		Sx += s;
	}
	return s < 0 ? 0 : s;
}
	
size_t CR::UDPSocket::recv(IP_Port &sender, CR::Packet &buffer){
	buffer.reset();
	size_t s = recv(sender, buffer.data);
	buffer.sender = sender;
	buffer.maxSize = s;
	return s;
}