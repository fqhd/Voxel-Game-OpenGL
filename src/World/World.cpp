//
// Created by cew05 on 10/07/2024.
//

#include "World.h"

#include "glm/gtc/noise.hpp"
#include "../Blocks/CreateBlock.h"
#include "CreateBiome.h"
#include "../Window.h"
#include "Chunk.h"

World::World() {
    // Create skybox, sun and moon
    skybox = CreateBlock({BLOCKID::AIR, 1});
    sun = CreateBlock({AIR, 2});
    moon = CreateBlock({AIR, 3});

    sunTransformation = Transformation();
    sunTransformation.SetScale({15.0f, 15.0f, 15.0f});
    sunTransformation.UpdateModelMatrix();

    moonTransformation = Transformation();
    moonTransformation.SetScale({12.0f, 12.0f, 12.0f});
    moonTransformation.UpdateModelMatrix();

    GLint uLocation;
    uLocation = glGetUniformLocation(window.GetShader(), "worldAmbients.lightingStrength");
    if (uLocation < 0) printf("location not found [worldAmbients.lightingStrength]\n");
    else glUniform1f(uLocation, 1.0f);

    uLocation = glGetUniformLocation(window.GetShader(), "worldAmbients.minFogDistance");
    if (uLocation < 0) printf("location not found [worldAmbients.minFogDistance]\n");
    else glUniform1f(uLocation, (renderRadius - 1) * chunkSize);

    uLocation = glGetUniformLocation(window.GetShader(), "worldAmbients.maxFogDistance");
    if (uLocation < 0) printf("location not found [worldAmbients.maxFogDistance]\n");
    else glUniform1f(uLocation, renderRadius * chunkSize);

    glEnable(GL_DEPTH_TEST);

    chunkBuilderThread.StartThread();
    chunkMesherThread.StartThread();
    chunkLoaderThread.StartThread();
}

World::~World() {
    chunkBuilderThread.EndThread();
    chunkMesherThread.EndThread();
    chunkLoaderThread.EndThread();
}

void World::Display() const {
    glEnable(GL_BLEND);

    // First draw in the skybox and decorations
    skybox->Display(skyboxTransformation);
    sun->Display(sunTransformation);
    moon->Display(moonTransformation);

    // Draw solid objects
    glEnable(GL_CULL_FACE);
    for (int x = -renderRadius; x < renderRadius; x++) {
        for (int z = -renderRadius; z < renderRadius; z++) {
            auto chunk = GetChunkAtIndex(loadingIndex + glm::ivec2{x, z});

            if (chunk != nullptr) chunk->DisplaySolid();
        }
    }
    glDisable(GL_CULL_FACE);

    // Draw transparent objects
    for (int x = -renderRadius; x < renderRadius; x++) {
        for (int z = -renderRadius; z < renderRadius; z++) {
            auto chunk = GetChunkAtIndex(loadingIndex + glm::ivec2{x, z});

            if (chunk != nullptr) chunk->DisplayTransparent();
        }
    }

}

void World::CheckCulling(const Camera &_camera) {
    displayingChunks = 0;

    for (int chunkX = 0; chunkX < worldSize; chunkX++) {
        for (int chunkZ = 0; chunkZ < worldSize; chunkZ++) {;
            worldChunks[chunkX][chunkZ]->CheckCulling(_camera);
            if (worldChunks[chunkX][chunkZ]->ChunkVisible())
                displayingChunks += 1;
        }
    }

//    printf("DISPLAYING %d / %d CHUNKS\n", displayingChunks, nChunks);
}



/*
 * SKYBOX
 */

void World::SetSkyboxProperties(const Player& player) {
    // Determine max distance for skybox
    std::pair<float, float> minMax = player.GetUsingCamera()->GetMinMaxDistance();
    double maxSqrd = std::pow(minMax.second, 2.0);

    // Set skybox scale
    skyboxTransformation.SetScale({float(sqrt(maxSqrd)), float(sqrt(maxSqrd)), float(sqrt(maxSqrd))});

    // Set skybox position centred on the player
    SetSkyboxPosition(player.GetPosition());
}



