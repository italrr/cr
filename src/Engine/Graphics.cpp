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
#include "Model.hpp"

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
static std::shared_ptr<CR::Gfx::Shader> shBCube = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
static CR::Gfx::MeshData mBRect;
static CR::Gfx::MeshData mBCube;
static std::shared_ptr<CR::Gfx::Mesh> mBCubeMesh = std::shared_ptr<CR::Gfx::Mesh>(new CR::Gfx::Mesh());


double CR::getDelta(){
    return currentDelta;
}

void __CR_init_input(GLFWwindow *window);
void __CR_end_input();

void __CR_init_job();
void __CR_end_job();
void __CR_update_job();

CR::Gfx::Vertex::Vertex(){
    memset(this->id, 0, 4 * sizeof(id[0]));
    memset(this->weight, 0, 4 * sizeof(weight[0]));   
}



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
            this->projection = CR::Math::perspective(45.0f, static_cast<float>(this->size.x) /  static_cast<float>(this->size.y), 0.1f, 100.0f);
            this->camera.init();
        } break;
        case RenderLayerType::T_2D: {
            this->projection = CR::Math::orthogonal(0, size.x, 0, size.y);
        } break;
    }

    this->objects.reserve(2000);

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

    const auto projection = CR::Math::orthogonal(0, rl->size.x, rl->size.y, 0); // draw from top to bottom

    // std::unique_lock<std::mutex> lock(rl->accesMutex);

    const auto &position = pos;


    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // CR::log("%s %i\n", size.str().c_str(), rl->fb->textureId);
    auto model = CR::MAT4Identity
                .translate(CR::Vec3<float>(position.x - origin.x * static_cast<float>(size.x), position.y - origin.y * static_cast<float>(size.y), 0.0f))
                .translate(CR::Vec3<float>(origin.x * static_cast<float>(size.x), origin.y * static_cast<float>(size.y), 0.0f))
                .rotate(angle, CR::Vec3<float>(0.0f, 0.0f, 1.0f))
                .translate(CR::Vec3<float>(-origin.x * static_cast<float>(size.x), -origin.y * static_cast<float>(size.y), 0.0f))
                .scale(CR::Vec3<float>(size.x, size.y, 1.0f));

    CR::Gfx::applyShader(shBRect->getRsc()->shaderId, shBRect->shAttrs, {
        {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
        {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(model)},
        {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(projection)},
        {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
    });

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rl->fb->textureId);

    glBindVertexArray(mBRect.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);  
    glUseProgram(0);

    glDisable(GL_BLEND);
    
    // lock.unlock();
}

