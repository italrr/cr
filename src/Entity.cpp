#include "Engine/Log.hpp"
#include "Engine/Tools.hpp"
#include "Entity.hpp"

CR::Entity::Entity(){
    this->controlType = CR::EntityControlType::IDLE;
    this->entType = CR::EntityType::INERT;
    this->anim = std::make_shared<CR::EntityAnim>(CR::EntityAnim());
}

void CR::Entity::build(CR::T_GENERICTYPE entType){
    // reset animations
    for(unsigned i = 0; i < 3; ++i){
        this->entStateTarget[i] = CR::EntityState::IDLE;
        this->entStateStart[i] = 0;
        this->entStateTarget[i] = 0;
    }
    this->controlType = CR::EntityControlType::IDLE;
    this->controllerId = 0;
}

void CR::Entity::setControlType(CR::T_GENERICTYPE ctype, CR::T_GENERICID cId){
    this->controllerId = cId;
    this->controlType = ctype;
}


void CR::Entity::walk(CR::T_WORLDPOS x, CR::T_WORLDPOS y){

}

void CR::Entity::draw(CR::Gfx::RenderLayer *layer){
    // layer->add(CR::Gfx::Draw::Mesh(rsc->mesh[i]->md, transform));
} 