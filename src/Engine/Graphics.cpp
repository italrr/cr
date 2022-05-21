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
static std::shared_ptr<CR::Gfx::Settings> settings = std::shared_ptr<CR::Gfx::Settings>(new CR::Gfx::Settings());

// since input comes from the window itself
void __CR_init_input(GLFWwindow *window);
void __CR_end_input();

void __CR_init_job();
void __CR_end_job();
void __CR_update_job();

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
    auto obj = parser.parseFile(path);

    if(!CR::File::exists(path)){
        CR::log("Failed to parse %s: Doesn't exist\n", path.c_str());
        return;
    }

    if(!obj.isValid()){
        CR::log("Failed to parse %s: It's invalid\n", path.c_str());
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
    CR::log("caught ctrl-c\n");
	CR::Gfx::end();
}

bool CR::Gfx::init(){

    std::string wst = settings->fullscreen ? "fullscreen" : "windowed";
    
    CR::log("[GFX] init | res %dx%d | %s | %s \n", settings->width, settings->height, Core::SupportedPlatform::name(Core::PLATFORM).c_str(), wst.c_str());


    if (!glfwInit()){
        CR::log("[GFX] failed to start 'GLFW'");
        CR::Core::exit(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, settings->resizable ? GL_TRUE : GL_FALSE);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(settings->width, settings->height, "CAVERN RUSH", NULL, NULL);

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
    // glfwWindowHint(GLFW_RESIZABLE, sett.resizeable ? GL_TRUE : GL_FALSE);



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
    __CR_end_input();
    __CR_end_job();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void CR::Gfx::onEnd(){
    CR::log("Bye!\n");
    running = false;
}

bool CR::Gfx::isRunning(){
    return running;
}