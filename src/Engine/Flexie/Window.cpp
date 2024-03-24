#include "../Texture.hpp"
#include "Flexie.hpp"

static CR::Gfx::Texture blank;

CR::UI::Window::Window(){
    this->type = ElementType::WINDOW;

    CR::Gfx::Bitmap bm;
    bm.build(CR::Color(255, 255, 255, 255), CR::Gfx::ImageFormat::RGBA, 32, 32);

    blank.load(bm);

}

void CR::UI::Window::step(){
}

void CR::UI::Window::move(const CR::Vec2<unsigned> &nPos){

}

void CR::UI::Window::close(){

}

void CR::UI::Window::setTitle(const std::string &title){
    this->title = title;
}

void CR::UI::Window::render(CR::Gfx::RenderLayer *layer){
    // We expect this layer to be 2D ^
    if(!this->visible){
        return;
    }
    this->rerender();
    // Render Base
    layer->add(CR::Gfx::Draw::Texture(blank, {0.0f, 0.0f}, {300, 300}, {0.0f, 0.0f}, 0.0f));

    // Render children


}
