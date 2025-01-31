#version 450

layout(binding = 1) uniform sampler2D texSampler[255];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(push_constant) uniform PushConstantData {
  uint textureIndex;
};

layout(location = 0) out vec4 outColor;

void main() {
  // Utilise l'indice transmis par l'application pour choisir la texture
  outColor = texture(texSampler[textureIndex], fragTexCoord);
}