/*
 * Updates the positions of elements within the skybox when the player moves
 */

void World::SetSkyboxPosition(glm::vec3 _position) {
    glm::vec3 originFromCentre;

    // Skybox
    originFromCentre = glm::vec3{_position - (skyboxTransformation.GetLocalScale() / 2.0f)};
    originFromCentre.y += skyboxTransformation.GetLocalScale().y;
    skyboxTransformation.SetPosition(originFromCentre);
    skyboxTransformation.UpdateModelMatrix();

    // Sun
    originFromCentre = _position - sunTransformation.GetLocalScale() / 2.0f;
    originFromCentre.y += sunTransformation.GetLocalScale().y;

    glm::mat4 sunAngle = glm::rotate(glm::mat4(1.0f), glm::radians(-((float)worldTime-(7*60)) * 0.125f), dirBack);
    glm::vec3 sunPos{originFromCentre.x, originFromCentre.y, originFromCentre.z};
    glm::vec3 sunOffset = sunAngle * glm::vec4(0.0f, 0.0f, 180.0f, 1.0f);

    sunTransformation.SetPosition(sunPos + sunOffset);
    sunTransformation.UpdateModelMatrix();

    // Moon
    originFromCentre = _position - moonTransformation.GetLocalScale() / 2.0f;
    originFromCentre.y += moonTransformation.GetLocalScale().y;

    glm::vec3 moonPos{originFromCentre.x, originFromCentre.y, originFromCentre.z};
    moonPos += -sunAngle * glm::vec4(0.0f, 0.0f, 180.0f, 1.0f);

    moonTransformation.SetPosition(moonPos);
    moonTransformation.UpdateModelMatrix();
}



/*
 * Continuously updates the world time and calculates the ambient light level from the time of day.
 */

void World::UpdateWorldTime(Uint64 _deltaTicks) {
    worldTicks += _deltaTicks;
    if (worldTicks >= 1000) {
        auto timeTicks = std::div((int)worldTicks, 1000);
        worldTime += timeTicks.quot;
        worldTicks = timeTicks.rem;
    }

    if (worldTime >= 24 * 60) {
        auto daysTime = std::div((int)worldTime, 24 * 60);
        worldDays += daysTime.quot;
        worldTime = daysTime.rem;
    }

    // Ambient lighting
    float min = 0.05f, max = 1.0f;
    int time = (int)worldTime;
    float lightLevel = max;

    if (time >= 6*60 && time < 7*60) {
        lightLevel = ((float)time - 6*60.0f) / ((7*60.0f) - (6*60.0f));
    }
    else if (time <= 19*60 && time > 18*60) {
        lightLevel = 1 - ((float)time - 18*60.0f)/ ((19*60.0f) - (18*60.0f));
    }
    else if ((time < 6*60) || (time > 19*60)) {
        lightLevel = min;
    }

    lightLevel = std::min(lightLevel, max);
    lightLevel = std::max(lightLevel, min);

    // Ambient Lighting
    GLint uLocation;
    uLocation = glGetUniformLocation(window.GetShader(), "worldAmbients.lightingStrength");
    if (uLocation < 0) printf("location not found [worldAmbients.lightingStrength]\n");
    else glUniform1f(uLocation, lightLevel);
}



/*
 * WORLD GENERATION
 * Creates chunk objects in the region around the player, adding their generation and
 * meshing functions into the threads
 */

void World::GenerateLoadedWorld() {
    using namespace std::placeholders;

    // Ensure chunks exist for loading region (and border) area
    ThreadAction createChunkRegion{std::bind(&World::CreateChunk, this, _1, _2), loadingIndex};
    chunkBuilderThread.AddPriorityActionRegion(createChunkRegion, loadRadius);

    // Generate the chunks within the loading region (and not border), this will be done after the chunks are created
    ThreadAction generateChunk{std::bind(&World::GenerateChunk, this, _1, _2), loadingIndex};
    chunkBuilderThread.AddActionRegion(generateChunk, loadRadius);

    // Mesh the chunks within the loading region
    ThreadAction createMesh{std::bind(&World::GenerateChunkMesh, this, _1, _2), loadingIndex};
    chunkMesherThread.AddActionRegion(createMesh, meshRadius);
}



