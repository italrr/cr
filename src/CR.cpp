#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Font.hpp"
#include "Engine/Model.hpp"
#include "Engine/Job.hpp"
#include "Engine/Input.hpp"
#include "Map.hpp"
#include "Entity.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "World.hpp"

static std::shared_ptr<CR::Gfx::RenderLayer> game;

namespace MeshType {
    enum MeshType : unsigned {
        UNIT_CUBE = 0,
        TALL_CUBE,
        PLANE,
    };
}

static CR::Gfx::MeshData buildMesh(float cubeScale, const std::vector<CR::Map::AtlasSingle> &faces, unsigned type){

    auto FTS    = CR::Map::FaceType::SOUTH;    
    auto FTN    = CR::Map::FaceType::NORTH;    
    auto FTW    = CR::Map::FaceType::WEST;    
    auto FTE    = CR::Map::FaceType::EAST;    
    auto FTT    = CR::Map::FaceType::TOP;    
    auto FTB    = CR::Map::FaceType::BOTTOM;    

    auto mesh = CR::Gfx::MeshData();

    switch(type){
        case MeshType::PLANE: {
            mesh = CR::Gfx::createMesh({
                // POSITION                             // TEXTURE
                // TOP
                -cubeScale,  cubeScale, -cubeScale,
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
            },{
                faces[FTB].coors.x,   faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].size.y                
            });
            mesh.vertn = 6;
        } break;
        case MeshType::UNIT_CUBE: {

            mesh = CR::Gfx::createMesh({ 
                // POSITION                             // TEXTURE

                // SOUTH
                -cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     

                cubeScale,  cubeScale, -cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                //  NORTH
                -cubeScale, -cubeScale,  cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     

                // WEST
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                -cubeScale, -cubeScale, -cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     

                // EAST
                cubeScale,  cubeScale,  cubeScale,    
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     

                cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     

                // TOP
                -cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     

                cubeScale, -cubeScale,  cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                // BOTTOM
                -cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
            }, {
faces[FTS].coors.x,   faces[FTS].coors.y,
faces[FTS].size.x,    faces[FTS].coors.y,
faces[FTS].size.x,    faces[FTS].size.y,
faces[FTS].size.x,    faces[FTS].size.y,
faces[FTS].coors.x,   faces[FTS].size.y,
faces[FTS].coors.x,   faces[FTS].coors.y,
faces[FTN].coors.x,   faces[FTN].coors.y,
faces[FTN].size.x,    faces[FTN].coors.y,
faces[FTN].size.x,    faces[FTN].size.y,
faces[FTN].size.x,    faces[FTN].size.y,
faces[FTN].coors.x,   faces[FTN].size.y,
faces[FTN].coors.x,   faces[FTN].coors.y,
faces[FTW].size.x,    faces[FTW].size.y,
faces[FTW].coors.x,   faces[FTW].size.y,
faces[FTW].coors.x,   faces[FTW].coors.y,
faces[FTW].coors.x,   faces[FTW].coors.y,
faces[FTW].size.x,    faces[FTW].coors.y,
faces[FTW].size.x,    faces[FTW].size.y,
faces[FTE].size.x,    faces[FTE].size.y,
faces[FTE].coors.x,   faces[FTE].size.y,
faces[FTE].coors.x,   faces[FTE].coors.y,
faces[FTE].coors.x,   faces[FTE].coors.y,
faces[FTE].size.x,    faces[FTE].coors.y,
faces[FTE].size.x,    faces[FTE].size.y,
faces[FTT].size.x,    faces[FTT].size.y,
faces[FTT].coors.x,   faces[FTT].size.y,
faces[FTT].coors.x,   faces[FTT].coors.y,
faces[FTT].coors.x,   faces[FTT].coors.y,
faces[FTT].size.x,    faces[FTT].coors.y,
faces[FTT].size.x,    faces[FTT].size.y,
faces[FTB].coors.x,   faces[FTB].size.y,
faces[FTB].size.x,    faces[FTB].size.y,
faces[FTB].size.x,    faces[FTB].coors.y,
faces[FTB].size.x,    faces[FTB].coors.y,
faces[FTB].coors.x,   faces[FTB].coors.y,
faces[FTB].coors.x,   faces[FTB].size.y
            });

        } break;        
        case MeshType::TALL_CUBE: {
            CR::Vec3<float> offset = CR::Vec3<float>(0, -cubeScale * 2, 0);

            mesh = CR::Gfx::createMesh({ 
                // POSITION                             // TEXTURE

                // SOUTH
                -cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     

                cubeScale,  cubeScale, -cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                //  NORTH
                -cubeScale, -cubeScale,  cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     

                // WEST
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                -cubeScale, -cubeScale, -cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     

                // EAST
                cubeScale,  cubeScale,  cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     

                cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     


                // BOTTOM
                -cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     

                // second level
                // SOUTH
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                  
                cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                  

                cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                  
                -cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                 
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 


                //  NORTH
                -cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                
                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 
                
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 
                -cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y,   cubeScale + offset.z,               

                // WEST
                -cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                

                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                


                // EAST
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 
                cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 

                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 

                // TOP
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                
                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 

                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 
                -cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                

            }, {
                faces[FTS].coors.x,   faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].coors.y,
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,
                faces[FTE].size.x,    faces[FTE].size.y,
                faces[FTE].coors.x,   faces[FTE].size.y,
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].size.y,
                faces[FTB].coors.x,   faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].size.y,
                faces[FTS].coors.x,   faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].coors.y,      
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,  
                faces[FTE].size.x,    faces[FTE].size.y,   
                faces[FTE].coors.x,   faces[FTE].size.y,  
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].size.y,
                faces[FTT].size.x,    faces[FTT].size.y,
                faces[FTT].coors.x,   faces[FTT].size.y,
                faces[FTT].coors.x,   faces[FTT].coors.y,
                faces[FTT].coors.x,   faces[FTT].coors.y,
                faces[FTT].size.x,    faces[FTT].coors.y,
                faces[FTT].size.x,    faces[FTT].size.y,                
            });
            // mesh.vertn = 60;
        } break;
    }
    return mesh;
};



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
    CR::JobSpec svSpec(true, true, true, {"TEMPORARY_SERVER"});
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





    CR::Gfx::Texture boxTexture;
    boxTexture.load("data/texture/container.png");

    CR::Gfx::Font font;
    CR::Gfx::FontStyle style;
    style.outlineThickness = 5;
    style.size = 64;
    font.load("data/font/nk57_monoscape.ttf", style);

    CR::Gfx::TextRenderOpts opts;
    opts.fill = CR::Color(1.0f, 0.0f, 0.0f, 1.0f);
    opts.outline = CR::Color(1.0f, 1.0f, 1.0f, 1.0f);
    opts.horBearingBonus = 3;

    auto sh = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());;
    sh->load("data/shader/b_cube_texture_f.glsl", "data/shader/b_cube_texture_v.glsl");
    sh->findAttrs({"color", "model", "projection", "image", "view"});

    CR::Gfx::Model model;
    model.load("data/model/Male_Standing.fbx");

    auto transform = new CR::Gfx::Transform();
    transform->shader = sh;
    transform->textures[CR::Gfx::TextureRole::DIFFUSE] = boxTexture.getRsc()->textureId;
    transform->shAttrsLoc = sh->shAttrs;

    auto position = CR::MAT4Identity
                    .translate(CR::Vec3<float>(0.0f, -30.0f, 0.0f))
                    .rotate(0, CR::Vec3<float>(0, 0, 0))
                    .scale(CR::Vec3<float>(50.0f));

    transform->shAttrsVal = {
        {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
        {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(position)},
        {"view", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
        {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
        {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
    };    
    transform->fixShaderAttributes({ "image", "model", "view", "projection", "color" });
            
    auto fakepos = CR::Vec3<float>(50, 0.0f, -25);

        // game->camera.position = CR::Vec3<float>(CR::Gfx::getWidth(), CR::Gfx::getHeight(), -CR::Gfx::getHeight()) * CR::Vec3<float>(0.5f);         



    std::shared_ptr<CR::Map::Map> map = std::make_shared<CR::Map::Map>(CR::Map::Map()); 
    map->build(CR::Vec2<int>(10, 10), 8, 50);


    while(CR::Gfx::isRunning()){


        // UP_LEFT
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD7)){
            float amnt = CR::getDelta() * 2000;
            gameL->camera.position.z += amnt;
            gameL->camera.position.x -= amnt;
        }
        //DOWN_LEFT
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD3)){


            float amnt = CR::getDelta() * 2000;

            gameL->camera.position.z -= amnt;
            gameL->camera.position.x += amnt;            
        }
        // UP_RIGHT    
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD9)){
            float amnt = CR::getDelta() * 2000;

            gameL->camera.position.z += amnt;
            gameL->camera.position.x += amnt;
        }
        // DOWN_RIGHTH
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD1)){
            float amnt = CR::getDelta() * 2000;

            gameL->camera.position.z -= CR::getDelta() * 2000;
            gameL->camera.position.x -= CR::getDelta() * 2000;
        }  

        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD8)){
            gameL->camera.position.z += CR::getDelta() * 2000;
        }
        //DOWN
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD2)){
            gameL->camera.position.z -= CR::getDelta() * 2000;
        }
        // LEFT    
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD4)){
            gameL->camera.position.x -= CR::getDelta() * 2000;
        }
        // RIGHT
        if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD6)){
            gameL->camera.position.x += CR::getDelta() * 2000;
        }            


        gameL->renderOn([&](CR::Gfx::RenderLayer *layer){
            auto rsc = model.getRsc();

            for(unsigned i = 0; i < rsc->mesh.size(); ++i){
                layer->add(CR::Gfx::Draw::Mesh(rsc->mesh[i]->md, transform));
            }

        });


        // uiL->renderOn([&](CR::Gfx::RenderLayer *layer){
        //     if(server.clients.size() < 1 || server.world->objects.size() < 1){
        //         return;
        //     }
        //     auto ppos = CR::String::format("Player %i,%i", server.world->objects[0]->loc->coords.x, server.world->objects[0]->loc->coords.y);
        //     layer->add(CR::Gfx::Draw::Text(font.getRsc(), ppos, CR::Vec2<float>(25.0f), opts));
        // });
        // map->render();
        client.step();
        CR::Gfx::render();
    }


    CR::Gfx::onEnd();

    return 0;
}