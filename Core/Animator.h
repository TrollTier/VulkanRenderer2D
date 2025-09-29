//
// Created by patri on 18.09.2025.
//

#ifndef ANIMATION_H
#define ANIMATION_H

#include "AnimationData.h"

class Animator
{
public:
    size_t m_currentKeyFrame{};
    size_t m_nextKeyFrame{};
    size_t m_ticksSinceLastUpdate{};
    size_t m_gameObjectIndex{};
    size_t m_animationDataIndex{};

    explicit Animator(const size_t animationDataIndex)
    {
        m_animationDataIndex = animationDataIndex;
    }
};

#endif //ANIMATION_H
