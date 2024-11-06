#pragma once
#include "Color.h"
#include "util/Span.h"


namespace spr::color{

static const Span<rgb32> gradientMap[] = {
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    magma,
    viridis,
    gradients0,
    gradient1,
    gradient2,
    gradient3,
    gradient4,
    gradient5,
    gradient6,
    gradient7,
    gradient8,
    gradient9,
    gradient10,
    gradient11,
    gradient12,
    gradient13,
    gradient14,
    gradient15,
    gradient16,
    gradient17,
    gradient18,
    gradient19,
    gradient20,
    turbo,
};

static rgb32 sample(Span<rgb32> data, float value){
    value = glm::clamp(value, 0.f, 1.f);

    float mapped = value * data.size();
    uint32 i1 = mapped;

    float t1 = mapped-glm::floor(mapped);
    float d = 0.5f-t1;
    float abs = glm::abs(d);

    if ((i1 == 0 && d > 0) || (i1 == data.size()-1 && d <= 0))
        return data[i1];
    
    vec3 current = data[i1];
    vec3 other = d > 0 ? data[i1-1] : data[i1+1];

    current /= 255.f;
    other /= 255.f;

    return ((1.f - abs)*current + (abs)*other)*255.f;
}

static inline rgb32 sample(Span<rgb32> data, uint8 value){
    float fvalue = ((float)glm::clamp(value, (uint8)0, (uint8)255))/255.f;
    return sample(data, fvalue);
}

}