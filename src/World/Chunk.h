//
// Created by cew05 on 11/07/2024.
//

#ifndef UNTITLED7_CHUNK_H
#define UNTITLED7_CHUNK_H

#include <vector>
#include <memory>

#include "../Blocks/NaturalBlocks.h"
#include "../BlockModels/ModelTransformations.h"
#include "../BlockModels/MaterialMesh.h"

#include "WorldGenConsts.h"
#include "Biome.h"

//static const int chunkSize = 16; // must be power of 2 for subchunk division ie 2, 4, 8, 16 | 32, 64, 128, ... ( too big)
//static const int chunkArea = chunkSize * chunkSize;
//static const int chunkVolume = chunkArea * chunkSize;

class Chunk;

// Used to construct a tree of chunks to manage object culling and display
// Either the subchunk vector is populated with further, smaller subchunks, or the Cube object is instantiated
// A SubChunk with a valid cube object is considered to be a terminating point of the subchunk tree.

class ChunkNode {
    protected:
        // Tree objects
        Chunk* rootChunk {};
        std::vector<std::unique_ptr<ChunkNode>> subNodes {};
        Block* nodeBlock {};

        // Positioning
        float scale = 1;
        glm::vec3 position {0,0,0};

        // Merge nodes
        bool isSingleType = true;

    public:
        ChunkNode(Block* _nodeBlock, glm::vec3  _blockPos, Chunk* _root);
        ChunkNode(std::vector<std::unique_ptr<ChunkNode>> _subNodes, Chunk* _root);
        ~ChunkNode();

        // Node material mesh creation
        void UpdateMaterialMesh(MaterialMesh* _mesh);
};





// REDEF LONG VAR TYPES
typedef std::array<std::array<std::array<std::unique_ptr<ChunkNode>, chunkSize>, chunkSize>, chunkSize> chunkNodeArray;

// CHUNKDATA STRUCT
struct ChunkData {
    Biome* biome {};
    ChunkDataMap heightMap {};
    ChunkDataMap heatMap {};
};

/*
 * Parent node to ChunkNodes
 */

class Chunk {
    private:
        // Chunk Display
        bool inCamera = true;
        std::vector<std::unique_ptr<MaterialMesh>> blockMeshes {};

        // Chunk Culling and positioning
        std::unique_ptr<BoxBounds> boxBounds {};
        std::unique_ptr<Transformation> chunkTransformation = std::make_unique<Transformation>();
        glm::vec3 chunkPosition {0,0,0};

        // Chunk tree
        std::unique_ptr<ChunkNode> rootNode {};
        std::array<Chunk*, 10> adjacentChunks {};

        // Block Data
        std::vector<std::pair<std::unique_ptr<Block>, int>> uniqueBlocks {}; // block, count
        chunkTerrainArray terrain {};
        bool generated = false;

        // Biome Data and chunk Generation info
        ChunkData chunkData;

    public:
        Chunk(const glm::vec3& _chunkPosition, ChunkData _chunkData);
        ~Chunk();

        // Display
        void CreateBlockMeshes();
        void DisplaySolid();
        void DisplayTransparent();

        // Object Culling
        void CheckCulling(const Camera& _camera);
        [[nodiscard]] std::vector<BLOCKFACE> GetHiddenFaces(glm::vec3 _position) const;
        [[nodiscard]] std::vector<BLOCKFACE> GetShowingFaces(glm::vec3 _position) const;

        // Chunk Generation
        void GenerateChunk();
        chunkNodeArray CreateTerrain();
        void CreateNodeTree(chunkNodeArray _chunkNodes);
        void SetAdjacentChunks(const std::array<Chunk*, 10>& _chunks);

        // Getters
        [[nodiscard]] Block* GetBlockAtPosition(glm::vec3 _position, int _depth) const;
        [[nodiscard]] float GetTopLevelAtPosition(glm::vec3 _position, float _radius) const;
        [[nodiscard]] Block* GetBlockFromData(BlockType _blockData) const;
        [[nodiscard]] glm::vec3 GetPosition() const { return chunkPosition; }
};


#endif //UNTITLED7_CHUNK_H
