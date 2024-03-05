#include "Console.hpp"
#include "cv/src/cv.hpp"

static std::shared_ptr<CV::Item> ctx = CV::createContext(NULL, false);
static std::vector<std::string> buffer;
static std::shared_ptr<CR::Gfx::RenderLayer> consoleLayer;
static const unsigned verticalLines = 15;
static const unsigned fontSize = 32;
static CR::Vec2<int> dimensions;
static CR::Color bgColor(0.4f, 0.4f, 0.4f, 1.0);


void __CR_console_add_line(const std::string &line){
    buffer.push_back(line);
}

void __CR_init_console(){
    auto ws = CR::Gfx::getSize();
    dimensions.x = ws.x;
    dimensions.y = verticalLines * fontSize;
    consoleLayer = CR::Gfx::createRenderLayer(dimensions, CR::Gfx::RenderLayerType::T_2D, "console", true, CR::Gfx::RenderLayers::CONSOLE);
    CR::log("[SYS] CANVAS INTERPRETER v%.0f.%.0f.%.0f\n", CANVAS_LANG_VERSION[0], CANVAS_LANG_VERSION[1], CANVAS_LANG_VERSION[2]);
}

void __CR_end_console(){
    buffer.clear();
}

void __CR_update_console(){

}

void __CR_render_console(CR::Gfx::RenderLayer *layer){

}