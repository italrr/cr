#version 450 core

layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoors;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 Bittanget;
layout (location = 5) in ivec4 VertID;
layout (location = 6) in vec4 VertWeight;

out vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
   texCoords = vec2(aTexCoors.x, aTexCoors.y);

   gl_Position = (projection * view * model) * vec4(aVertex, 1.0);
}