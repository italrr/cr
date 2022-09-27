#define GLFW_INCLUDE_NONE

#include <signal.h>
#include <stdarg.h>
#include <algorithm>
#include <string.h>
#include <memory>
#include <GLFW/glfw3.h>

#include "Graphics.hpp"
#include "3rdparty/glad.h"
#include "Tools.hpp"
#include "Log.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

static GLFWwindow *window = NULL;
static bool running = true;
static std::shared_ptr<CR::Gfx::Settings> settings = std::shared_ptr<CR::Gfx::Settings>(new CR::Gfx::Settings());
static double lastDeltaCheck = 0;
static double currentDelta = 0;
static CR::Vec2<int> size = CR::Vec2<int>(1920, 1080);

// Garbage collection
static std::vector<int> textureList;
static std::mutex textureListMutex;
static std::vector<int> shaderList;
static std::mutex shaderListMutex;
static std::unordered_map<int, std::shared_ptr<CR::Gfx::FramebufferObj>> framebufferList;
static std::mutex framebufferListMutex;


static unsigned lastGenId = CR::Math::random(25, 50);
static std::mutex genIdMutex;

static unsigned genId(){
    std::unique_lock<std::mutex> texLock(genIdMutex);
    int id = ++lastGenId;
    texLock.unlock();
    return id;
}

static std::unordered_map<unsigned, std::shared_ptr<CR::Gfx::RenderLayer>> systemLayers;
static std::unordered_map<unsigned, std::shared_ptr<CR::Gfx::RenderLayer>> userLayers; // TODO: create unload procedure
static std::mutex renderLayerListMutex;
static std::mutex framebufferRenderMutex;
static std::mutex textureRenderMutex;
static std::mutex shaderUseMutex;

static std::vector<std::shared_ptr<CR::Gfx::RenderLayer>> getSortedRenderList(const std::unordered_map<unsigned, std::shared_ptr<CR::Gfx::RenderLayer>> &layers){
    std::vector<std::shared_ptr<CR::Gfx::RenderLayer>> list;

    for(auto &it : layers){
        list.push_back(it.second);
    }

    std::sort(list.begin(), list.end(), [&](std::shared_ptr<CR::Gfx::RenderLayer> &a, std::shared_ptr<CR::Gfx::RenderLayer> &b) {
        return a->order < a->order;
    });       

    return list;
}

static std::shared_ptr<CR::Gfx::Texture> dummyTexture = std::make_shared<CR::Gfx::Texture>(CR::Gfx::Texture());
static std::shared_ptr<CR::Gfx::Shader> shBRect = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
static CR::Gfx::MeshData mBRect;
static CR::Gfx::Transform trans2DTexture;


double CR::getDelta(){
    return currentDelta;
}

void __CR_init_input(GLFWwindow *window);
void __CR_end_input();

void __CR_init_network();
void __CR_end_network();

void __CR_init_job();
void __CR_end_job();
void __CR_update_job();


static std::string getOpenGLError(int v){
	switch(v){
		case 0x0500:
			return "GL_INVALID_ENUM";
		case 0x0501:
			return "GL_INVALID_VALUE";
		case 0x0502:
			return "GL_INVALID_OPERATION";
		case 0x0503:
			return "GL_STACK_OVERFLOW";
		case 0x0504:
			return "GL_STACK_UNDERFLOW";
		case 0x0505:
			return "GL_OUT_OF_MEMORY";
		case 0x0506:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case 0x0507:
			return "GL_CONTEXT_LOST";
		case 0x8031:
			return "GL_TABLE_TOO_LARGE1";
		default:
			return "UNDEFINED";
	}
}

static std::string handleOpenGLFrameBufferError(){
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:
		    return "[FB OK]";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		    return "An attachment could not be bound to frame buffer object!";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		    return "Attachments are missing! At least one image (texture) must be bound to the frame buffer object!";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		    return "A Draw buffer is incomplete or undefinied. All draw buffers must specify attachment points that have images attached.";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		    return "A Read buffer is incomplete or undefinied. All read buffers must specify attachment points that have images attached.";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		    return "All images must have the same number of multisample samples.";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
		    return "If a layered image is attached to one attachment, then all attachments must be layered attachments. The attached layers do not have to have the same number of layers, nor do the layers have to come from the same kind of texture.";
		case GL_FRAMEBUFFER_UNSUPPORTED:
		    return "Attempt to use an unsupported format combinaton!";
		default:
		    return "Unknown error while attempting to create frame buffer object!";
	}
}

static void handleOpenGLError(const std::string &at){
	static GLenum err;
	std::string fbError = handleOpenGLFrameBufferError();
	while ((err = glGetError()) != GL_NO_ERROR){
            auto msg = std::string("OpenGL"+(at.length() > 0 ? " '"+at+"' " : "")+": "+getOpenGLError(err)+" "+fbError);
			CR::log("%s\n", msg.c_str());
	}
}





