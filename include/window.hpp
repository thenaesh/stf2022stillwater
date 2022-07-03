#ifndef BOILERPLATE_H
#define BOILERPLATE_H
#include <boilerplate.hpp>
#endif


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
