#version 420 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D image;
uniform vec3 color;

void main(){
    FragColor = texture(image, texCoords) * vec4(color, 1.0);
}