bool CR::Gfx::RenderLayer::init(unsigned type, int width, int height){

    std::unique_lock<std::mutex> access(this->accesMutex);

    this->size.set(width, height);
    this->id = genId();
    this->fb = CR::Gfx::createFramebuffer(this->size.x, this->size.y);
    this->order = 0;
    this->depth = 0;
    this->objects.clear();
    this->type = type;

    switch(this->type){
        case RenderLayerType::T_3D: {
            this->projection = CR::Math::orthogonal(0, size.x, 0.0f, size.y, -5000.0f, 5000.0f);
            // this->projection = CR::Math::perspective(45.0f, static_cast<float>(this->size.x) /  static_cast<float>(this->size.y), -2500.0f, 2500.0f);
        } break;
        case RenderLayerType::T_2D: {
            this->projection = CR::Math::orthogonal(0, size.x, 0, size.y, -1.0f, 1.0f);
        } break;
    }

    this->objects.reserve(2048);

    access.unlock();

    return true;
}

void CR::Gfx::RenderLayer::add(const std::vector<Renderable*> &objs){
    std::unique_lock<std::mutex> access(this->accesMutex);

    for(int i = 0; i < objs.size(); ++i){
        if(objs[i] == NULL){
            continue;
        }
        this->objects.push_back(objs[i]);
    }

    access.unlock();
}

void CR::Gfx::RenderLayer::add(Renderable* obj){
    if(obj == NULL){
        return;
    }
    std::unique_lock<std::mutex> access(this->accesMutex);

    this->objects.push_back(obj);

    access.unlock();
}

bool CR::Gfx::RenderLayer::init(unsigned type){
    return init(type, CR::Gfx::getWidth(), CR::Gfx::getHeight());    
}

void CR::Gfx::RenderLayer::setDepth(int n){
    std::unique_lock<std::mutex> access(this->accesMutex);
    this->depth = n;   
    access.unlock();
}

void CR::Gfx::RenderLayer::renderOn(const std::function<void(RenderLayer *layer)> &what){
    renderOn(what, false, false);
}

void CR::Gfx::RenderLayer::renderOn(const std::function<void(CR::Gfx::RenderLayer *layer)> &what, bool flush, bool clear){
    std::unique_lock<std::mutex> access(this->accesMutex);
    if(clear){
        this->clear();
    }
    access.unlock();    
    what(this);
    if(flush){
        this->flush();
    }
}

void CR::Gfx::RenderLayer::clear(){
    std::unique_lock<std::mutex> fblock(framebufferRenderMutex);
    if(type == RenderLayerType::T_3D){
        glEnable(GL_DEPTH_TEST);   
    }    
    glBindFramebuffer(GL_FRAMEBUFFER, this->fb->framebufferId);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
    glViewport(0, 0, this->size.x, this->size.y);   
    if(type == RenderLayerType::T_3D){
        glDisable(GL_DEPTH_TEST);
    }else    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fblock.unlock();
}   

void CR::Gfx::RenderLayer::flush(){
    std::unique_lock<std::mutex> fblock(framebufferRenderMutex);
    glBindFramebuffer(GL_FRAMEBUFFER, this->fb->framebufferId);
    if(type == RenderLayerType::T_3D){
        glEnable(GL_DEPTH_TEST);
        
    }else
    if(type == RenderLayerType::T_2D){
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
    // TODO: solve depth for T_2D
    for(int i = 0; i < this->objects.size(); ++i){
        this->objects[i]->render(this->objects[i], this);
    }
    for(int i = 0; i < this->objects.size(); ++i){
        this->objects[i]->~Renderable();
        delete this->objects[i];
    }    
    this->objects.clear();
    if(type == RenderLayerType::T_3D){
        glDisable(GL_DEPTH_TEST);
    }else
    if(type == RenderLayerType::T_2D){
        glDisable(GL_BLEND);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fblock.unlock();
}



static void drawRLImmediate(const std::shared_ptr<CR::Gfx::RenderLayer> &rl, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle){

    const auto projection = CR::Math::orthogonal(0, rl->size.x, rl->size.y, 0, 1.0f, -1.0f); // draw from top to bottom (TODO: remake this if rl->size doesn't match)

    const auto &position = pos;

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    // Apply transformations to model
    static_cast<CR::Gfx::ShaderAttrMat4*>(trans2DTexture.shAttrsValVec[1])->mat = CR::MAT4Identity
                .translate(CR::Vec3<float>(position.x - origin.x * static_cast<float>(size.x), position.y - origin.y * static_cast<float>(size.y), 0.0f))
                .translate(CR::Vec3<float>(origin.x * static_cast<float>(size.x), origin.y * static_cast<float>(size.y), 0.0f))
                .rotate(angle, CR::Vec3<float>(0.0f, 0.0f, 1.0f))
                .translate(CR::Vec3<float>(-origin.x * static_cast<float>(size.x), -origin.y * static_cast<float>(size.y), 0.0f))
                .scale(CR::Vec3<float>(size.x, size.y, 1.0f));

    // Use generic projection
    static_cast<CR::Gfx::ShaderAttrMat4*>(trans2DTexture.shAttrsValVec[2])->mat = projection;

    CR::Gfx::applyShader(trans2DTexture.shader->getRsc()->shaderId, trans2DTexture.shAttrsLocVec, trans2DTexture.shAttrsValVec);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rl->fb->textureId);

    glBindVertexArray(mBRect.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);  
    glUseProgram(0);

    glDisable(GL_BLEND);    
}

CR::Gfx::Renderable *CR::Gfx::Draw::RenderLayer(const std::shared_ptr<CR::Gfx::RenderLayer> &rl, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle){
    CR::Gfx::Renderable2D *self = new CR::Gfx::Renderable2D(); // layer's flush is in charge of deleting this

    self->position = pos;
    self->size = size;
    self->origin = origin;
    self->angle = angle;
    self->origSize = rl->size;
    self->region = CR::Rect<float>(0, 0, rl->size.x, rl->size.y);
    self->type = RenderableType::TEXTURE;
    self->handleId = rl->fb->textureId;

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<Renderable2D*>(renobj);

        auto &position = obj->position;
        auto &size = obj->size;
        auto &scale = obj->scale;
        auto &origin = obj->origin;
        auto &region = obj->region;
        auto &origSize = obj->origSize;
        auto &angle = obj->angle;
        auto &fbId = obj->handleId;

        static_cast<CR::Gfx::ShaderAttrMat4*>(trans2DTexture.shAttrsValVec[1])->mat = CR::MAT4Identity
                .translate(CR::Vec3<float>(position.x - origin.x * static_cast<float>(size.x), position.y - origin.y * static_cast<float>(size.y), 0.0f))
                .translate(CR::Vec3<float>(origin.x * static_cast<float>(size.x), origin.y * static_cast<float>(size.y), 0.0f))
                .rotate(angle, CR::Vec3<float>(0.0f, 0.0f, 1.0f))
                .translate(CR::Vec3<float>(-origin.x * static_cast<float>(size.x), -origin.y * static_cast<float>(size.y), 0.0f))
                .scale(CR::Vec3<float>(size.x, size.y, 1.0f));


        static_cast<CR::Gfx::ShaderAttrMat4*>(trans2DTexture.shAttrsValVec[2])->mat = rl->projection;

        CR::Gfx::applyShader(trans2DTexture.shader->getRsc()->shaderId, trans2DTexture.shAttrsLocVec, trans2DTexture.shAttrsValVec);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbId);

        glBindVertexArray(mBRect.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);         
        glUseProgram(0);   
    };

    return self;
}

