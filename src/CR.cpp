#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Font.hpp"
#include "Engine/Job.hpp"
#include "Engine/Input.hpp"
#include "Map.hpp"
#include "Entity.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "World.hpp"

static std::shared_ptr<CR::Gfx::RenderLayer> game;




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
    gameL->camera.targetBias = CR::Vec3<float>(-25, -65.0f, -75.0f);
    game = CR::Gfx::getRenderLayer("world", true);

    CR::Server server;
    CR::JobSpec svSpec(true, true, false, {"TEMPORARY_SERVER"});
    server.listen("Social Room", 24, CR::NetworkDefaultPort);    
    CR::spawn([&](CR::Job &Ctx){
        server.step();
    }, svSpec); 


    // imitate start up script  
    // if(!puppetMode){
    //     CR::spawn([&](CR::Job &ctx){
    //         auto job = CR::findJob({"TEMPORARY_SERVER"}, 1);
    //         // job->addBacklog([&](CR::Job &ctx){
    //         //     // add player
    //         //     // auto paudit = createEntity("PLAYER", CR::EntityType::PLAYER, CR::GridLoc(0, 0));
    //         // });
    //     }, false, false, false, 1000);
    // }    

    CR::Client client;
    CR::spawn([&](CR::Job &Ctx){
        client.connect("127.0.0.1", CR::NetworkDefaultPort);
    }, false, false, false, 100);


    while(CR::Gfx::isRunning()){
        client.step();
        CR::Gfx::render();
    }


    CR::Gfx::onEnd();

    return 0;
}