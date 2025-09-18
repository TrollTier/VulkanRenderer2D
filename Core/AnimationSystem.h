//
// Created by patri on 18.09.2025.
//

#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H

#include <vector>
#include "AnimationData.h"
#include "Animator.h"
#include "Timestep.h"

class AnimationSystem
{
public:
    AnimationSystem();

    void addAnimationData(AnimationData data);
    void addAnimator(Animator animator);
    void update(const Timestep& timestep);
    [[nodiscard]] AnimationData& getAnimationDataByName(std::string name);
    [[nodiscard]] const AnimationData& getAnimationDataByName(std::string name) const;
    [[nodiscard]] const Animator& getAnimator(size_t index) const;

private:
    std::vector<AnimationData> m_animationData;
    std::vector<Animator> m_animators;
};

#endif //ANIMATIONSYSTEM_H
