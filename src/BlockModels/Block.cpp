//
// Created by cew05 on 07/07/2024.
//

#include "Block.h"

#include "../Window.h"
#include <SDL.h>
#include <SDL_image.h>

BlockVAOs::BlockVAOs() {
    // Generate objectIDs
    glGenVertexArrays(nModels, (GLuint*)&vertexArrayObject[0]);
    glGenBuffers(nModels, (GLuint*)&vertexBufferObject[0]);
    glGenBuffers(nModels, (GLuint*)&indexBufferObject[0]);
}

BlockVAOs::~BlockVAOs() {
    glDeleteBuffers(nModels, (GLuint*)&vertexBufferObject[0]);
    glDeleteBuffers(nModels, (GLuint*)&indexBufferObject[0]);
    glDeleteVertexArrays(nModels, (GLuint*)&vertexArrayObject[0]);
}


std::vector<Vertex> BlockVAOs::FullblockVA() {
    // [POSITION], [TEXCOORD], both values are offsets relative to the set origin points
    return {
            // Front
            { glm::vec3(0.0f, 0.0f, 0.0f),  glm::vec2(0.0f, 0.0f) },            // TOPLEFT VERTEX
            { glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(1.0f, 0.0f) },            // TOPRIGHT VERTEX
            { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },            // BOTTOMLEFT VERTEX
            { glm::vec3(0.0f, -1.0f, 1.0f), glm::vec2(1.0f, 1.0f) },            // BOTTOMRIGHT VERTEX

            // Left
            { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(-1.0f, 0.0f) },            // TOPLEFT VERTEX
            { glm::vec3(1.0f, -1.0f, 0.0f),glm::vec2(-1.0f, 1.0f) },            // BOTTOMLEFT VERTEX
//            { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) },            //  VERTEX
//            { glm::vec3(0.0f, -1.0f, 0.0f),glm::vec2(0.0f, 1.0f) },            //  VERTEX

            // Right
            { glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(2.0f, 0.0f) },            // TOPRIGHT VERTEX
            { glm::vec3(1.0f, -1.0f, 1.0f), glm::vec2(2.0f, 1.0f) },            // BOTTOMRIGHT VERTEX
//            { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f) },            //  VERTEX
//            { glm::vec3(0.0f, -1.0f, 1.0f),glm::vec2(1.0f, 1.0f) },            //  VERTEX

            // Back
            { glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(3.0f, 0.0f) },             // TOPRIGHT VERTEX
            { glm::vec3(1.0f, -1.0f, 0.0f),  glm::vec2(3.0f, 1.0f) },             // BOTTOMRIGHT VERTEX
//            { glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(2.0f, 0.0f) },            //  VERTEX
//            { glm::vec3(1.0f, -1.0f, 1.0f),glm::vec2(2.0f, 1.0f) },            //  VERTEX

            // Top
            { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, -1.0f) },            // TOPLEFT VERTEX
            { glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(1.0f, -1.0f) },            // TOPRIGHT VERTEX
//            { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) },            //  VERTEX
//            { glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 0.0f) },            //  VERTEX

            // Bottom
            { glm::vec3(1.0f, -1.0f, 0.0f),glm::vec2(0.0f, 2.0f) },            // BACKLEFT VERTEX
            { glm::vec3(1.0f, -1.0f, 1.0f),glm::vec2(1.0f, 2.0f) },            // BACKRIGHT VERTEX
//            { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, -1.0f) },            //  VERTEX
//            { glm::vec3(0.0f, -1.0f, 1.0f),glm::vec2(1.0f, -1.0f) },            //  VERTEX

    };
}

std::vector<GLuint> BlockVAOs::FullblockIA() {
    return {
            1, 0, 3, 3, 0, 2,     // FRONT
            8, 6, 9, 9, 6, 7,     // BACK
            0, 4, 2, 2, 4, 5,     // LEFT
            6, 1, 7, 7, 1, 3,     // RIGHT
            11, 10, 1, 1, 10, 0,  // TOP
            3, 2, 13, 13, 2, 12   // BOTTOM
    };
}



