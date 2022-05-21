#define GLFW_INCLUDE_NONE

#include <signal.h>
#include <stdarg.h>
#include <algorithm>
#include <string.h>
#include <memory>
#include <GLFW/glfw3.h>

#include "3rdparty/glad.h"
#include "Tools.hpp"
#include "Log.hpp"
#include "Graphics.hpp"

static GLFWwindow *window = NULL;
static bool running = true;
static CR::Indexing::Indexer indexer;
static CR::Resource::ResourceManager rscmng;
static std::shared_ptr<CR::Gfx::Settings> settings = std::shared_ptr<CR::Gfx::Settings>(new CR::Gfx::Settings());

void __CR_init_input(GLFWwindow *window);
void __CR_end_input();

void __CR_init_job();
void __CR_end_job();
void __CR_update_job();

CR::Indexing::Indexer *CR::getIndexer(){
    return &indexer;
}

CR::Resource::ResourceManager *CR::getResourceMngr(){
    return &rscmng;
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
        CR::log("Failed to parse %s: Doesn't exist\n", fpath.c_str());
        return;
    }

    if(!obj.isValid()){
        CR::log("Failed to parse %s: It's invalid\n", fpath.c_str());
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

    glViewport(0, 0, settings->width, settings->height);

    glEnable(GL_DEPTH_TEST);

    
    signal(SIGINT, ctrlC);

    __CR_init_input(window);
    __CR_init_job();
    indexer.scan("data"+CR::File::dirSep());

    return true;
}

void CR::Gfx::render(){
    if(!running){
        return;
    }

    if(glfwWindowShouldClose(window)){
        end();
        return;
    }

    // RENDER

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