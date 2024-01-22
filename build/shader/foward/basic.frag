#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout(binding = 0) uniform sampler2D bColor;
layout(binding = 1) uniform sampler2D bMaterial;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

in vec3 viewPos;
in vec3 viewVector;

void main() {
    fragColor = vec4(1.0);
    fragNormal = vec3(0.0);
    fragEmmisive = vec3(0.0);
}