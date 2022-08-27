#ifndef _BOILERPLATE_H
#define _BOILERPLATE_H


#include <iostream>
#include <array>
#include <vector>
#include <tuple>
#include <string>
#include <cstring>
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


using namespace std;


template <typename U, typename V>
constexpr vector<V> map(vector<U> const& xs, function<V(U const&)> f) {
    vector<V> ys(xs.size());
    transform(xs.begin(), xs.end(), ys.begin(), f);
    return ys;
}

template <typename T>
constexpr bool contains(vector<T> const& outer, vector<T> const& inner) {
    for (auto const& e : inner) {
        if (find(outer.begin(), outer.end(), e) == outer.end()) {
            return false;
        }
    }

    return true;
}


#endif
