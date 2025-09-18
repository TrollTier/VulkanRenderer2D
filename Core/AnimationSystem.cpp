//
// Created by patri on 18.09.2025.
//

#include "AnimationSystem.h"

#include <stdexcept>

AnimationSystem::AnimationSystem()
{
    m_animationData.clear();
    m_animators.clear();
}

void AnimationSystem::addAnimationData(AnimationData data)
{
    m_animationData.push_back(data);
}

void AnimationSystem::addAnimator(Animator animator)
{
    m_animators.push_back(animator);
}

void AnimationSystem::update(const Timestep &timestep)
{
    for (auto& animator : m_animators)
    {
        if (animator.m_animationData == nullptr)
        {
            continue;
        }

        if (animator.m_currentKeyFrame == animator.m_animationData->keyFrames.size() - 1 &&
            !animator.m_animationData->loops)
        {
            continue;
        }

        animator.m_ticksSinceLastUpdate += 1;

        const auto& keyFrame = animator.m_animationData->keyFrames[animator.m_currentKeyFrame];

        size_t nextIndex = (animator.m_currentKeyFrame + 1) % animator.m_animationData->keyFrames.size();
        const auto& nextKeyFrame = animator.m_animationData->keyFrames[nextIndex];

        if (animator.m_ticksSinceLastUpdate >= nextKeyFrame.afterFrames)
        {
            animator.m_currentKeyFrame = nextIndex;
            animator.m_ticksSinceLastUpdate = 0;
        }
    }
}

AnimationData& AnimationSystem::getAnimationDataByName(std::string name)
{
    for (size_t i = 0; i < m_animationData.size(); i++)
    {
        if (m_animationData[i].name == name)
        {
            return m_animationData[i];
        }
    }

    throw std::runtime_error("Could not find animation data");
}

const AnimationData& AnimationSystem::getAnimationDataByName(std::string name) const
{
    for (size_t i = 0; i < m_animationData.size(); i++)
    {
        if (m_animationData[i].name == name)
        {
            return m_animationData[i];
        }
    }

    throw std::runtime_error("Could not find animation data");
}

const Animator& AnimationSystem::getAnimator(size_t index) const
{
    return m_animators[index];
}
