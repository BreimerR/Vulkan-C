//
// Created by brymher on 18/12/24.
//

#ifndef LEARNING_GLFW_H
#define LEARNING_GLFW_H

#ifndef LEARNING_GLFW_APP_H

#include "glfw_app.h"

#endif // LEARNING_GLFW_APP_H

#include <GLFW/glfw3.h>

void disableOpenGL() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void disableResize() {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

#endif //LEARNING_GLFW_H