CR::Gfx::Renderable *CR::Gfx::Draw::Texture(const std::shared_ptr<CR::Gfx::Texture> &tex, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle){
    CR::Gfx::Renderable2D *self = new CR::Gfx::Renderable2D(); // layer's flush is in charge of deleting this

    self->position = pos;
    self->size = size;
    self->origin = origin;
    self->angle = angle;
    self->origSize = tex->getRsc()->size;
    self->region = CR::Rect<float>(0, 0, size.x, size.y);
    self->type = RenderableType::TEXTURE;
    self->handleId = tex->getRsc()->textureId;

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<Renderable2D*>(renobj);
        
        auto &position = obj->position;
        auto &size = obj->size;
        auto &scale = obj->scale;
        auto &origin = obj->origin;
        auto &region = obj->region;
        auto &origSize = obj->origSize;
        auto &angle = obj->angle;

        static_cast<CR::Gfx::ShaderAttrMat4*>(trans2DTexture.shAttrsValVec[1])->mat = CR::MAT4Identity
                    .translate(CR::Vec3<float>(position.x - origin.x * static_cast<float>(size.x), position.y - origin.y * static_cast<float>(size.y), 0.0f))
                    .translate(CR::Vec3<float>(origin.x * static_cast<float>(size.x), origin.y * static_cast<float>(size.y), 0.0f))
                    .rotate(angle, CR::Vec3<float>(0.0f, 0.0f, 1.0f))
                    .translate(CR::Vec3<float>(-origin.x * static_cast<float>(size.x), -origin.y * static_cast<float>(size.y), 0.0f))
                    .scale(CR::Vec3<float>(size.x, size.y, 1.0f));

        static_cast<CR::Gfx::ShaderAttrMat4*>(trans2DTexture.shAttrsValVec[2])->mat = rl->projection;

        CR::Gfx::applyShader(trans2DTexture.shader->getRsc()->shaderId, trans2DTexture.shAttrsLocVec, trans2DTexture.shAttrsValVec);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->handleId);

        glBindVertexArray(mBRect.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);       
        glUseProgram(0);     
    };

    return self;
}

std::shared_ptr<CR::Gfx::RenderLayer> CR::Gfx::createRenderLayer(const CR::Vec2<int> &size, int type){
    return createRenderLayer(size, type, "", false, -1);
}

std::shared_ptr<CR::Gfx::RenderLayer> CR::Gfx::createRenderLayer(const CR::Vec2<int> &size, int type, const std::string &tag, bool systemLayer, int order){
    auto rl = std::shared_ptr<CR::Gfx::RenderLayer>(new CR::Gfx::RenderLayer());
    
    rl->init(type, size.x, size.y);
    rl->type = type;
    rl->tag = tag;
    rl->order = order == -1 ? 0 : order;

    if(systemLayer){
        rl->order = order == -1 ?  : order;   
        systemLayers[rl->id] = rl;
    }else{
        userLayers[rl->id] = rl;
    }

    return rl;
}

