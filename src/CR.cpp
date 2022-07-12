#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Job.hpp"
#include "Engine/Input.hpp"
#include "Map.hpp"

int main(int argc, char* argv[]){
    
    CR::Gfx::loadSettings(CR::Core::loadParams(argc, argv), "data/cfg/settings.json");

    CR::Gfx::init();

    // setup gfx
    auto gameL = CR::Gfx::createRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_3D, "world", true, 0);
    auto uiL = CR::Gfx::createRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_2D, "ui", true, 1);

    // setup camera
    gameL->camera.setPosition(CR::Vec3<float>(CR::Vec3<float>(-106.964981, -500.000000, 405.812775)));    
    gameL->camera.setTarget(CR::Vec3<float>(0, 0, 0));
    gameL->camera.setUp(CR::Vec3<float>(0.0f, 1.0f, 0.0f));
    gameL->camera.targetBias = CR::Vec3<float>(-55.0f, -35.0f, -75.0f);
    // gameL->camera.targetBias = CR::Vec3<float>(-55.0f, -55.0f, -75.0f);

    std::shared_ptr<CR::Map::Map> map = std::make_shared<CR::Map::Map>(CR::Map::Map()); 

    map->build(CR::Vec2<int>(32, 32), 100);

    while(CR::Gfx::isRunning()){

        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD8)){
            gameL->camera.targetBias.z += CR::getDelta() * 20;
        }
        //DOWN
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD5)){
            gameL->camera.targetBias.z -= CR::getDelta() * 20;
        }
        // LEFT    
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD4)){
            gameL->camera.targetBias.x -= CR::getDelta() * 20;
        }
        // RIGHT
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD6)){
            gameL->camera.targetBias.x += CR::getDelta() * 20;
        }  

        // LEFT    
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD9)){
            gameL->camera.targetBias.y -= CR::getDelta() * 20;
        }
        // RIGHT
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD3)){
            gameL->camera.targetBias.y += CR::getDelta() * 20;
        } 


        if(CR::Input::keyboardPressed(CR::Input::Key::NUMPAD0)){
            gameL->camera.targetBias = CR::Vec3<float>(-55.0f, -35.0f, -75.0f);
        }    

        // CR::log("%f\n", CR::getDelta());
        // UP
        if(CR::Input::keyboardCheck(CR::Input::Key::W)){
            gameL->camera.position.z += CR::getDelta() * 2000;
        }
        //DOWN
        if(CR::Input::keyboardCheck(CR::Input::Key::S)){
            gameL->camera.position.z -= CR::getDelta() * 2000;
        }
        // LEFT    
        if(CR::Input::keyboardCheck(CR::Input::Key::A)){
            gameL->camera.position.x -= CR::getDelta() * 2000;
        }
        // RIGHT
        if(CR::Input::keyboardCheck(CR::Input::Key::D)){
            gameL->camera.position.x += CR::getDelta() * 2000;
        }    

        map->render();
        CR::Gfx::render();
    }

    map->clear();

    CR::Gfx::onEnd();

    return 0;
}