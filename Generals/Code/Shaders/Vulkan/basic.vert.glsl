#version 450

// Vertex input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec4 inColor;

// Vertex output
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragColor;
layout(location = 3) out vec3 fragWorldPos;

// Uniform buffer
layout(binding = 0) uniform UniformBufferObject {
    mat4 world;
    mat4 view;
    mat4 projection;
    vec4 lights[4];  // Simplified light data
    vec4 fog_color;
    float fog_start;
    float fog_end;
    int fog_enabled;
    int light_enables[4];
} ubo;

void main() {
    // Transform position to world space
    vec4 worldPos = ubo.world * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    // Transform to clip space
    gl_Position = ubo.projection * ubo.view * worldPos;

    // Transform normal to world space
    fragNormal = mat3(ubo.world) * inNormal;

    // Pass through texture coordinates and color
    fragTexCoord = inTexCoord;
    fragColor = inColor;
}
