#version 450

// Fragment input (from vertex shader)
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragColor;
layout(location = 3) in vec3 fragWorldPos;

// Fragment output
layout(location = 0) out vec4 outColor;

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

// Texture sampler
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    // Sample texture
    vec4 texColor = texture(texSampler, fragTexCoord);

    // Combine with vertex color
    vec4 baseColor = texColor * fragColor;

    // Simple lighting (directional light from above)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.2);

    // Apply lighting
    vec3 litColor = baseColor.rgb * diffuse;

    // Apply fog if enabled
    if (ubo.fog_enabled != 0) {
        vec3 cameraPos = vec3(ubo.view[3][0], ubo.view[3][1], ubo.view[3][2]);
        float dist = length(fragWorldPos - cameraPos);
        float fogFactor = clamp((ubo.fog_end - dist) / (ubo.fog_end - ubo.fog_start), 0.0, 1.0);
        litColor = mix(ubo.fog_color.rgb, litColor, fogFactor);
    }

    // Output final color
    outColor = vec4(litColor, baseColor.a);
}