void BlockVAOs::BindBlockModels() const {
    // Obtain vertex data for each model, and bind to VAO
    for (int model = 0; model < nModels; model++) {
        // Bind VAO
        glBindVertexArray(vertexArrayObject[model]);

        // Fetch model's VertexArray and IndexArray
        std::vector<Vertex> vertexArray = GetBaseVertexArray((BLOCKMODEL)model);
        std::vector<GLuint> indexArray = GetBaseIndexArray((BLOCKMODEL)model);

        // Bind VertexBuffer
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject[model]);
        glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(vertexArray.size() * sizeof(Vertex)), vertexArray.data(), GL_STATIC_DRAW);

        // Position Data Attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(Vertex, position));

        // Texture Data Attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(Vertex, textureCoord));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject[model]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexArray.size() * sizeof(GLuint)), indexArray.data(), GL_STATIC_DRAW);
    }

    // Unbind arrays and buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

std::vector<GLuint> BlockVAOs::GetBaseIndexArray(BLOCKMODEL _model) {
    std::vector<GLuint> indexArray;
    switch (_model) {
        case FULL:
            indexArray = FullblockIA();
            break;

            // ...

        default:
            indexArray = FullblockIA();
    }

    return indexArray;
}

std::vector<Vertex> BlockVAOs::GetBaseVertexArray(BLOCKMODEL _model) {
    std::vector<Vertex> vertexArray;
    switch (_model) {
        case FULL:
            vertexArray = FullblockVA();
            break;

            // ...

        default:
            vertexArray = FullblockVA();
    }

    return vertexArray;
}







Block::Block() {

}

Block::~Block() {

}

void Block::Display(Transformation* _t) {
    if (!inCamera || culled) return;
    if (blockData.blockID == BLOCKID::AIR && blockData.variantID == 0) return;

    // Bind to the model
    glBindVertexArray(blockVAOmanager->vertexArrayObject[blockModel]);

    // Update uniform
    GLint modelMatrixLocation = glGetUniformLocation(window.GetShader(), "uModelMatrix");
    if (modelMatrixLocation < 0) printf("block location not found [uModelMatrix]\n");
    if (modelMatrixLocation >= 0) glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &_t->GetModelMatrix()[0][0]);

    // Apply Texture
    textureManager->EnableTextureSheet(sheet);
    GLint uniformLocation = glGetUniformLocation(window.GetShader(), "uVertexTextureCoordOffset");
    if (uniformLocation < 0) printf("block location not found [uVertexTextureCoordOffset]\n");
    if (uniformLocation >= 0) glUniform2fv(uniformLocation, 1, &origin[0]);

    // Draw Block (max count = 36)
    glDrawElements(GL_TRIANGLES, 6*(int)visibleFaces.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Block::SetTransformation(Transformation *_t) {
//    blockTransformation = _t;
}





std::vector<Vertex> Block::GetFaceVerticies(const std::vector<BLOCKFACE> &_faces) const {
    // Get base index and vertex arrays of the model. Create vector to store requested face verticies list in.
    std::vector<GLuint> baseIndexArray = blockVAOmanager->GetBaseIndexArray(blockModel);
    std::vector<Vertex> baseVertexArray = blockVAOmanager->GetBaseVertexArray(blockModel);
    std::vector<Vertex> vertexArray {};

    // For each requested face
    for (auto& face : _faces) {
        std::vector<GLuint> usedIndicies {};
        for (int i = face*6; i < (face*6)+6; i++) {
            // unique verticies for each face only
            if (std::any_of(usedIndicies.begin(), usedIndicies.end(), [&](int index){
                return index == baseIndexArray[i];
            })) continue;

            // Store Vertex and used Index
            vertexArray.push_back(baseVertexArray[baseIndexArray[i]]);
            usedIndicies.push_back(baseIndexArray[i]);
        }
    }

    return vertexArray;
}

int Block::GetAttributeValue(BLOCKATTRIBUTE _attribute) const {
    switch (_attribute) {
        case BLOCKATTRIBUTE::TRANSPARENT:
            return transparent;

        default:
            return 0;
    }
}







TestBlock::TestBlock(int _variant) {
    blockData = {BLOCKID::TEST, _variant};
    sheet = TEXTURESHEET::TEST16;

    // Set texture origin
    switch (_variant) {
        case 1: origin = {4,2}; break;
        case 2: origin = {5,4}; break;
        case 3: origin = {7,1}; break;
        case 4: origin = {10,2}; break;
        case 5: origin = {13,1}; break;
        default: origin = {1,1};
    }
}