#version 450 core

out vec4 color;

in vec2 texCoords;

uniform sampler2D tex;
uniform vec3 spriteColor;

void main(){
    color = texture(tex, texCoords) * vec4(spriteColor, 1.0);
}