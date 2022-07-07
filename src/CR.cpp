#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Job.hpp"
#include "Engine/Input.hpp"


int main(int argc, char* argv[]){
    
    CR::Gfx::loadSettings(CR::Core::loadParams(argc, argv), "data/cfg/settings.json");

    CR::Gfx::init();

    // setup gfx
    auto gameL = CR::Gfx::createRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_3D, "world", true, 0);
    auto uiL = CR::Gfx::createRenderLayer(CR::Gfx::getSize(), CR::Gfx::RenderLayerType::T_2D, "ui", true, 1);

    gameL->camera.setPosition(CR::Vec3<float>(CR::Vec3<float>(35.0f, 35.0f, 35.0f)));
    gameL->camera.setTarget(CR::Vec3<float>(-70.0f, -70.0f, -70.0f));
    gameL->camera.setUp(CR::Vec3<float>(0.0f, 1.0f, 0.0f));

    // gameL->camera.biasAngle = 

    while(CR::Gfx::isRunning()){

		// float cameraSpeed = CR::getDelta(); 
		// if (CR::Input::keyboardCheck(CR::Input::Key::W))
		// 	gameL->camera.setPosition(gameL->camera.position - CR::Vec3<float>(0.0f, 0.0f, 1.0f) * cameraSpeed);
		// if (CR::Input::keyboardCheck(CR::Input::Key::S))
		// 	gameL->camera.setPosition(gameL->camera.position + CR::Vec3<float>(0.0f, 0.0f, 1.0f) * cameraSpeed);

		// if (CR::Input::keyboardCheck(CR::Input::Key::A))
		// 	gameL->camera.setPosition(gameL->camera.position + CR::Vec3<float>(1.0f, 0.0f, 0.0f) * cameraSpeed);
		// if (CR::Input::keyboardCheck(CR::Input::Key::D))
		// 	gameL->camera.setPosition(gameL->camera.position - CR::Vec3<float>(1.0f, 0.0f, 0.0f) * cameraSpeed);			



        CR::Gfx::render();
    }

    CR::Gfx::onEnd();

    return 0;
}