/*
 * Thread-Called function to retrieve chunk data for a given chunk index position.
 */

void World::CreateChunk(const glm::ivec2& _chunkIndex, const glm::vec3& _blockPos) {
    // Chunk object already exists, dont overwrite
    if (GetChunkAtIndex(_chunkIndex) != nullptr) {
        return;
    }

    // Get ChunkData and create the chunk
    ChunkData chunkData = GenerateChunkData(_chunkIndex);
    chunkData.biome = GenerateBiome(GetBiomeIDFromData(chunkData));

    glm::vec3 index{_chunkIndex.x, 0, _chunkIndex.y};
    CreateChunkAtIndex(index, chunkData);
}



/*
 * Obtains chunk data and generates a chunk (base terrain and decorative)
 */

void World::GenerateChunk(const glm::ivec2& _chunkIndex, const glm::vec3& _blockPos) {
    auto st = SDL_GetTicks64();

    auto chunk = GetChunkAtIndex(_chunkIndex);
    if (chunk == nullptr) {
        // if this situation occurs, probably will be multiple visible generation issues with cross-chunk structures
        CreateChunk(_chunkIndex, {0, 0, 0});
        chunk = GetChunkAtIndex(_chunkIndex);
        if (chunk == nullptr) return;
    }

    // Generate the chunk's blocks
    if (!chunk->Generated()) {
        chunk->GenerateChunk();

        auto et = SDL_GetTicks64();

        chunkSumTicksTaken += et - st;
        nChunksCreated++;
        chunkAvgTicksTaken = chunkSumTicksTaken / nChunksCreated;
    }
}


void World::GenerateChunkMesh(const glm::ivec2 &_chunkIndex, const glm::vec3& _blockPos) {
    auto chunk = GetChunkAtIndex(_chunkIndex);

    if (chunk != nullptr && chunk->RegionGenerated() && chunk->NeedsMeshUpdates()) {
        auto st = SDL_GetTicks64();
        chunk->CreateChunkMeshes();

        auto et = SDL_GetTicks64();

        meshSumTicksTaken += et - st;
        nMeshesCreated++;
        meshAvgTicksTaken = meshSumTicksTaken / nMeshesCreated;
    }
    else if (chunk == nullptr || !chunk->RegionGenerated()) {
        // Waiting on the chunk to be made / generate
        ThreadAction meshChunk{std::bind(&World::GenerateChunkMesh, this, std::placeholders::_1, std::placeholders::_2),
                               _chunkIndex};
        chunkMesherThread.AddActions({meshChunk});
    }
}


void World::ManageLoadedChunks(const std::shared_ptr<Chunk>& _currentChunk, const std::shared_ptr<Chunk>& _newChunk) {
    using namespace std::placeholders;

    glm::vec2 oldChunkPos = {_currentChunk->GetIndex().x, _currentChunk->GetIndex().z};

    // hijack the blockPos intended for precision to store a second chunkPos instead
    ThreadAction markUnloaded{std::bind(&World::CheckChunkLoaded, this, _1, _2),
                              oldChunkPos, _newChunk->GetIndex()};
    chunkLoaderThread.AddActionRegion(markUnloaded, loadRadius);
}



/*
 * Enacts on all chunks in the current loaded region and checks if they continue to
 * be loaded in the new loaded region
 */

void World::CheckChunkLoaded(const glm::ivec2 &_currentChunkPos, const glm::vec3 &_newChunkPos) {
    glm::vec3 chunkIndex = {_currentChunkPos.x, 0, _currentChunkPos.y};
    auto chunk = GetChunkAtIndex(chunkIndex);

    int diffX = std::abs(_currentChunkPos.x - (int)_newChunkPos.x);
    int diffZ = std::abs(_currentChunkPos.y - (int)_newChunkPos.z);

    // chunk within load region, update adjacent chunks and ignore
    if (!(diffX > loadRadius || diffZ > loadRadius)) return;

    // chunk is outside of the load radius so unload
    DestroyChunkAtIndex(chunkIndex);
}