CR::Gfx::Renderable *CR::Gfx::Draw::RenderLayer(const std::shared_ptr<CR::Gfx::RenderLayer> &rl, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle){
    CR::Gfx::Renderable2D *self = new CR::Gfx::Renderable2D(); // layer's flush is in charge of deleting this

    std::unique_lock<std::mutex> lock(rl->accesMutex);

    self->position = pos;
    self->size = size;
    self->origin = origin;
    self->angle = angle;
    self->origSize = rl->size;
    self->region = CR::Rect<float>(0, 0, rl->size.x, rl->size.y);
    self->type = RenderableType::TEXTURE;
    self->handleId = rl->fb->textureId;

    lock.unlock();

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<Renderable2D*>(renobj);

        const auto projection = CR::Math::orthogonal(0, rl->size.x, rl->size.y, 0);

        auto &position = obj->position;
        auto &size = obj->size;
        auto &scale = obj->scale;
        auto &origin = obj->origin;
        auto &region = obj->region;
        auto &origSize = obj->origSize;
        auto &angle = obj->angle;
        auto &fbId = obj->handleId;

        auto model = CR::MAT4Identity
                    .translate(CR::Vec3<float>(position.x - origin.x * static_cast<float>(size.x), position.y - origin.y * static_cast<float>(size.y), 0.0f))
                    .translate(CR::Vec3<float>(origin.x * static_cast<float>(size.x), origin.y * static_cast<float>(size.y), 0.0f))
                    .rotate(angle, CR::Vec3<float>(0.0f, 0.0f, 1.0f))
                    .translate(CR::Vec3<float>(-origin.x * static_cast<float>(size.x), -origin.y * static_cast<float>(size.y), 0.0f))
                    .scale(CR::Vec3<float>(size.x, size.y, 1.0f));

        applyShader(shBRect->getRsc()->shaderId, shBRect->shAttrs, {
            {"image", std::make_shared<ShaderAttrInt>(0)},
            {"model", std::make_shared<ShaderAttrMat4>(model)},
            {"projection", std::make_shared<ShaderAttrMat4>(rl->projection)},
            {"color", std::make_shared<ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        });

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
        
        // const auto projection = CR::Math::orthogonal(0, rl->size.x, rl->size.y, 0);


        auto &position = obj->position;
        auto &size = obj->size;
        auto &scale = obj->scale;
        auto &origin = obj->origin;
        auto &region = obj->region;
        auto &origSize = obj->origSize;
        auto &angle = obj->angle;

        auto model = CR::MAT4Identity
                    .translate(CR::Vec3<float>(position.x - origin.x * static_cast<float>(size.x), position.y - origin.y * static_cast<float>(size.y), 0.0f))
                    .translate(CR::Vec3<float>(origin.x * static_cast<float>(size.x), origin.y * static_cast<float>(size.y), 0.0f))
                    .rotate(angle, CR::Vec3<float>(0.0f, 0.0f, 1.0f))
                    .translate(CR::Vec3<float>(-origin.x * static_cast<float>(size.x), -origin.y * static_cast<float>(size.y), 0.0f))
                    .scale(CR::Vec3<float>(size.x, size.y, 1.0f));

        applyShader(shBRect->getRsc()->shaderId, shBRect->shAttrs, {
            {"image", std::make_shared<ShaderAttrInt>(0)},
            {"model", std::make_shared<ShaderAttrMat4>(model)},
            {"projection", std::make_shared<ShaderAttrMat4>(rl->projection)},
            {"color", std::make_shared<ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        });

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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

    __CR_init_input(window);
    __CR_init_job();

    // basic rectangle for 2d rendering
    shBRect->load("data/shader/b_rect_texture_f.glsl", "data/shader/b_rect_texture_v.glsl");
    shBCube->load("data/shader/b_cube_texture_f.glsl", "data/shader/b_cube_texture_v.glsl");
    
    shBRect->findAttrs({"color", "model", "projection", "image"});
    shBCube->findAttrs({"color", "model", "projection", "image", "view"});

    mBRect = createPrimMesh({ 
        // pos              // tex
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 
    
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f
    });

    mBCube = createPrimMesh({ 
        // pos                  // tex
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f
    });
    mBCube.vertn = 36;

    dummyTexture->load("data/texture/container.png");

    return true;
}


static float add = 0.0f;

CR::Gfx::Renderable *CR::Gfx::Draw::PrimMesh(CR::Gfx::MeshData &md, unsigned nverts, unsigned textureId, const CR::Vec3<float> &position, const CR::Vec3<float> &scale, const CR::Vec4<float> &rotation){
    CR::Gfx::Renderable3D *self = new CR::Gfx::Renderable3D(); // layer's flush is in charge of deleting this

    self->transform = std::make_shared<CR::Gfx::Transform>(CR::Gfx::Transform());
    self->transform->model = CR::MAT4Identity.translate(position).rotate(rotation.w, CR::Vec3<float>(rotation.x, rotation.y, rotation.z)).scale(scale);
    self->transform->textures[CR::Gfx::TextureRole::DIFFUSE] = textureId;
    self->md = md;

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<CR::Gfx::Renderable3D*>(renobj);
        
        CR::Gfx::applyShader(shBCube->getRsc()->shaderId, shBCube->shAttrs, {
            {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
            {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(obj->transform->model)},
            {"view", std::make_shared<CR::Gfx::ShaderAttrMat4>(rl->camera.getView())},
            {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(rl->projection)},
            {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        });

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->transform->textures[CR::Gfx::TextureRole::DIFFUSE]);

        glBindVertexArray(obj->md.vao);
        glDrawArrays(GL_TRIANGLES, 0, obj->md.vertn);
        glBindVertexArray(0);       
        glUseProgram(0);      
    };

    return self;

}

CR::Gfx::Renderable *CR::Gfx::Draw::Mesh(CR::Gfx::MeshData &md, const std::shared_ptr<CR::Gfx::Transform> &transform, const std::shared_ptr<CR::Gfx::Shader> &shader){
    CR::Gfx::Renderable3D *self = new CR::Gfx::Renderable3D(); // layer's flush is in charge of deleting this

    self->transform = transform;
    self->shader = shader;
    self->md = md;

    self->render = [](CR::Gfx::Renderable *renobj, CR::Gfx::RenderLayer *rl){
        auto *obj = static_cast<CR::Gfx::Renderable3D*>(renobj);
        
        //
        // Setup shader attributes
        //


        // values
        std::unordered_map<std::string, std::shared_ptr<CR::Gfx::ShaderAttr>> fShAttrs;
        fShAttrs.insert(obj->transform->shAttrsVal.begin(), obj->transform->shAttrsVal.end());
        fShAttrs["model"] = std::make_shared<CR::Gfx::ShaderAttrMat4>(obj->transform->model);
        fShAttrs["view"] = std::make_shared<CR::Gfx::ShaderAttrMat4>(rl->camera.getView());
        fShAttrs["projection"] = std::make_shared<CR::Gfx::ShaderAttrMat4>(rl->projection);
        
        // locations
        // std::unordered_map<std::string, unsigned> fShAttrsLoc;
        // fShAttrsLoc.insert(obj->transform->shAttrsLoc.begin(), obj->transform->shAttrsLoc.end());
        // fShAttrsLoc.insert(obj->shader->shAttrs.begin(), obj->shader->shAttrs.begin());

        // TODO: attend lighting

        // Apply shader
        CR::Gfx::applyShader(obj->shader->getRsc()->shaderId, obj->shader->shAttrs, fShAttrs);


        // Bind DIFFUSE for now
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dummyTexture->getRsc()->textureId);

        // Draw Triangles
        glBindVertexArray(obj->md.vao);
        glDrawArrays(GL_TRIANGLES, 0, obj->md.vertn);

        // Clean up
        // TODO: optimize program for the same shader
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



    static bool yes = false;
    static float add = 0.0f;
    static std::shared_ptr<RenderLayer> dummyLayer;
    static std::shared_ptr<CR::Gfx::Model> model = std::make_shared<CR::Gfx::Model>(CR::Gfx::Model());

    static std::shared_ptr<CR::Gfx::Transform> transform = std::make_shared<CR::Gfx::Transform>(CR::Gfx::Transform());
    static std::shared_ptr<CR::Gfx::Shader> shader = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());


    add += 360.0f * currentDelta;

    if(!yes){
        yes = true;
        dummyLayer = CR::Gfx::createRenderLayer(CR::Vec2<int>(1000, 1000), CR::Gfx::RenderLayerType::T_2D);

        model->load("data/model/nathan.fbx");
        transform->textures[CR::Gfx::TextureRole::DIFFUSE] = model->getRsc()->texDeps.begin()->texture->getRsc()->textureId;
        transform->shAttrsVal["material.diffuse"] = std::make_shared<CR::Gfx::ShaderAttrInt>(0);
        transform->shAttrsVal["material.color"] = std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f));
        transform->model = CR::MAT4Identity.translate(CR::Vec3<float>(0.0f)).rotate(CR::Math::rads(add), CR::Vec3<float>(0.5f, 1.0f, 0.0f)).scale(CR::Vec3<float>(1.0f));

        shader->load("data/shader/b_generic_simple_light_f.glsl", "data/shader/b_generic_simple_light_v.glsl");
        shader->findAttrs({"model", "projection", "view", "material.diffuse", "material.color"});
    }

    dummyLayer->renderOn([](CR::Gfx::RenderLayer *layer){
        layer->add(Draw::Texture(dummyTexture, CR::Vec2<float>(0), dummyTexture->getRsc()->size, CR::Vec2<float>(0.0f), CR::Math::rads(0)));
    });
    dummyLayer->clear();
    dummyLayer->flush();



    static std::shared_ptr<CR::Gfx::RenderLayer> uiL = CR::Gfx::getRenderLayer("ui");
    static std::shared_ptr<CR::Gfx::RenderLayer> wL = CR::Gfx::getRenderLayer("world");
    



    wL->renderOn([&](CR::Gfx::RenderLayer *layer){    
        // layer->add(Draw::Texture(dummyTexture, CR::Vec2<float>(0), dummyTexture->size, CR::Vec2<float>(0.5f), CR::Math::rads(0)));
        // layer->add(CR::Gfx::Draw::PrimMesh(mBCube, 36, dummyTexture->getRsc()->textureId, CR::Vec3<float>(0.0f), CR::Vec3<float>(1.0f), CR::Vec4<float>(0.5f, 1.0f, 0.0f, CR::Math::rads(add)))) ;

        
        auto rscmodel = model->getRsc();
        for(int i = 0; i < rscmodel->meshes.size(); ++i){
            auto &mesh = rscmodel->meshes[i];
            layer->add(CR::Gfx::Draw::Mesh(mesh->md, transform, shader));
        }
    });


    uiL->renderOn([](CR::Gfx::RenderLayer *layer){    
        // layer->add(Draw::Texture(dummyTexture, CR::Vec2<float>(0), dummyTexture->size, CR::Vec2<float>(0.5f), CR::Math::rads(0)));
        layer->add(CR::Gfx::Draw::RenderLayer(dummyLayer, CR::Vec2<float>(0), CR::Vec2<int>(1000, 1000), CR::Vec2<float>(0.0f), 0.0f));
    });

    
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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


