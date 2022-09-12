#ifndef CR_NETWORK_HPP
	#define CR_NETWORK_HPP

	#include "Tools.hpp"
	// #include "Indexer.hpp"
	// #include "Packets.hpp"

	namespace CR {
		static const uint16 NetworkDefaultPort = 14101; 
		static const uint16 NetworkDefaultFileTransferPort = 14102;	

		static const size_t NetworkHeaderSize = sizeof(uint16);
		static const size_t NetworkOrderSize = sizeof(uint32);
		static const size_t NetworkAckSize = sizeof(uint32);
		static const size_t NetworkMaxHeaderSize = NetworkHeaderSize + NetworkOrderSize + NetworkAckSize;
		static const size_t NetworkMaxPacketSize = 512; // 512 bytes is the maximum size for reliable packet transmission over the internet

		struct IP_Port {
			IP_Port();
			IP_Port(const std::string &ip, uint16 port);
			IP_Port(const CR::IP_Port &ip, uint16 nport);
			bool isBlack();
			void clear();
			std::string ip;
			uint16 port;
			int32 address;
			const std::string str();
			void set(const std::string &ip, uint16 port);
			operator std::string() const;
			bool operator== (const IP_Port &other);
			bool isSame(const IP_Port &other);
		};

		struct Packet {
			CR::IP_Port receiver;
			CR::IP_Port sender;
			size_t index;
			size_t maxSize;
			char data[CR::NetworkMaxPacketSize];
			Packet(uint16 header);
			Packet();
			~Packet();
			uint32 getOrder();
			void setOrder(uint32 order);
			uint32 getAck();
			void setAck(uint32 ack);
			uint16 getHeader();
			void setHeader(uint16 header);
			void copy(const Packet &other);
			size_t getSize();
			void reset();
			void clear();
			void setIndex(size_t index);
			bool write(const void *data, size_t Si);
			bool read(std::string &str);
			bool read(void *data, size_t Si);
			bool write(const std::string str);
			bool read(CR::Vec2<float> &v);
			bool write(const CR::Vec2<float>  &v);
			CR::Packet& operator= (const CR::Packet &other);
		};

		struct UDPSocket {
			int32 sock;
			uint16 port;
			bool opened;
			bool nonBlocking;
			UDPSocket();
			~UDPSocket();
			bool open(uint16 port);
			void close();
			bool setNonBlocking(bool m);
			size_t send(const IP_Port &dest, char *data, size_t size);
			size_t send(const IP_Port &dest, CR::Packet &packet);
			size_t recv(IP_Port &sender, char *buffer);
			size_t recv(IP_Port &sender, CR::Packet &buffer);
		};


		// namespace FileTransfer {

		// 	enum FTHeader : uint16 {
		// 		REQUEST = 0,
		// 		ACCEPT,
		// 		CANCELLED,
		// 		OK,
		// 		CHUNK,
		// 		DONE
		// 	};			

		// 	typedef std::function<void(const CR::IndexedFile &file, bool success)> FTCallback;

		// 	struct UDPClient;
		// 	struct FTSession {
		// 		FILE *file;
		// 		nite::FileTransfer::UDPClient *client;
		// 		nite::IP_Port ip;
		// 		nite::IndexedFile indexed;
		// 		UInt32 index;
		// 		UInt32 lastAdd;
		// 		bool sender;
		// 		UInt64 lastPing;
		// 		UInt64 initTime;
		// 		char buffer[nite::NetworkMaxPacketSize];
		// 		nite::FileTransfer::FTCallback callback;
		// 		String id;
		// 		UInt64 lastResend;
		// 		FTSession(){
		// 			index = 0;
		// 			file = NULL;
		// 			lastResend = nite::getTicks();
		// 			callback = [](const nite::IndexedFile &file, bool success){
						
		// 			};
		// 			ping();
		// 		}
		// 		// these got too long jeez (I dont like doing this, but this situation is an emergency)
		// 		void init(	const nite::IndexedFile &file, const nite::IP_Port &ip, bool sender,
		// 					nite::FileTransfer::UDPClient *client,
		// 					const nite::FileTransfer::FTCallback &callback){
		// 			this->indexed = file;
		// 			this->ip = ip;
		// 			this->callback = callback;
		// 			this->sender = sender;
		// 			this->client = client;
		// 			this->lastPing = nite::getTicks();
		// 			this->initTime = nite::getTicks();
		// 			this->index = 0;
		// 		}
		// 		void ping(){
		// 			this->lastPing = nite::getTicks();
		// 		}
		// 		UInt64 getLastPing(){
		// 			return nite::getTicks() - lastPing;
		// 		}
		// 		void open(){
		// 			if(file != NULL){
		// 				return;
		// 			}
		// 			ping();
		// 			file = fopen(indexed.path.c_str(), this->sender ? "rb" : "wb");
		// 		}
		// 		void write(char *buffer, UInt32 size){
		// 			open();
		// 			ping();
		// 			index += size;
		// 			fwrite(buffer, size, 1, file);
		// 		}
		// 		void read(char *buffer, UInt32 size){
		// 			open();
		// 			ping();
		// 			index += size;		
		// 			fread(buffer, size, 1, file);
		// 		}
		// 		void seek(UInt32 index){
		// 			fseek(file, index, SEEK_SET); 
		// 		}
		// 		void close(){
		// 			if(file != NULL){
		// 				fclose(file);
		// 				file = NULL;
		// 			}
		// 		}
		// 	};

		// 	struct UDPClient { 
		// 		// do not manipulate this field manually
		// 		Dict<String, FTSession> sessions;
		// 		nite::UDPSocket sock;
		// 		bool listening;
		// 		UInt16 port;
		// 		~UDPClient();
		// 		UDPClient();
		// 		void listen(UInt16 port);
		// 		void stop(const String &id = "");
		// 		void clear();
		// 		void request(	const nite::IP_Port &client,
		// 						const String &hash,
		// 						const String &output,
		// 						const nite::FileTransfer::FTCallback &callback = [](const IndexedFile &file, bool success){ return; },
		// 						bool overwrite = true);
		// 	};	
		// 	void init();
		// 	void end();		
		// }

	}

#endif