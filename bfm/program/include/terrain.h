#if ! defined(__TERRAIN_TILE__)
#define __TERRAIN_TILE__

#include <glm/glm.hpp>

void TerrainInitialise(float radius);
void TerrainDraw(const glm::mat4& cameraViewProjectionMatrix, Effect& effect);

#endif // __TERRAIN_TILE__
