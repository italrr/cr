#version 450 core

out vec4 FragColor;

in vec2 texCoords;


/*
    MATERIAL
*/
struct Material {
    sampler2D diffuse;
    vec3 color; // added to diffuse
};
uniform Material material;

/*
    LIGHTNING
    (TODO)
*/

void main(){
    FragColor = texture(material.diffuse, texCoords) * vec4(material.color, 1.0);
}