float World::GenerateBlockHeight(glm::vec2 _blockPos) {
    float height;

    /*
     * PRIMARY TERRAIN LEVELS
     */

    // Seabed / ContinentBed Generation
    float continentiality = glm::simplex(glm::vec2( _blockPos.x / 2000.0, _blockPos.y / 2000.0));
    continentiality = 1;

    // Primary Noise based around waterlevel
    float baseHeight = glm::simplex(glm::vec2( _blockPos.x / 128.0, _blockPos.y / 128.0));
    baseHeight *= 5;

    // Set the base height of the block
    height = baseHeight * continentiality + MINBLOCKHEIGHT;

    // Secondary base level noise applied
    float secondHeight = glm::simplex(glm::vec2( _blockPos.x / 16.0, _blockPos.y / 16.0));
    secondHeight *= 1;
    height += secondHeight;

    /*
     *  MOUNTAIN GENERATION
     */

    // Produce noise values for mountain
    float peakHeight = glm::simplex(glm::vec2( _blockPos.x / 128.0, _blockPos.y / 128.0));
    peakHeight = (peakHeight + 1) / 2;
    peakHeight *= 128.0;

    // Determine if mountain should generate
    float areaHeight = glm::simplex(glm::vec2( _blockPos.x / 500.0, _blockPos.y / 500.0));
    areaHeight = (areaHeight + 1) / 2;

    float mountainFreq = 20; // increase to reduce number of mountains
    height += peakHeight * std::pow(areaHeight, mountainFreq);


    return std::round(height);
}

float World::GenerateBlockHeat(glm::vec3 _blockPos) {
    float heat = glm::simplex(glm::vec2(_blockPos.x / 64.0, _blockPos.z / 64.0));
    heat = (heat + 1) / 2;
    heat *= 20;
    heat += BASETEMP;

    // Relate heat to height (higher = colder)
    heat -= (_blockPos.y / MAXBLOCKHEIGHT) * 10;

    return heat;
}

float World::GenerateBlockVegetation(glm::vec3 _blockPos, float _heat) {
    float grassDensity = glm::simplex(glm::vec2( _blockPos.x / 8.0, _blockPos.z / 8.0));
    grassDensity = (grassDensity + 1) / 2;

    float treeDensity = glm::simplex(glm::vec2( _blockPos.x / 1.0, _blockPos.z / 1.0));
    treeDensity = (treeDensity + 1) / 2;
    treeDensity = std::pow(treeDensity, 10.0f);

    return grassDensity + treeDensity;
}

// Generate the height and temp maps for the given chunk starting pos
ChunkData World::GenerateChunkData(glm::vec2 _chunkPosition) {
    int chunkX = (int)_chunkPosition.x * chunkSize;
    int chunkZ = (int)_chunkPosition.y * chunkSize;
    ChunkData chunkData {};

    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            int bx = chunkX + x, bz = chunkZ + z;

            // Get the toplevel (highest y) of the given x z position
            float height = GenerateBlockHeight({bx, bz});
            chunkData.heightMap[x + z * chunkSize] = height;

            // Get the heat value for the block
            float heat = GenerateBlockHeat({bx,height,bz});
            chunkData.heatMap[x + z * chunkSize] = heat;

            // Create block vegetation value (relate to heat, height)
            float vegetation = GenerateBlockVegetation({bx, height, bz}, heat);
            chunkData.plantMap[x + z * chunkSize] = vegetation;
        }
    }

    return chunkData;
}



/*
 * Ensure that a biome of type BIOMEID has been generated for the world
 */

Biome* World::GenerateBiome(BIOMEID _biomeID) {
    // If the biome has been generated before then exit
    for (auto& uniqueBiome : uniqueBiomes) {
        if (uniqueBiome->GetBiomeID() == _biomeID) return uniqueBiome.get();
    }

    // Else required to create a new unique biome
    uniqueBiomes.emplace_back(CreateBiome(_biomeID));
    return uniqueBiomes.back().get();
}


void World::SetLoadingOrigin(const glm::vec3 &_origin) {
    loadingIndex = {_origin.x, _origin.z};  // index of player's centre chunk
}

