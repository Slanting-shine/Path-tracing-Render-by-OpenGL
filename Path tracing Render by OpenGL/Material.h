#pragma once
#include "Common.h"

struct Material {
    vec3 emissive = vec3(0, 0, 0);  // 作为光源时的发光颜色
    vec3 baseColor = vec3(1, 1, 1);
    float subsurface = 0.0;
    float metallic = 0.0;
    float specular = 0.5;
    float specularTint = 0.0;
    float roughness = 0.5;
    float anisotropic = 0.0;
    float sheen = 0.0;
    float sheenTint = 0.5;
    float clearcoat = 0.0;
    float clearcoatGloss = 1.0;
    float IOR = 1.0;
    float transmission = 0.0;
    float dielectric = 0.0;
};