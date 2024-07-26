//
// Created by cew05 on 26/07/2024.
//

#ifndef UNTITLED7_CREATEBLOCK_H
#define UNTITLED7_CREATEBLOCK_H

#include "NaturalBlocks.h"

// Create a new block instance from a particular ID and Variant upon request
inline std::unique_ptr<Block> CreateBlock(BLOCKID _id, int _variant = 0) {
    std::unique_ptr<Block> newBlock {};

    switch (_id) {
        case BLOCKID::GRASS:
            newBlock = std::make_unique<Grass>(_variant);
            break;

        case BLOCKID::DIRT:
            newBlock = std::make_unique<Dirt>(_variant);
            break;

        case BLOCKID::STONE:
            newBlock = std::make_unique<Stone>(_variant);
            break;

        case BLOCKID::WATER:
            newBlock = std::make_unique<Water>(_variant);
            break;

        default:
            newBlock = std::make_unique<Air>();
    }

    return newBlock;
}

#endif //UNTITLED7_CREATEBLOCK_H
