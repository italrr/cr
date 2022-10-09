#version 420 core

layout (location = 0) in vec3 aVertex; 
layout (location = 1) in vec2 aTexCoors; 

out vec2 texCoords;

uniform mat4 model;
uniform mat4 projection;

void main(){
   texCoords = vec2(aTexCoors.x, aTexCoors.y);

   gl_Position = (projection * model) * vec4(aVertex, 1.0);
}