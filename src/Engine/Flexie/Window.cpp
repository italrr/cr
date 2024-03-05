#include "Flexie.hpp"

CR::UI::Window::Window(){
    this->type = ElementType::WINDOW;
}

void CR::UI::Window::move(const CR::Vec2<unsigned> &nPos){

}

void CR::UI::Window::close(){

}

void CR::UI::Window::setTitle(const std::string &title){
    this->title = title;
}

void CR::UI::Window::render(CR::Gfx::RenderLayer *layer){

}

void CR::UI::Window::setSize(const CR::Vec2<unsigned> &nSize){
    
}