std::shared_ptr<CR::Gfx::RenderLayer> CR::Gfx::getRenderLayer(int id, bool isSystemLayer){
    std::unique_lock<std::mutex> rllock(renderLayerListMutex);
    auto &list = isSystemLayer ? systemLayers : userLayers;
    auto it = list.find(id);
    if(it == list.end()){
        rllock.unlock();
        return std::shared_ptr<CR::Gfx::RenderLayer>(NULL);
    }
    auto rl = it->second;
    rllock.unlock();
    return rl;
}

std::shared_ptr<CR::Gfx::RenderLayer> CR::Gfx::getRenderLayer(const std::string &tag, bool isSystemLayer){
    std::unique_lock<std::mutex> rllock(renderLayerListMutex);
    auto &list = isSystemLayer ? systemLayers : userLayers;

    for(auto &it : list){
        if(it.second->tag == tag){
            auto rl = it.second;
            rllock.unlock();
            return rl;
        }
    }
    rllock.unlock();
    return std::shared_ptr<CR::Gfx::RenderLayer>(NULL);
}

void CR::Gfx::Settings::setParams(const std::vector<std::string> &params){
    for(int i = 0; i < params.size(); ++i){
        if((params[i] == "-w" || params[i] == "--width") && i < params.size()-1){
            this->width = std::stoi(params[i + 1]);
        }
        if((params[i] == "-h" || params[i] == "--height") && i < params.size()-1){
            this->height = std::stoi(params[i + 1]);
        }
        if((params[i] == "-r" || params[i] == "--resizable-window") && i < params.size()-1){
            this->resizable = std::stoi(params[i + 1]);
        } 
        if((params[i] == "-f" || params[i] == "--fullscreen") && i < params.size()-1){
            this->fullscreen = std::stoi(params[i + 1]);
        }                    
    }
}

void CR::Gfx::Settings::readSettings(const std::string &path){
    Jzon::Parser parser;

    auto fpath = CR::File::fixPath(path);

    auto obj = parser.parseFile(fpath);

    if(!CR::File::exists(fpath)){
        CR::log("readSettings: Failed to parse %s: Doesn't exist\n", fpath.c_str());
        return;
    }

    if(!obj.isValid()){
        CR::log("readSettings: Failed to parse %s: It's invalid\n", fpath.c_str());
        return;
    }

    this->width = obj.get("width").toInt(1920);
    this->height = obj.get("height").toInt(1080);
    this->resizable = obj.get("resizable").toBool(false);
    this->fullscreen = obj.get("fullscreen").toBool(false);
}

void CR::Gfx::loadSettings(const std::vector<std::string> &params, const std::string &path){
    settings->readSettings(path);
    settings->setParams(params);
}

static void ctrlC(int s){
    CR::log("Caught Ctrl+C: Requested exit\n");
	CR::Gfx::end();
}

