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

struct Character {
    CR::Vec2<float> inTexSize;
    CR::Vec2<float> frameSize;
    std::shared_ptr<CR::Gfx::Texture> atlas;

    CR::Gfx::MeshData meshFrame;
    CR::Gfx::Transform *transform;
    std::shared_ptr<CR::Gfx::Shader> charShader;

    unsigned lookAt;

    CR::Vec3<float> position;
    CR::EntityAnim anim;
    
    Character(){
        charShader = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
        atlas = std::make_shared<CR::Gfx::Texture>(CR::Gfx::Texture());
    }
    void load(){

        lookAt = CR::CharFace::SOUTH;
        atlas->load("data/texture/human_male.png");
        inTexSize.set(90, 90);
        frameSize = CR::Vec2<float>(90, 90) * CR::Vec2<float>(3.0f);

        float fwith = inTexSize.x / static_cast<float>(atlas->getRsc()->size.x);
        float fheight = inTexSize.y / static_cast<float>(atlas->getRsc()->size.y);
        this->meshFrame = CR::Gfx::createMesh({
            // POSITION
            -frameSize.x*0.5f,      -frameSize.y*0.5f,      0,    

            -frameSize.x*0.5f,      frameSize.y*0.5f,       0,              

            frameSize.x*0.5f,       frameSize.y*0.5f,       0,              


            frameSize.x*0.5f,       frameSize.y*0.5f,       0,              


            frameSize.x*0.5f,       -frameSize.y*0.5f,      0,              

            -frameSize.x*0.5f,      -frameSize.y*0.5f,      0,              
        }, {
            // TEXTURE COORs
            0,         0,
            0,         1.0f,
            1.0f,      1.0f,
            1.0f,      1.0f,
            1.0f,      0,
            0,         0               
        }, CR::Gfx::VertexStoreType::STATIC, CR::Gfx::VertexStoreType::DYNAMIC);

        this->charShader->load("data/shader/b_cube_texture_f.glsl", "data/shader/b_cube_texture_v.glsl");
        this->charShader->findAttrs({ "color", "model", "projection", "image", "view" });         

        this->transform = new CR::Gfx::Transform();
        this->transform->shader = charShader;
        this->transform->textures[CR::Gfx::TextureRole::DIFFUSE] = this->atlas->getRsc()->textureId;
        this->transform->shAttrsLoc = charShader->shAttrs;
        

        auto position = CR::MAT4Identity
                        .translate(CR::Vec3<float>(0.0f, -30.0f, 0.0f))
                        .rotate(0, CR::Vec3<float>(0, 0, 0))
                        .scale(CR::Vec3<float>(1.0f));

        this->transform->shAttrsVal = {
            {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
            {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(position)},
            {"view", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
            {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
            {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        };    
        this->transform->fixShaderAttributes({ "image", "model", "view", "projection", "color" });
               

        this->position.set(50, -frameSize.y * 0.25f, -25);

        anim.load("data/entity/base_human.json");
        
    }

    void render(bool nagger = false){

        static auto currentAnim = CR::AnimType::STAND_SOUTH;
        static auto frame = 0;
        static float inCoorsWidth = 90.0f / 1350.0f;
        static float inCoorsHeight = 90.0f / 1350.0f;
        static auto lastF = CR::ticks();        
        static float rate = 1.0f;
        static float speedo = 450 * rate;


        if(CR::Input::keyboardCheck(CR::Input::Key::SPACE)){
            rate = 5.0f;
            speedo = 450 * rate;
        }else{
            rate = 1.0f;
            speedo = 450;
        }

        if(CR::Input::keyboardCheck(CR::Input::Key::W) && CR::Input::keyboardCheck(CR::Input::Key::A)){
            position.z += CR::getDelta() * speedo;
            position.x -= CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::NORTH_WEST;
            currentAnim = CR::AnimType::WALKING_NORTH_WEST;
        }else     
        if(CR::Input::keyboardCheck(CR::Input::Key::W) && CR::Input::keyboardCheck(CR::Input::Key::D)){
            position.z += CR::getDelta() * speedo;
            position.x += CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::NORTH_EAST;
            currentAnim = CR::AnimType::WALKING_NORTH_EAST;
        }else 
        if(CR::Input::keyboardCheck(CR::Input::Key::W)){
            position.z += CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::NORTH;
            currentAnim = CR::AnimType::WALKING_NORTH;
        }else
           

        if(CR::Input::keyboardCheck(CR::Input::Key::S) && CR::Input::keyboardCheck(CR::Input::Key::A)){
            position.z -= CR::getDelta() * speedo;
            position.x -= CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::SOUTH_WEST;
            currentAnim = CR::AnimType::WALKING_SOUTH_WEST;
        }else 
        if(CR::Input::keyboardCheck(CR::Input::Key::S) && CR::Input::keyboardCheck(CR::Input::Key::D)){
            position.z -= CR::getDelta() * speedo;
            position.x += CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::SOUTH_EAST;
            currentAnim = CR::AnimType::WALKING_SOUTH_EAST;
        }else  
        if(CR::Input::keyboardCheck(CR::Input::Key::S)){
            position.z -= CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::SOUTH;
            currentAnim = CR::AnimType::WALKING_SOUTH;
        }else       
        if(CR::Input::keyboardCheck(CR::Input::Key::A)){
            position.x -= CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::WEST;
            currentAnim = CR::AnimType::WALKING_WEST;
        }else
        if(CR::Input::keyboardCheck(CR::Input::Key::D)){
            position.x += CR::getDelta() * speedo;
            this->lookAt = CR::CharFace::EAST;
            currentAnim = CR::AnimType::WALKING_EAST;
        }else{
            frame = 0;
            switch(lookAt){
                case CR::CharFace::NORTH: {
                    currentAnim = CR::AnimType::STAND_NORTH;
                } break;
                case CR::CharFace::NORTH_EAST: {
                    currentAnim = CR::AnimType::STAND_NORTH_EAST;
                } break;
                case CR::CharFace::NORTH_WEST: {
                    currentAnim = CR::AnimType::STAND_NORTH_WEST;
                } break;    
                case CR::CharFace::SOUTH_EAST: {
                    currentAnim = CR::AnimType::STAND_SOUTH_EAST;
                } break;
                case CR::CharFace::SOUTH_WEST: {
                    currentAnim = CR::AnimType::STAND_SOUTH_WEST;
                } break; 
                case CR::CharFace::SOUTH: {
                    currentAnim = CR::AnimType::STAND_SOUTH;
                } break;      
                case CR::CharFace::EAST: {
                    currentAnim = CR::AnimType::STAND_EAST;
                } break;          
                case CR::CharFace::WEST: {
                    currentAnim = CR::AnimType::STAND_WEST;
                } break;                                 
            }
        }           




        game->camera.position = this->position - CR::Vec3<float>(CR::Gfx::getWidth(), CR::Gfx::getHeight(), -CR::Gfx::getHeight()) * CR::Vec3<float>(0.5f);         



        auto diff = (game->camera.getCenter());

        auto normal = diff.normalize();


        // auto theta = CR::Math::asin(normal.dot(front));
        auto theta = CR::Math::degs(CR::Math::atan(game->camera.targetBias.z, game->camera.targetBias.x));

        // CR::log("%f\n", theta);


        if(CR::ticks()-lastF > static_cast<float>(this->anim.framerate) * (1.0f / rate)){
            ++frame;
            frame = frame % this->anim.anims[currentAnim].frames.size();
            lastF = CR::ticks();
        }

        game->renderOn([&](CR::Gfx::RenderLayer *layer){

            
            auto &animFrame = this->anim.anims[currentAnim].frames[frame];
            CR::Gfx::updateMesh(this->meshFrame, CR::Gfx::VertexRole::TEXCOORD, {
                animFrame.x,    animFrame.y,
                animFrame.x,    animFrame.h,
                animFrame.w,    animFrame.h,
                animFrame.w,    animFrame.h,
                animFrame.w,    animFrame.y,
                animFrame.x,    animFrame.y               
            });
            

            auto mposition = CR::MAT4Identity
                            .translate(position)
                            .rotate(0, CR::Vec3<float>(0, 1, 0))
                            .scale(CR::Vec3<float>(1.0f));            

            static_cast<CR::Gfx::ShaderAttrMat4*>(this->transform->shAttrsValVec[1])->mat = mposition;


            layer->add(CR::Gfx::Draw::Mesh(this->meshFrame, this->transform));
        });  

    };
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
    // gameL->camera.targetBias = CR::Vec3<float>(-55.0f, -35.0f, -75.0f);
    // gameL->camera.targetBias = CR::Vec3<float>(-55.0f, -55.0f, -75.0f);

    game = CR::Gfx::getRenderLayer("world", true);

    // std::shared_ptr<CR::Map::Map> map = std::make_shared<CR::Map::Map>(CR::Map::Map()); 

    // map->build(CR::Vec2<int>(32, 32), 8, 50);

    
    CR::Server server;
    server.listen("Social Room", 24, CR::NetworkDefaultPort);
    CR::spawn([&](CR::Job &Ctx){
        server.step();
    }, true, true, false);

    CR::Client client;
    CR::spawn([&](CR::Job &Ctx){
        client.connect("127.0.0.1", CR::NetworkDefaultPort);
    }, false, false, false);

    Character player;

    player.load();
    
    auto font = std::make_shared<CR::Gfx::Font>(CR::Gfx::Font());

    auto style = CR::Gfx::FontStyle(32, CR::Gfx::FontEncondig::ASCII, 2.0f, CR::Gfx::FontStyleType::SOLID);
    font->load("data/font/nk57_monoscape.ttf", style);

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


        // if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD8)){
        //     gameL->camera.targetBias.z += CR::getDelta() * 20;
        // }
        // //DOWN
        // if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD5)){
        //     gameL->camera.targetBias.z -= CR::getDelta() * 20;
        // }
        // // LEFT    
        // if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD4)){
        //     gameL->camera.targetBias.x -= CR::getDelta() * 20;
        // }
        // // RIGHT
        // if(CR::Input::keyboardCheck(CR::Input::Key::NUMPAD6)){
        //     gameL->camera.targetBias.x += CR::getDelta() * 20;
        // }  

        // LEFT    
        if(CR::Input::keyboardCheck(CR::Input::Key::SUBTRACT)){
            gameL->camera.targetBias.y -= CR::getDelta() * 20;
        }
        // RIGHT
        if(CR::Input::keyboardCheck(CR::Input::Key::ADD)){
            gameL->camera.targetBias.y += CR::getDelta() * 20;
        } 


        // if(CR::Input::keyboardPressed(CR::Input::Key::NUMPAD0)){
        //     gameL->camera.targetBias = CR::Vec3<float>(-55.0f, -35.0f, -75.0f);
        // }    

        // CR::log("%f\n", CR::getDelta());
        // UP
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

        client.step();

        // map->render();
        player.render();
        CR::Gfx::render();
    }

    // map->clear();

    CR::Gfx::onEnd();

    return 0;
}