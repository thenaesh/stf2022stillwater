#ifndef WINDOW_H
#define WINDOW_H
#include <window.hpp>
#endif


WindowState::WindowState(
    const char* title,
    uint32_t width,
    uint32_t height
) : title{title}, width{width}, height{height} {
    glfwInit();

    this->createWindow();
}

WindowState::~WindowState() {
    this->destroyWindow();

    glfwTerminate();
}


void WindowState::createWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->_window = glfwCreateWindow(this->width, this->height, this->title, nullptr, nullptr);
    this->getRequiredExtensions();
}

void WindowState::destroyWindow() {
    glfwDestroyWindow(this->_window);
}


bool WindowState::isActive() {
    return !glfwWindowShouldClose(this->_window);
}


void WindowState::getRequiredExtensions() {
    this->glfwExtensions = glfwGetRequiredInstanceExtensions(&this->glfwExtensionCount);
}
