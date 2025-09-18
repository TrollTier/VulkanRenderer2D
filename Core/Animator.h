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
    size_t m_ticksSinceLastUpdate{};
    size_t m_gameObjectIndex{};
    const AnimationData* m_animationData = nullptr;

    explicit Animator(const AnimationData* animationData)
    {
        m_animationData = animationData;
    }
};

#endif //ANIMATION_H
