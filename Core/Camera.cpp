//
// Created by patri on 02.08.2025.
//

#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(
    glm::vec3 worldPosition,
    CameraArea visibleArea,
    float extentsWidth,
    float extentsHeight)
{
    m_extentsWidth = extentsWidth;
    m_extentsHeight = extentsHeight;
    m_visibleArea = visibleArea;

    updatePosition(worldPosition);
}

void Camera::resize(CameraArea visibleArea, float extentsWidth, float extentsHeight)
{
    m_visibleArea = visibleArea;
    m_extentsWidth = extentsWidth;
    m_extentsHeight = extentsHeight;

    updatePosition(m_worldPosition);
}


void Camera::moveBy(glm::vec3 deltaPosition)
{
    updatePosition(m_worldPosition + deltaPosition);
}

void Camera::moveTo(glm::vec3 worldPosition)
{
    updatePosition(worldPosition);
}

void Camera::setVisibleArea(CameraArea visibleArea)
{
    m_visibleArea = visibleArea;
    updatePosition(m_worldPosition);
}


void Camera::updatePosition(glm::vec3 worldPosition)
{
    m_worldPosition = worldPosition;

    glm::vec3 eye    = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up     = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(eye, center, up);

    glm::mat4 projection = glm::ortho(
        0.0f,
        m_extentsWidth,
        0.0f,
        m_extentsHeight);

    m_viewProjectionMatrix = projection * view;

    const auto halfWidth = m_visibleArea.width / 2;
    const auto halfHeight = m_visibleArea.height / 2;

    m_cameraFrustum.toX = worldPosition.x + halfWidth;
    m_cameraFrustum.toY = worldPosition.y + halfHeight;
    m_cameraFrustum.x = worldPosition.x - halfWidth;
    m_cameraFrustum.y = worldPosition.y - halfHeight;
    m_cameraFrustum.width = m_visibleArea.width;
    m_cameraFrustum.height = m_visibleArea.height;
}

const glm::mat4& Camera::getViewProjectionMatrix() const
{
    return m_viewProjectionMatrix;
}

const Camera::CameraFrustum &Camera::getFrustum() const
{
    return m_cameraFrustum;
}



