#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"


int main(int argc, char* argv[]){
    
    CR::Gfx::loadSettings(CR::Core::loadParams(argc, argv), "data/cfg/settings.json");

    CR::Gfx::init();

    while(CR::Gfx::isRunning()){
        CR::Gfx::render();
    }

    CR::Gfx::onEnd();

    return 0;
}