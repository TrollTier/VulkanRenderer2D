#include "Input.h"

#include "WindowContext.h"
//
// Created by patri on 14.01.2026.
//
void Input::init(GLFWwindow *glfwWindow)
{
    m_window = glfwWindow;
    glfwSetMouseButtonCallback(glfwWindow, glfwMouseButtonHandler);
}

void Input::onClick(std::function<void(const MouseClickEvent &data)> handler)
{
    m_clickHandlers.push_back(std::move(handler));
}

void Input::glfwMouseButtonHandler(GLFWwindow *window, int button, int action, int mods)
{
    auto context = reinterpret_cast<WindowContext*>(glfwGetWindowUserPointer(window));
    context->input->mouseButtonCallback(button, action, mods);
}

void Input::mouseButtonCallback(int button, int action, int mods)
{
    const auto enumButton = getMouseButton(button);

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    const MouseClickEvent event(
        enumButton,
        static_cast<uint32_t>(xpos),
        static_cast<uint32_t>(ypos));

    for (const auto& listener : m_clickHandlers)
    {
        listener(event);
    }
}

MouseButton Input::getMouseButton(int button)
{
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_1:
            return MouseButton::Left;

        case GLFW_MOUSE_BUTTON_2:
            return MouseButton::Right;

        case GLFW_MOUSE_BUTTON_3:
            return MouseButton::Middle;

        default: throw std::runtime_error("MouseButton not supported");
    }
}


