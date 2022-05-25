#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"


int main(int argc, char* argv[]){
    
    CR::Gfx::loadSettings(CR::Core::loadParams(argc, argv), "data/cfg/settings.json");

    CR::Gfx::init();

    // setup gfx
    auto gameL = CR::Gfx::addRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_3D, "game", true, 0);
    auto uiL = CR::Gfx::addRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_2D, "ui", true, 1);

    while(CR::Gfx::isRunning()){
        CR::Gfx::render();
    }

    CR::Gfx::onEnd();

    return 0;
}