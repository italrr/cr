#version 420 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D image;
uniform vec3 color;

void main(){
    vec4 texColor = texture(image, texCoords);
    if(texColor.a < 0.01){
        discard;
    }
    FragColor = texColor * vec4(color, 1.0);
}