bool CR::Gfx::init(){

    std::string wst = settings->fullscreen ? "Fullscreen" : "Windowed";
    auto platformName = Core::SupportedPlatform::name(Core::PLATFORM).c_str();
    auto archName = Core::SupportedArchitecture::name(Core::ARCH).c_str();
    
    size.x = settings->width;
    size.y = settings->height;
    CR::log("[GFX] CAVERN RUSH | res: %dx%d | OS: %s | ARCH: %s | Mode: %s \n", settings->width, settings->height, platformName, archName, wst.c_str());


    if (!glfwInit()){
        CR::log("[GFX] failed to start 'GLFW'");
        CR::Core::exit(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, settings->resizable ? GL_TRUE : GL_FALSE);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(settings->width, settings->height, "C A V E R N   R U S H", NULL, NULL);

    if(!window){
        CR::log("[GFX] Failed to open window\n");
        CR::Core::exit(1);
    }

    glfwMakeContextCurrent(window);
    
    gladLoadGL();


    CR::log("[GFX] GPU OpenGL version: %s\n", glGetString(GL_VERSION));
    
    signal(SIGINT, ctrlC);
    glfwSwapInterval(1);
    __CR_init_input(window);
    __CR_init_job();
    __CR_init_network();

    // basic rectangle for 2d rendering
    shBRect->load("data/shader/b_rect_texture_f.glsl", "data/shader/b_rect_texture_v.glsl");
    shBRect->findAttrs({"color", "model", "projection", "image"});

    // Global transform for 2D textures
    trans2DTexture.shader = shBRect;
    trans2DTexture.shAttrsLoc = shBRect->shAttrs;
    trans2DTexture.shAttrsVal = {
        {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
        {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
        {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(MAT4Identity)},
        {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
    };    
    trans2DTexture.fixShaderAttributes({"image", "model", "projection", "color"});

    mBRect = createMesh({ 
        // pos
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    },
    {
        // tex
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f, 
    
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f        
    });


    dummyTexture->load("data/texture/container.png");

    return true;
}


static float add = 0.0f;

CR::Gfx::Renderable *CR::Gfx::Draw::Mesh(CR::Gfx::MeshData &md, CR::Gfx::Transform *transform){
    CR::Gfx::Renderable3D *self = new CR::Gfx::Renderable3D(); // layer's flush is in charge of deleting this

    self->transform = transform;
    self->md = md;

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<CR::Gfx::Renderable3D*>(renobj);

        static_cast<CR::Gfx::ShaderAttrMat4*>(obj->transform->shAttrsValVec[2])->mat = rl->camera.getView();
        static_cast<CR::Gfx::ShaderAttrMat4*>(obj->transform->shAttrsValVec[3])->mat = rl->projection;

        CR::Gfx::applyShader(obj->transform->shader->getRsc()->shaderId, obj->transform->shAttrsLocVec, obj->transform->shAttrsValVec);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->transform->textures[CR::Gfx::TextureRole::DIFFUSE]);

        glBindVertexArray(obj->md.vao);
        glDrawArrays(GL_TRIANGLES, 0, obj->md.vertn);
        glBindVertexArray(0);       
        glUseProgram(0);      
    };

    return self;
}

// gotta go fasto optimization
CR::Gfx::Renderable *CR::Gfx::Draw::MeshBatch(std::vector<CR::Gfx::MeshData*> &md, std::vector<CR::Gfx::Transform*> &transform, bool shareTexture, bool shareShader, bool shareModelTrans, unsigned modelPos){
    auto *self = new CR::Gfx::Renderable3DBatch(); // layer's flush is in charge of deleting this

    self->md = &md;
    self->transform = &transform;

    self->shareShader = shareShader;
    self->shareShader = shareShader;
    self->shareModelTrans = shareModelTrans;
    self->modelPos = modelPos;

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<CR::Gfx::Renderable3DBatch*>(renobj);
  
        auto camCent = rl->camera.getCenter();
        bool shFirst = false;
        bool texFirst = false;

        CR::Vec3<unsigned> view = CR::Vec3<unsigned>(rl->size.x, rl->size.x  * 2, rl->size.y * 1.5f);

        glActiveTexture(GL_TEXTURE0);
        for(unsigned i = 0; i < obj->md->size(); ++i){

            auto &mp = obj->transform->at(i)->position;
            
            if( CR::Math::abs(camCent.x-mp.x) > view.x ||
                CR::Math::abs(camCent.y-mp.y) > view.y ||
                CR::Math::abs(camCent.z-mp.z) > view.z){
                continue;
            }

            if(!obj->shareShader || !shFirst){
                static_cast<CR::Gfx::ShaderAttrMat4*>(obj->transform->at(i)->shAttrsValVec[2])->mat = rl->camera.getView();
                static_cast<CR::Gfx::ShaderAttrMat4*>(obj->transform->at(i)->shAttrsValVec[3])->mat = rl->projection;
                CR::Gfx::applyShader(obj->transform->at(i)->shader->getRsc()->shaderId, obj->transform->at(i)->shAttrsLocVec, obj->transform->at(i)->shAttrsValVec);     
                shFirst = true;             
            }else
            if(obj->shareShader && obj->shareModelTrans){
                CR::Gfx::applyShaderPartial(obj->transform->at(i)->shAttrsLocVec[obj->modelPos], obj->transform->at(i)->shAttrsValVec[obj->modelPos]);
            }            
            if(!obj->shareTexture || !texFirst){
                glBindTexture(GL_TEXTURE_2D, obj->transform->at(i)->textures[CR::Gfx::TextureRole::DIFFUSE]);
                texFirst = true;
            }

            glBindVertexArray(obj->md->at(i)->vao);
            glDrawArrays(GL_TRIANGLES, 0, obj->md->at(i)->vertn);            

        }

        glBindVertexArray(0);       
        glUseProgram(0);     
    };


    return self;
}

void CR::Gfx::render(){
    if(!running){
        return;
    }

    if(glfwWindowShouldClose(window)){
        end();
        return;
    }

    // time
    auto currentTime = glfwGetTime();
    currentDelta = (currentTime - lastDeltaCheck);
    lastDeltaCheck = currentTime;


    // dummyLayer = CR::Gfx::createRenderLayer(CR::Vec2<int>(dummyTexture->getRsc()->size), CR::Gfx::RenderLayerType::T_2D);
    // dummyLayer->renderOn([](CR::Gfx::RenderLayer *lyr){
    //     lyr->add(Draw::Texture(dummyTexture, CR::Vec2<float>(), dummyTexture->getRsc()->size, CR::Vec2<float>(0.0f), CR::Math::rads(0)));
    // });
    // dummyLayer->clear();
    // dummyLayer->flush();



    static std::shared_ptr<CR::Gfx::RenderLayer> uiL = CR::Gfx::getRenderLayer("ui");
    static std::shared_ptr<CR::Gfx::RenderLayer> wL = CR::Gfx::getRenderLayer("world");



    // uiL->renderOn([](CR::Gfx::RenderLayer *layer){    
    //     // layer->add(Draw::Texture(dummyTexture, CR::Vec2<float>(0), dummyTexture->size, CR::Vec2<float>(0.5f), CR::Math::rads(0)));
    //     layer->add(CR::Gfx::Draw::RenderLayer(dummyLayer, CR::Vec2<float>(layer->size.x - dummyLayer->size.x,layer->size.y - dummyLayer->size.y), CR::Vec2<int>(dummyLayer->size), CR::Vec2<float>(0.0f), 0.0f));
    // });

    
    // Flush system layers
    std::vector<std::shared_ptr<CR::Gfx::RenderLayer>> layerList;
    for(auto &it : systemLayers){
        auto &layer = it.second;
        layer->clear();
        layer->flush();           
        layerList.push_back(layer);
    }
    
    // Sort layers to be rendered in order
    std::sort(layerList.begin(), layerList.end(), [&](std::shared_ptr<CR::Gfx::RenderLayer> &a, std::shared_ptr<CR::Gfx::RenderLayer> &b) {
        return a->order < b->order;
    });         

    // Render onto screen
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, size.x, size.y);  
    for(int i = 0; i < layerList.size(); ++i){
        auto &layer = layerList[i];
        drawRLImmediate(layer, Vec2<float>(0), layer->size, Vec2<float>(0.0f), CR::Math::rads(0));

    }   

    glfwSwapBuffers(window);

    __CR_update_job();
    glfwPollEvents();
}

