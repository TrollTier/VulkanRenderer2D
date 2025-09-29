//
// Created by patri on 18.09.2025.
//

#include "AnimationSystem.h"

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
    const auto& animationData = m_animationData[animator.m_animationDataIndex];
    animator.m_nextKeyFrame = (animator.m_currentKeyFrame + 1) % animationData.keyFrames.size();

    m_animators.push_back(animator);
}

void AnimationSystem::update(const Timestep &timestep)
{
    for (auto& animator : m_animators)
    {
        const auto& animationData = m_animationData[animator.m_animationDataIndex];

        if (animator.m_currentKeyFrame == animationData.keyFrames.size() - 1 &&
            !animationData.loops)
        {
            continue;
        }

        animator.m_ticksSinceLastUpdate += 1;

        const auto& nextKeyFrame = animationData.keyFrames[animator.m_nextKeyFrame];

        if (animator.m_ticksSinceLastUpdate >= nextKeyFrame.afterFrames)
        {
            size_t nextFrame = (animator.m_currentKeyFrame + 1) % animationData.keyFrames.size();
            size_t frameAfterNext = (animator.m_currentKeyFrame + 2) % animationData.keyFrames.size();

            animator.m_currentKeyFrame = nextFrame;
            animator.m_ticksSinceLastUpdate = 0;
            animator.m_nextKeyFrame = frameAfterNext;
        }
    }
}

size_t AnimationSystem::getAnimationDataIndexByName(const std::string_view& name) const
{
    for (size_t i = 0; i < m_animationData.size(); i++)
    {
        if (m_animationData[i].name == name)
        {
            return i;
        }
    }

    return 0;
}

const AnimationData* AnimationSystem::getAnimationData(const size_t index) const
{
    return &m_animationData[index];
}


const Animator& AnimationSystem::getAnimator(size_t index) const
{
    return m_animators[index];
}
