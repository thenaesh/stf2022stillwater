#ifndef _BOILERPLATE_H
#define _BOILERPLATE_H


#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <cstdint>
#include <functional>
#include <algorithm>
#include <limits>
#include <chrono>
#include <thread>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>


template <typename U, typename V>
constexpr std::vector<V> map(std::vector<U> const& xs, std::function<V(U const&)> f) {
    std::vector<V> ys(xs.size());
    std::transform(xs.begin(), xs.end(), ys.begin(), f);
    return ys;
}

template <typename T>
constexpr bool contains(std::vector<T> const& outer, std::vector<T> const& inner) {
    for (auto const& e : inner) {
        if (std::find(outer.begin(), outer.end(), e) == outer.end()) {
            return false;
        }
    }

    return true;
}


#endif
