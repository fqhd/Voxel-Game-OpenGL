//
// Created by cew05 on 19/08/2024.
//

#include "NaturalBlocks.h"

Leaves::Leaves(int _variant) {
    blockData = {BLOCKID::LEAVES, _variant};
    sheet = TEXTURESHEET::NATURAL;

    if (_variant == 0) {
        origin = {1,1};
    }
    if (_variant == 1) {
        origin = {4, 2};
        transparent = 1;
    }

    generationPriority = 1;
}

Wood::Wood(int _variant) {
    blockData = {BLOCKID::WOOD, _variant};
    sheet = TEXTURESHEET::NATURAL;

    if (_variant == 0) {
        origin = {7,1};
    }

    transparent = 0;
}