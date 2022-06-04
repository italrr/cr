#version 450 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 color;
in vec2 texCoords;

uniform sampler2D tex;

void main(){
    FragColor = texture(tex, texCoords) * vec4(color, 1.0);
}