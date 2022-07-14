#include "Engine/Log.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Job.hpp"
#include "Engine/Input.hpp"
#include "Map.hpp"

static std::shared_ptr<CR::Gfx::RenderLayer> game;

struct Character {
    CR::Vec2<float> inTexSize;
    CR::Vec2<float> frameSize;
    std::shared_ptr<CR::Gfx::Texture> atlas;
    CR::Gfx::MeshData meshFrame;
    CR::Gfx::Transform *transform;
    std::shared_ptr<CR::Gfx::Shader> charShader;
    Character(){
        charShader = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
        atlas = std::make_shared<CR::Gfx::Texture>(CR::Gfx::Texture());
    }
    void load(){
        atlas->load("data/texture/male_body_assassin.png");
        inTexSize.set(50, 90);
        frameSize = CR::Vec2<float>(50, 90) * CR::Vec2<float>(3.0f);

        float inCoorsWidth = inTexSize.x / static_cast<float>(atlas->getRsc()->size.x);
        float inCoorsHeight = inTexSize.y / static_cast<float>(atlas->getRsc()->size.y);

        float coorsX = inCoorsWidth * 0;
        float coorsY = inCoorsHeight * 0;
        float coorsW = coorsX + inCoorsWidth;
        float coorsH = coorsY + inCoorsHeight;

        float cubeScale = 200.0f;
        
        meshFrame = CR::Gfx::createMesh({
            // POSITION                                                 // TEXTURE COOR
            -frameSize.x*0.5f,    frameSize.y*0.5f,     0,              coorsX,   coorsH,
            -frameSize.x*0.5f,   -frameSize.y*0.5f,     0,              coorsX,   coorsY,
            frameSize.x*0.5f,    -frameSize.y*0.5f,     0,              coorsW,   coorsY,

            frameSize.x*0.5f,    -frameSize.y*0.5f,     0,              coorsW,   coorsY,
            frameSize.x*0.5f,    frameSize.y*0.5f,      0,              coorsW,   coorsH,
            -frameSize.x*0.5f,    frameSize.y*0.5f,     0,              coorsX,   coorsH
            


            // // POSITION                                // TEXTURE
            // -cubeScale,   cubeScale,  -cubeScale,     coorsX,   coorsH,
            // cubeScale,    cubeScale,  -cubeScale,     coorsW,   coorsH,
            // cubeScale,    cubeScale,  cubeScale,      coorsW,   coorsY
            
            // cubeScale,    cubeScale,  cubeScale,      coorsW,   coorsY,
            // -cubeScale,   cubeScale,  cubeScale,      coorsX,   coorsY,
            // -cubeScale,   cubeScale, -cubeScale,      coorsX,   coorsH              
        });

        meshFrame.vertn = 6;

        this->charShader->load("data/shader/b_cube_texture_f.glsl", "data/shader/b_cube_texture_v.glsl");
        this->charShader->findAttrs({"color", "model", "projection", "image", "view"});         

        this->transform = new CR::Gfx::Transform();
        this->transform->shader = charShader;
        this->transform->textures[CR::Gfx::TextureRole::DIFFUSE] = this->atlas->getRsc()->textureId;
        this->transform->shAttrsLoc = charShader->shAttrs;
        
        auto position = CR::MAT4Identity
                        .translate(CR::Vec3<float>(0.0f, -30.0f, 0.0f))
                        .rotate(0, CR::Vec3<float>(1, 1, 1))
                        .scale(CR::Vec3<float>(1.0f));

        this->transform->shAttrsVal = {
            {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
            {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(position)},
            {"view", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
            {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
            {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        };    
        this->transform->fixShaderAttributes({"image", "model", "view", "projection", "color"});
               
    }

    void render(){
        static float add = 0;
        game->renderOn([&](CR::Gfx::RenderLayer *layer){
            // for(unsigned i = 0; i < totalUnits; ++i){
                // layer->add(CR::Gfx::Draw::Mesh(this->tiles[i].source->md, this->tiles[i].transform));
            // }

            add += 45 * CR::getDelta();

            auto position = CR::Vec3<float>(450.0f, 0.0f, 0.0f);

            auto diff = (game->camera.position) - position;

            auto normal = diff.normalize();

            CR::Vec3<float> front(1, 0, 0);

            

            auto theta = CR::Math::acos(normal.dot(front)) + CR::Math::PI * 0.75f;

            // CR::log("%f\n", theta);

            auto mposition = CR::MAT4Identity
                            .translate(position)
                            .rotate(theta, CR::Vec3<float>(0, 1, 0))
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

    std::shared_ptr<CR::Map::Map> map = std::make_shared<CR::Map::Map>(CR::Map::Map()); 

    map->build(CR::Vec2<int>(64, 64), 50);

    Character player;

    player.load();

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

        // map->render();
        player.render();
        
        CR::Gfx::render();
    }

    map->clear();

    CR::Gfx::onEnd();

    return 0;
}