//
// Created by patri on 02.08.2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include "ViewFrustrum.h"

class Camera
{
public:
    typedef struct
    {
        float x;
        float y;
        float width;
        float height;
        float toX;
        float toY;
    } CameraFrustum;

    Camera(
        glm::vec3 worldPosition,
        CameraArea visibleArea,
        float extentsWidth,
        float extentsHeight);

    void resize(
        CameraArea visibleArea,
        float extentsWidth,
        float extentsHeight);

    void moveTo(glm::vec3 worldPosition);
    void moveBy(glm::vec3 deltaPosition);
    void setVisibleArea(CameraArea visibleArea);

    [[nodiscard]] const glm::mat4& getViewProjectionMatrix() const;
    [[nodiscard]] const CameraFrustum& getFrustum() const;
private:
    glm::vec3 m_worldPosition = glm::vec3(1);
    glm::mat4 m_viewProjectionMatrix = glm::mat4(1.0f);
    float m_extentsWidth = 1.0f;
    float m_extentsHeight = 1.0f;
    CameraArea m_visibleArea{1.0f, 1.0f, 1.0f, 1.0f};
    CameraFrustum m_cameraFrustum{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

    void updatePosition(glm::vec3 worldPosition);
};

#endif //CAMERA_H