void CR::Gfx::end(){
    running = false;
    CR::log("Exiting...\n");
    glfwDestroyWindow(window);
    glfwTerminate();
}

void CR::Gfx::onEnd(){
    running = false;
    __CR_end_input();
    __CR_end_network();
    __CR_end_job();    
    CR::log("Bye!\n");
}

bool CR::Gfx::isRunning(){
    return running;
}

int CR::Gfx::getWidth(){
    return size.x;
}

int CR::Gfx::getHeight(){
    return size.y;
}

CR::Vec2<int> CR::Gfx::getSize(){
    return size;
}

unsigned CR::Gfx::createTexture2D(unsigned char *data, unsigned w, unsigned h, unsigned format){
    unsigned texture;
    std::unique_lock<std::mutex> texLock(textureRenderMutex);
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum glformat;
    switch(format){
        case ImageFormat::RED: {
            glformat = GL_RED;
        } break;
        case ImageFormat::GREEN: {
            glformat = GL_GREEN;
        } break;        
        case ImageFormat::BLUE: {
            glformat = GL_BLUE;
        } break;        
        case ImageFormat::RGB: {
            glformat = GL_RGB;
        } break;
        case ImageFormat::RGBA: {
            glformat = GL_RGBA;
        } break;                
    }
    glTexImage2D(GL_TEXTURE_2D, 0, glformat, w, h, 0, glformat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    texLock.unlock();
    std::unique_lock<std::mutex> lock(textureListMutex);
    textureList.push_back(texture);
    lock.unlock();
    // TODO: check OpenGL errors
    return texture;
}

bool CR::Gfx::deleteTexture2D(unsigned id){
    std::unique_lock<std::mutex> lock(textureListMutex);
    for(int i = 0; i < textureList.size(); ++i){
        if(textureList[i] == id){
            std::unique_lock<std::mutex> texLock(textureRenderMutex);
            glDeleteTextures(1, &id);
            texLock.unlock();
            textureList.erase(textureList.begin() + i);
            return true;
        }
    }
    lock.unlock();    
    return false;
}

std::shared_ptr<CR::Gfx::FramebufferObj> CR::Gfx::createFramebuffer(unsigned w, unsigned h){
    unsigned id;
    std::unique_lock<std::mutex> fblock(framebufferRenderMutex);

    unsigned renderbufferId;
    glGenRenderbuffers(1, &renderbufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    unsigned texture = CR::Gfx::createTexture2D(0, w, h, ImageFormat::RGBA);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbufferId);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    fblock.unlock();

    std::unique_lock<std::mutex> lock(framebufferListMutex);
    auto handle = std::shared_ptr<FramebufferObj>(new FramebufferObj(id, texture, renderbufferId, w, h));
    framebufferList[id] = handle;
    lock.unlock();    
    
    // TODO: check OpenGL errors
    return handle;
}

bool CR::Gfx::deleteFramebuffer(unsigned id){
    std::unique_lock<std::mutex> lock(framebufferListMutex);

    auto it = framebufferList.find(id);

    if(it == framebufferList.end()){
        return false;
    }
    
    std::unique_lock<std::mutex> fblock(framebufferRenderMutex);
    glDeleteFramebuffers(1, &it->second->framebufferId);
    fblock.unlock();

    deleteTexture2D(it->second->textureId);
    
    framebufferList.erase(it);

    lock.unlock();    
    return true;    
}

bool CR::Gfx::updateMesh(CR::Gfx::MeshData &md, unsigned vrole, const std::vector<float> &vertex){

    if(vrole != VertexRole::POSITION && vrole != VertexRole::TEXCOORD){
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, md.vbo[vrole]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex.size() * sizeof(float), &vertex[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}


CR::Gfx::MeshData CR::Gfx::createMesh(const std::vector<float> &pos, const std::vector<float> &tex){
    return CR::Gfx::createMesh(pos, tex, VertexStoreType::STATIC, VertexStoreType::STATIC);
}


CR::Gfx::MeshData CR::Gfx::createMesh(const std::vector<float> &pos, const std::vector<float> &tex, unsigned vPosStType, unsigned vTexStType){

    unsigned vbo[2], vao;

    glGenVertexArrays(1, &vao);
    glGenBuffers(2, vbo);
    
    // GENERATE POSITION BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VertexRole::POSITION]);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), &pos[0], vPosStType == VertexStoreType::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    // GENERATE TEXTURE COORDINATES
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VertexRole::TEXCOORD]);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), &tex[0], vTexStType == VertexStoreType::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);


    glBindVertexArray(vao);

    // BIND POSITION TO VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VertexRole::POSITION]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // BIND TEXCOORDS TO VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VertexRole::TEXCOORD]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);    
    

    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);


    CR::Gfx::MeshData md;
    md.vao = vao;
    // memcpy(md.vbo, vbo, sizeof(vbo));
    md.vbo[0] = vbo[0];
    md.vbo[1] = vbo[1];
    md.vertn = static_cast<float>(pos.size()) / 3.0f;
    return md;
}

