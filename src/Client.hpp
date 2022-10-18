#ifndef CR_GAME_CLIENT_HPP
    #define CR_GAME_CLIENT_HPP

    #include "Network.hpp"
    #include "World.hpp"


    namespace CR {


        
        struct Client : NetHandle {
            CR::World *world;
            CR::IP_Port svAddress;
            bool isHandleRunning;
            uint64 connReqAt;
            Client();
            bool connect(const std::string &ip, unsigned port, CR::World *world);
            void end();
            void step();

        };



    }

#endif
