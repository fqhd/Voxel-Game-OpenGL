//
// Created by cew05 on 16/04/2024.
//

#ifndef UNTITLED7_CUBE_H
#define UNTITLED7_CUBE_H

#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <glew.h>
#include <random>

#include "CoreGlobals.h"
#include "Window.h"

class Triangle {
    private:
        // GL bindings
        unsigned int vertexArrayObject {};
        unsigned int vertexBufferObject {};

        // Vertex Data
        GLint numAttribs{};
        std::vector<float> vertexArray {};

        // Display
        unsigned int textureObject {};
        unsigned int colorBufferObject {};
        std::vector<float> vertexColorArray {};

        // Movement
        std::pair<float, float> vel {0.005f, 0.008f};

    public:
        // Constructors
        Triangle();
        ~Triangle();
        void ConstructTriangle(GLint _numAttribs, const std::vector<float>& _vertexArray);

        // Movement
        void Move();

        // Display
        void Display() const;
};


#endif //UNTITLED7_CUBE_H