unsigned CR::Gfx::createShader(const std::string &vert, const std::string &frag){
    unsigned shaderId;
    std::string str;
    const char *fragSrc = frag.c_str();
    const char *vertSrc = vert.c_str();
    // create shaders
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLint gResult = GL_FALSE;
    int logLength = 1024 * 5;
    char *buffer = new char[logLength];
    memset(buffer, 0, logLength);
    // compile vertex
    std::unique_lock<std::mutex> shaderlk(shaderUseMutex);
    glShaderSource(vertShader, 1, &vertSrc, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &gResult);
    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
    memset(buffer, 0, logLength);
    glGetShaderInfoLog(vertShader, logLength, NULL, buffer);
    str = std::string(buffer);
    if(str.length() > 0) {
        CR::log("Shader compilation error at vert: \n\n%s\n\n", str.c_str());
        return 0;
    }
    str = "";
    // compile frag
    glShaderSource(fragShader, 1, &fragSrc, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &gResult);
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
    memset(buffer, 0, logLength);
    glGetShaderInfoLog(fragShader, logLength, NULL, buffer);
    str = std::string(buffer);
    if(str.length() > 0) {
        CR::log("Shader compilation error at frag: \n\n%s\n\n", str.c_str());
        return 0;
    }
    str = "";
    // put together
    shaderId = glCreateProgram();
    glAttachShader(shaderId, vertShader);
    glAttachShader(shaderId, fragShader);
    glLinkProgram(shaderId);
    // check status
    glGetProgramiv(shaderId, GL_LINK_STATUS, &gResult);
    glGetProgramiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    memset(buffer, 0, logLength);
    glGetProgramInfoLog(shaderId, logLength, NULL, buffer);
    str = std::string(buffer);
    if(str.length() > 0) {
        CR::log("Shader compilation error at LINK: \n\n%s\n\n", str.c_str());
        return 0;
    }
    str = "";
    // clean
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    shaderlk.unlock();
    delete buffer;
    std::unique_lock<std::mutex> lock(shaderListMutex);
    shaderList.push_back(shaderId);
    lock.unlock();
    return shaderId;
}

bool CR::Gfx::deleteShader(unsigned id){
    std::unique_lock<std::mutex> lock(shaderListMutex);
    for(int i = 0; i < shaderList.size(); ++i){
        if(shaderList[i] == id){
            std::unique_lock<std::mutex> shaderlk(shaderUseMutex);
            glDeleteShader(id);
            shaderlk.unlock();
            shaderList.erase(shaderList.begin() + i);
            return true;
        }
    }
    lock.unlock();    
    return false;    
}

int CR::Gfx::findShaderAttr(unsigned shaderId, const std::string &name){
    int locId = 0;
    std::unique_lock<std::mutex> lock(shaderUseMutex);
    glUseProgram(shaderId);
    locId = glGetUniformLocation(shaderId, name.c_str());
    glUseProgram(0);
    lock.unlock();
    if(locId == -1){
        CR::log("CR::Gfx::findShaderAttr: Failed to find attribute '%s'\n", name.c_str());
    }
    return locId;
}
#include <stdio.h>

