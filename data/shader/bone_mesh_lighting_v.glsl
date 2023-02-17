#version 420 core

const int MAX_BONES = 100; // Max number of bones

layout (location = 0) in vec3 aVertex; 
layout (location = 1) in vec2 aTexCoors; 
layout (location = 2) in vec3 aNormal;
layout (location = 3) in ivec4 VertID;
layout (location = 4) in vec4 VertWeight;

out vec3 FragPos;
out vec3 Normal;
out vec2 texCoords;
out ivec4 ID;
out vec4 Weight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 gBones[MAX_BONES]; 

void main(){

   mat4 BoneTransform = gBones[ VertID[0] ] * VertWeight[0];
	BoneTransform += gBones[ VertID[1] ] * VertWeight[1];
   BoneTransform += gBones[ VertID[2] ] * VertWeight[2];
   BoneTransform += gBones[ VertID[3] ] * VertWeight[3];


   vec4 tNormal = BoneTransform * vec4(aNormal, 1.0);
   mat3 NormalMatrix = mat3(transpose(inverse(model)));
   Normal = normalize(mat4(NormalMatrix) * tNormal).xyz;


   vec4 tpos = BoneTransform * vec4(aVertex, 1.0);

   gl_Position = (projection * view * model) * tpos;

   texCoords = vec2(aTexCoors.x, aTexCoors.y);
   ID = VertID;
   Weight = VertWeight;    
   FragPos = (model * tpos).xyz;

}