
#version 450 core

layout (location = 0) in vec3 aVertex; 
layout (location = 1) in vec2 aTexCoors; 

out vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 inTexCoords;
uniform vec2 inTexSize;

void main(){

   if(aTexCoors.x == 0.0){
      texCoords.x = inTexCoords.x;
   }
   if(aTexCoors.y == 0.0){
      texCoords.x = inTexCoords.y;
   }

   if(aTexCoors.x == 1.0){
      texCoords.x = inTexSize.x;
   }
   if(aTexCoors.y == 1.0){
      texCoords.y = inTexSize.y;
   }  

   gl_Position = (projection * view * model) * vec4(aVertex, 1.0);
}