void CR::Gfx::applyShaderPartial(unsigned loc, CR::Gfx::ShaderAttr* attrv){
    switch(attrv->type){
        case CR::Gfx::ShaderAttrType::FLOAT: {
            CR::Gfx::ShaderAttrFloat *attr = static_cast<CR::Gfx::ShaderAttrFloat*>(attrv);
            glUniform1f(loc, attr->n);
        } break;
        case CR::Gfx::ShaderAttrType::INT: {
            CR::Gfx::ShaderAttrInt *attr = static_cast<CR::Gfx::ShaderAttrInt*>(attrv);
            glUniform1f(loc, attr->n);
        } break;                 
        case CR::Gfx::ShaderAttrType::COLOR: {
            CR::Gfx::ShaderAttrColor *attr = static_cast<CR::Gfx::ShaderAttrColor*>(attrv);
            glUniform3fv(loc, 1, attr->color);
        } break;
        case CR::Gfx::ShaderAttrType::VEC2: {
            CR::Gfx::ShaderAttrVec2 *attr = static_cast<CR::Gfx::ShaderAttrVec2*>(attrv);
            glUniform2fv(loc, 1, attr->vec);
        } break;     
        case CR::Gfx::ShaderAttrType::VEC3: {
            CR::Gfx::ShaderAttrVec3 *attr = static_cast<CR::Gfx::ShaderAttrVec3*>(attrv);
            glUniform3fv(loc, 1, attr->vec);
        } break; 
        case CR::Gfx::ShaderAttrType::MAT4: {
            CR::Gfx::ShaderAttrMat4 *attr = static_cast<CR::Gfx::ShaderAttrMat4*>(attrv);
            glUniformMatrix4fv(loc, 1, GL_FALSE, attr->mat.mat);          
        } break;                                                           
        default: {
            CR::log("[GFX] undefined shader type to apply '%s'\n", attrv->type);
        } break;
    }        
}


void CR::Gfx::applyShader(unsigned shaderId, const std::vector<unsigned> &loc, const std::vector<CR::Gfx::ShaderAttr*> &attributes){
    glUseProgram(shaderId);
    for(unsigned i = 0; i < loc.size(); ++i){
        switch(attributes[i]->type){
            case CR::Gfx::ShaderAttrType::FLOAT: {
                auto attr = static_cast<CR::Gfx::ShaderAttrFloat*>(attributes[i]);
                glUniform1f(loc[i], attr->n);
            } break;
            case CR::Gfx::ShaderAttrType::INT: {
                auto attr = static_cast<CR::Gfx::ShaderAttrInt*>(attributes[i]);
                glUniform1f(loc[i], attr->n);
            } break;                 
            case CR::Gfx::ShaderAttrType::COLOR: {
                auto attr = static_cast<CR::Gfx::ShaderAttrColor*>(attributes[i]);
                glUniform3fv(loc[i], 1, attr->color);
            } break;
            case CR::Gfx::ShaderAttrType::VEC2: {
                auto attr = static_cast<CR::Gfx::ShaderAttrVec2*>(attributes[i]);
                glUniform2fv(loc[i], 1, attr->vec);
            } break;     
            case CR::Gfx::ShaderAttrType::VEC3: {
                auto attr = static_cast<CR::Gfx::ShaderAttrVec3*>(attributes[i]);
                glUniform3fv(loc[i], 1, attr->vec);
            } break; 
            case CR::Gfx::ShaderAttrType::MAT4: {
                auto attr = static_cast<CR::Gfx::ShaderAttrMat4*>(attributes[i]);
                glUniformMatrix4fv(loc[i], 1, GL_FALSE, attr->mat.mat);                        
            } break;                                                           
            default: {
                CR::log("[GFX] undefined shader type to apply '%s'\n", attributes[i]->type);
                return;
            } break;
        }        
    }
}

CR::Gfx::Transform::Transform(){
    this->textures = {0, 0, 0, 0};
    this->position.set(0.0f);
    this->rotation.set(0.0f);
    this->scale.set(0.0f);
    this->shader = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
}

void CR::Gfx::Transform::fixShaderAttributes(const std::vector<std::string> &locOrder){
    shAttrsLocVec.clear();
    shAttrsValVec.clear();
    for(unsigned i = 0; i < locOrder.size(); ++i){
        auto &attr = shAttrsVal[locOrder[i]];
        auto &loc = shAttrsLoc[locOrder[i]];
        shAttrsLocVec.push_back(loc);
        shAttrsValVec.push_back(attr.get());
    }
}

CR::Gfx::Camera::Camera(){
    this->position.set(0.0f, 0.0f, 0.0f);
    this->up.set(0.0f, 0.0f, 0.0f);
    this->target.set(0.0f, 0.0f, 0.0f);
}


void CR::Gfx::Camera::setPosition(const CR::Vec3<float> &pos){
    this->position = pos;    
}

void CR::Gfx::Camera::update(){

}

void CR::Gfx::Camera::setUp(const CR::Vec3<float> &up){
    this->up = up;
}

void CR::Gfx::Camera::setTarget(const CR::Vec3<float> &target){
    this->target = target;
}

CR::Vec3<float> CR::Gfx::Camera::getCenter(){
    return this->position + CR::Vec3<float>(CR::Gfx::getWidth(), CR::Gfx::getHeight(), -CR::Gfx::getHeight()) * CR::Vec3<float>(0.5f);
}

CR::Mat<4, 4, float> CR::Gfx::Camera::getView(){
    
    return Math::lookAt(this->position, this->position + this->targetBias, CR::Vec3<float>(0.0f, 1.0f, 0.0f));

    // return Math::lookAt(this->position, this->position + this->targetBias, CR::Vec3<float>(0.0f, 1.0f, 0.0f));
    // return Math::lookAt(this->position, CR::Vec3<float>(0.0f, 0.0f, 0.0f), CR::Vec3<float>(0.0f, 1.0f, 0.0f));
}