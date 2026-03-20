//
// Created by patri on 03.01.2026.
//

#ifndef INPUT_H
#define INPUT_H
#include <functional>
#include <memory>
#include <vector>

#include "GLFW/glfw3.h"

enum class MouseButton
{
    Left = 0,
    Middle = 1,
    Right = 2
};

typedef struct
{
    MouseButton button;
    uint32_t x;
    uint32_t y;
} MouseClickEvent;

class Input
{
public:
    void onClick(std::function<void(const MouseClickEvent& data)> handler);
    void init(GLFWwindow* glfwWindow);

private:
    std::vector<std::function<void(const MouseClickEvent&)>> m_clickHandlers{};
    GLFWwindow* m_window;

    static void glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
    void mouseButtonCallback(int button, int action, int mods);
    static MouseButton getMouseButton(int button);
};

#endif //INPUT_H
