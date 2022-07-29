#ifndef _WINDOW_H
#define _WINDOW_H


#include <boilerplate.hpp>


class WindowState {
    GLFWwindow* _window;

public:
    const char* title;
    uint32_t width;
    uint32_t height;
    uint32_t glfwExtensionCount;
    const char** glfwExtensions;

public:
    WindowState(const char* title, uint32_t width, uint32_t height);
    virtual ~WindowState();

    bool isActive();

    operator GLFWwindow*() const { return this->_window; }

private:
    void createWindow();
    void destroyWindow();
    void getRequiredExtensions();
};


#endif
