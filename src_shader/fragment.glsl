#version 410 core

// PIPE IN
in float v_vertexDistanceFromCamera;
in float v_vertexLightLevel;
in vec2 v_vertexTextureCoord;
in vec4 v_vertexTextureColorOverride;

// UNIFORMS
uniform sampler2D tex0;
uniform int uCanFog = 1;

layout(std140) uniform struct WorldAmbients {
    float lightingStrength;
    float minFogDistance;
    float maxFogDistance;
} worldAmbients;


// PIPE OUT
out vec4 color;





float FogFactor() {
    if (v_vertexDistanceFromCamera <= worldAmbients.minFogDistance) return 0.0f;
    else if (v_vertexDistanceFromCamera >= worldAmbients.maxFogDistance) return 1.0f;

    // match minDist -> dist -> maxDist to 0.0f -> x -> 1.0f
    else return v_vertexDistanceFromCamera - worldAmbients.minFogDistance / worldAmbients.maxFogDistance - worldAmbients.minFogDistance;
}

void main() {
    // Fetch block texture + override color
    color = texture(tex0, v_vertexTextureCoord);
    if (v_vertexTextureColorOverride.a > 0) {
        color = v_vertexTextureColorOverride;
    }

    // if the color has no alpha, discard it and prevent it blocking other pixels behind it
    if (color.a <= 0.0f) discard;

    // Fog
    if (uCanFog == 1) {
        color.a = min(1-FogFactor(), color.a);
    }

    // Apply ambient lighting
    color = vec4(vec3(color.r, color.g, color.b) * worldAmbients.lightingStrength, color.a);
    color = vec4(color.rgb * v_vertexLightLevel, color.a);
}