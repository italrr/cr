#include <fstream>

#include "Flexie.hpp"
#include "../Graphics.hpp"
#include "../3rdparty/json11.hpp"

static std::vector<std::shared_ptr<CR::UI::Base>> windows;

CR::UI::Base::Base(){
    this->visible = true;
    this->solid = true;
}

CR::UI::Panel::Panel(){
}

void CR::UI::Panel::create(){
    this->toReRender = true;
    this->layer->init(CR::Gfx::RenderLayerType::T_2D, this->dims.size.x, this->dims.size.y);
}

void CR::UI::Panel::recalculate(){
    this->layer->resize(this->dims.size.x, this->dims.size.y);
    this->toReRender = true;
}

void CR::UI::Panel::setSize(const CR::Vec2<unsigned> &nSize){
    this->dims.setSize(nSize);
    recalculate();
}

void CR::UI::Panel::rerender(){
    if(!this->toReRender){
        return;
    }
    this->layer->clear();
    this->layer->renderOn([&](CR::Gfx::RenderLayer *layer){
        for(int i = 0; i < this->children.size(); ++i){
            this->children[i]->render(layer);
        }
    });
    this->layer->flush();
}


static std::string readToString(const std::string &path){
    std::ifstream file(path);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size); 
    return buffer;
}


static std::string parseString(json11::Json &obj, const std::string &name, const std::string &backup){
    auto v = obj[name];
    return v.is_string() ? v.string_value() : backup;
}

static std::shared_ptr<CR::UI::Layout> parseLayout(json11::Json &obj, const std::string &backup = "inline"){

    auto v = obj["layout"];
    auto type = v.is_string() ? v.string_value() : backup;


    auto layout = std::shared_ptr<CR::UI::Layout>(NULL);

    if(type == "vbox"){
        return std::make_shared<CR::UI::Layout>(CR::UI::LayoutType::VBox());
    }else
    if(type == "hbox"){  
        return std::make_shared<CR::UI::Layout>(CR::UI::LayoutType::HBox());
    }else
    if(type == "inline"){  
        return std::make_shared<CR::UI::Layout>(CR::UI::LayoutType::Inline());
    }

    return layout;
}


void CR::UI::Base::clear(){
    // TODO: destroy children
}

void CR::UI::LayoutType::VBox::recalculate(Base *head){

}


void CR::UI::LayoutType::HBox::recalculate(Base *head){

}

void CR::UI::LayoutType::Inline::recalculate(Base *head){

}


std::shared_ptr<CR::UI::Base> CR::UI::build(const std::string &path){

    if(!CR::File::exists(path)){
        CR::log("[UI] Failed to build '%s': It doesn't exist\n", CR::File::filename(path).c_str());
        return std::shared_ptr<CR::UI::Base>(NULL);
    }

    auto file = readToString(path);
    std::string error;
    auto obj = json11::Json::parse(file, error);
    if(error.size() > 0){
        CR::log("[UI] Failed to build '%s': JSON parsing error: \"%s\"\n", CR::File::filename(path).c_str(), error.c_str());
        return std::shared_ptr<CR::UI::Base>(NULL);
    }

    std::shared_ptr<CR::UI::Base> element;
    std::string type = parseString(obj, "type", "window");


    if(type == "window"){
        element = std::make_shared<CR::UI::Window>(CR::UI::Window());
        auto window = std::static_pointer_cast<CR::UI::Window>(element);
        auto layout = parseLayout(obj, "inline");
        if(!layout.get()){
            CR::log("[UI] Failed to build '%s': Layout type '%s' is unsupported\n", CR::File::filename(path).c_str(), parseString(obj, "layout", "").c_str());
            return std::shared_ptr<CR::UI::Base>(NULL);
        }
        auto title = parseString(obj, "title", "WINDOW");
        window->setTitle(title);
        windows.push_back(window);
    }else
    if(type == "PANEL"){

    }else
    if(type == "LABEL"){
        
    }    

    return element;
}





void __CR_init_FLEXIE(){
    auto win = CR::UI::build("./UITest.json");
}

void __CR_end_FLEXIE(){
    // TODO: Close Windows
}

void __CR_update_FLEXIE(){
    for(unsigned i = 0; i < windows.size(); ++i){
        if(windows[i]->type != CR::UI::ElementType::WINDOW){
            continue;
        }
        std::static_pointer_cast<CR::UI::Window>(windows[i])->step();
    }
}

void __CR_render_FLEXIE(CR::Gfx::RenderLayer *layer){
    for(unsigned i = 0; i < windows.size(); ++i){
        if(windows[i]->type != CR::UI::ElementType::WINDOW){
            continue;
        }
        std::static_pointer_cast<CR::UI::Window>(windows[i])->render(layer);
    }
}