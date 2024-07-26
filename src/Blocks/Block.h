//
// Created by cew05 on 07/07/2024.
//

#ifndef UNTITLED7_BLOCK_H
#define UNTITLED7_BLOCK_H

#include <memory>
#include <vector>
#include <string>

#include <glew.h>
#include <glm/matrix.hpp>

#include "../Textures/TextureData.h"
#include "ModelStructs.h"
#include "ModelTransformations.h"
#include "../Player/Camera.h"
#include "../Textures/TextureManager.h"

enum class BLOCKID {
        GRASS, DIRT, STONE, WATER, AIR,
};

struct BlockData {
    // Used to identify what the actual block object is of
    BLOCKID blockID {BLOCKID::AIR};
    int variantID {0};

    static bool Compare(BlockData A, BlockData B) {
        return A.blockID == B.blockID && A.variantID == B.variantID;
    }
};


class Block {
    protected:
        // Buffer objects
        unsigned int vertexArrayObject {};
        unsigned int vertexBufferObject {};
        unsigned int indexBufferObject {};

        // Culling Information
        bool transparent = false;
        bool isCulled = false;
        bool visibleFaces[6] {true, true, true, true, true, true};

        // Block Data
        BlockData blockData {};

        // Determines correct texture sheet to draw texture from
        TEXTURESHEET textureSheet = TEXTURESHEET::WORLD;

    public:
        Block();
        ~Block();

        // Object Creation
        static std::vector<Vertex> BaseVertexArray();
        static std::vector<GLuint> BaseIndexArray();
        void SetBlockData(BlockData _data);
        void BindCube() const;

        // Display
        void Display(const Transformation& _transformation) const;
        bool CheckCulling(const Camera& _camera, const Transformation& _transformation);

        // Textures
        void SetTexture(TEXTURESHEET _textureID, glm::vec2 _origin);
        static std::vector<Vertex> GetTrueTextureCoords(TEXTURESHEET _sheetID, glm::vec2 _textureOrigin);

        // Transformations
//        void SetPositionOrigin(glm::vec3 _originPosition);
//        void SetPositionCentre(glm::vec3 _centre);
//        void SetScale(glm::vec3 _scale);
//        void SetRotation(glm::vec3 _rotation);
//        void UpdateModelMatrix();
//        void UpdateModelMatrix(const glm::mat4& _parentTransformationMatrix);

        // Getters
        [[nodiscard]] bool IsTransparent() const { return transparent; }
        [[nodiscard]] BlockData GetBlockData() const { return blockData; }
};

#endif //UNTITLED7_BLOCK_H
