#version 420 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D image;
uniform vec3 outlineCol;
uniform vec3 fillCol;

void main(){  
    vec2 tc = texCoords.xy;
    // vec3 outlineCol    = vec3(0.0, 0.0, 0.0); // e.g blue
    // vec3 fillCol = vec3(1.0, 0.0, 0.0); // e.g red

    vec2 tex = texture2D(image, tc).rg;
    float fill    = tex.g;
    float outline = tex.r;

    float alpha    = max( fill, outline );
    vec3 mixColor = mix( mix(vec3(0.0), outlineCol, fill), fillCol, outline );

    FragColor = vec4(mixColor, alpha);
}