CR::Gfx::MeshData CR::Gfx::createPrimMesh(const std::vector<float> &vertices){

    unsigned vbo, vao;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindVertexArray(vao);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);    
    

    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);


    CR::Gfx::MeshData md;
    md.vao = vao;
    md.vbo = vbo;
    return md;
}

CR::Gfx::MeshData CR::Gfx::createMesh(const std::vector<CR::Gfx::Vertex> &vertices, const std::vector<unsigned int> &indices){
    unsigned int vao, vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
  
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, id));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weight));

    glBindVertexArray(0);        

    MeshData md;
    md.vao = vao;
    md.vbo = vbo;
    md.ebo = ebo;
    md.vertn = indices.size();
    return md;
}

bool CR::Gfx::deleteMesh(CR::Gfx::MeshData &md){
    return true;
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

bool CR::Gfx::applyShader(unsigned shaderId, const std::unordered_map<std::string, unsigned> &loc, const std::unordered_map<std::string, std::shared_ptr<CR::Gfx::ShaderAttr>> &attrs){
    glUseProgram(shaderId);
    if(attrs.size() == 0){
        return false;
    }
    
    for(auto &it : attrs){
        auto lit = loc.find(it.first);
        if(lit == loc.end()){
            CR::log("[GFX] applyShader: failed to find location for %s\n", it.first.c_str());
        }
        unsigned attrLoc = lit->second;
        switch(it.second->type){
            case CR::Gfx::ShaderAttrType::FLOAT: {
                auto attrf = std::static_pointer_cast<CR::Gfx::ShaderAttrFloat>(it.second);
                glUniform1f(attrLoc, attrf->n);
            } break;
            case CR::Gfx::ShaderAttrType::INT: {
                auto attri = std::static_pointer_cast<CR::Gfx::ShaderAttrInt>(it.second);
                glUniform1i(attrLoc, attri->n);
            } break;                 
            case CR::Gfx::ShaderAttrType::COLOR: {
                auto attrc = std::static_pointer_cast<CR::Gfx::ShaderAttrColor>(it.second);
                float v[3] = {attrc->color.r, attrc->color.g, attrc->color.b};
                glUniform3fv(attrLoc, 1, v);
            } break;
            case CR::Gfx::ShaderAttrType::VEC2: {
                auto attrvec = std::static_pointer_cast<CR::Gfx::ShaderAttrVec2>(it.second);
                float v[2] = {attrvec->vec.x, attrvec->vec.y};
                glUniform2fv(attrLoc, 1, v);
            } break;     
            case CR::Gfx::ShaderAttrType::VEC3: {
                auto attrvec = std::static_pointer_cast<CR::Gfx::ShaderAttrVec3>(it.second);
                float v[3] = {attrvec->vec.x, attrvec->vec.y, attrvec->vec.z};
                glUniform3fv(attrLoc, 1, v);
            } break; 
            case CR::Gfx::ShaderAttrType::MAT4: {
                auto attrmat = std::static_pointer_cast<CR::Gfx::ShaderAttrMat4>(it.second);
                glUniformMatrix4fv(attrLoc, 1, GL_FALSE, attrmat->mat.mat);                        
            } break;                                                           
            default: {
                CR::log("[GFX] undefined shader type to apply '%s'\n", it.second->type);
                return false;
            } break;
        }
    }
    return true;
}



void CR::Gfx::Camera::setPosition(const CR::Vec3<float> &pos){
    this->position = pos;    
}

void CR::Gfx::Camera::setFront(const CR::Vec3<float> &front){
    this->front = front;
    this->update();
}

void CR::Gfx::Camera::update(){
    front.x = Math::cos(Math::rads(yaw)) * Math::cos(Math::rads(pitch));
    front.y = Math::sin(Math::rads(pitch));
    front.z = Math::sin(Math::rads(yaw)) * Math::cos(Math::rads(pitch));
    front = front.normalize();
    right = front.cross(this->worldUp).normalize();  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = right.cross(front).normalize();
}

void CR::Gfx::Camera::setUp(const CR::Vec3<float> &up){
    this->worldUp = up;
    this->update();
}

void CR::Gfx::Camera::init(){
    this->yaw = -90.0f;
    this->pitch = 0.0f;
    this->up = CR::Vec3<float>(0.0f, 1.0f, 0.0f);
    this->worldUp = this->up;
    this->position = CR::Vec3<float>(0.0f, 0.0f, 0.0f);
    this->front = CR::Vec3<float>(0.0f, 0.0f, -1.0f);
    update();
}

CR::Mat<4, 4, float> CR::Gfx::Camera::getView(){
    return Math::lookAt(position, position + front, up);
}