void World::BindChunks() const {
    for (int x = -meshRadius; x < meshRadius; x++) {
        for (int z = -meshRadius; z < meshRadius; z++) {
            auto chunk = GetChunkAtIndex(loadingIndex + glm::ivec2{x, z});

            if (chunk != nullptr && chunk->UnboundMeshChanges()) {
                chunk->BindChunkMeshes();
            }
        }
    }
}






std::shared_ptr<Chunk> World::GetChunkAtBlockPosition(glm::vec3 _blockPos) const {
    glm::vec3 index = _blockPos / (float)chunkSize;
    return GetChunkAtIndex(index);
}

std::shared_ptr<Chunk> World::GetChunkAtIndex(glm::vec2 _chunkIndex) const {
    return GetChunkAtIndex({_chunkIndex.x, 0, _chunkIndex.y});
}

std::shared_ptr<Chunk> World::GetChunkAtIndex(glm::vec3 _chunkIndex) const {
    glm::vec3 index = _chunkIndex + glm::vec3{1000, 0, 1000}; // centre of the worlds chunks

    if (index.x < 0 || index.x >= 2000) return nullptr;
    if (index.z < 0 || index.z >= 2000) return nullptr;

    // permits multiple fetch requests for chunk
    std::shared_lock lock(worldChunks[(int)index.x][(int)index.z].chunkLock);
    std::shared_ptr<Chunk> chunkPtr = worldChunks[(int)index.x][(int)index.z].chunkPtr;

    return chunkPtr;
}


THREAD_ACTION_RESULT World::DestroyChunkAtIndex(glm::vec3 _chunkIndex) {
    glm::vec3 index = _chunkIndex + glm::vec3{1000, 0, 1000};

    if (index.x < 0 || index.x >= 2000) return ThreadAction::FAIL;
    if (index.y < 0 || index.y >= 2000) return ThreadAction::FAIL;

    // only one thread may destroy the chunk, and only when no fetch requests are active
    std::unique_lock lock(worldChunks[(int)index.x][(int)index.z].chunkLock, std::try_to_lock);
    if (!lock.owns_lock()) {
        // Chunk is busy right now so lock failed. Return it into the list
        return ThreadAction::RETRY;
    }

    // owns lock, destroy chunk
    worldChunks[(int)index.x][(int)index.z].chunkPtr.reset();
    return ThreadAction::OK;
}



THREAD_ACTION_RESULT World::CreateChunkAtIndex(glm::vec3 _chunkIndex, ChunkData _chunkData) {
    glm::vec2 index = {_chunkIndex.x + 1000, _chunkIndex.z + 1000};

    if (index.x < 0 || index.x >= 2000) return ThreadAction::FAIL;
    if (index.y < 0 || index.y >= 2000) return ThreadAction::FAIL;

    // only one thread may create the chunk, and only when no fetch requests are active
    std::unique_lock lock(worldChunks[(int)index.x][(int)index.y].chunkLock, std::try_to_lock);
    if (!lock.owns_lock()) {
        // Chunk is busy right now so lock failed. Return it into the list
        return ThreadAction::RETRY;
    }

    // owns lock, create chunk
    worldChunks[(int)index.x][(int)index.y] = std::make_unique<Chunk>(_chunkIndex, _chunkData);
    return ThreadAction::OK;
}




Biome* World::GetBiome(BIOMEID _biomeID) {
    // Fetch biome
    for (auto& uniqueBiome : uniqueBiomes) {
        if (uniqueBiome->GetBiomeID() == _biomeID) return uniqueBiome.get();
    }

    // Biome did not exist?
    uniqueBiomes.emplace_back(CreateBiome(_biomeID));
    return uniqueBiomes.back().get();
}


ChunkThreads* World::GetThread(THREAD _thread) {
    switch (_thread) {
        case THREAD::CHUNKBUILDING:
            return &chunkBuilderThread;

        case THREAD::CHUNKMESHING:
            return &chunkMesherThread;

        case THREAD::CHUNKLOADING:
            return &chunkLoaderThread;

        case THREAD::CHUNKLIGHTING:
            return &chunkLighterThread;

        default:
            return nullptr;
    }
}