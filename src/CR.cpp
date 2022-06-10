#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Job.hpp"


int main(int argc, char* argv[]){
    
    CR::Gfx::loadSettings(CR::Core::loadParams(argc, argv), "data/cfg/settings.json");

    CR::Gfx::init();

    // setup gfx
    auto gameL = CR::Gfx::createRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_3D, "world", true, 0);
    auto uiL = CR::Gfx::createRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_2D, "ui", true, 1);

    while(CR::Gfx::isRunning()){
        CR::Gfx::render();
    }

    CR::Gfx::onEnd();

    return 0;
}