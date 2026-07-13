//
// Created by patri on 11.07.2026.
//

#ifndef DRAWREQUEST_H
#define DRAWREQUEST_H
#include <cstdint>

typedef struct
{
    size_t pipelineIndex = 0;
    size_t instanceIndex = 0;
    uint32_t layer = 0;
    uint32_t orderInLayer = 0;
} DrawRequest;

#endif //DRAWREQUEST_H
