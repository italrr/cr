#version 420 core

out vec4 FragColor;

uniform sampler2D image;
uniform vec3 color;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 texCoords;
flat in ivec4 ID;
in vec4 Weight;

void main(){
    vec4 texColor = texture(image, texCoords);
    if(texColor.a < 0.01){
        discard;
    }
    FragColor = texColor * vec4(color, 1.0);
}