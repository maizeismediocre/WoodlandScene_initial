#version 330

in vec4 ambientColor;
in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;

out vec4 outColor;

// Uniforms for directional light
struct DIRECTIONAL {
    vec3 direction;
    vec3 diffuse;
};

uniform DIRECTIONAL lightDir;

// Uniforms for material colors
uniform vec3 materialDiffuse;

// Uniform for texture
uniform sampler2D textureSampler;

vec4 DirectionalLight(DIRECTIONAL light, vec3 normal) {
    // Calculate Directional Light
    vec4 color = vec4(0, 0, 0, 0);

    vec3 L = normalize(light.direction);
    float NdotL = dot(normal, L);

    color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);

    return color;
}

void main(void) 
{
    // Normalize the normal
    vec3 norm = normalize(fragNormal);

    // Calculate the directional light component
    vec4 directionalColor = DirectionalLight(lightDir, norm);

    // Sample the texture
    vec4 texColor = texture(textureSampler, fragTexCoord);

    // Combine the ambient, directional components, and texture color
    vec3 result = (ambientColor.rgb + directionalColor.rgb) * texColor.rgb;

    // Output the final color with alpha
    outColor = vec4(result, texColor.a);
}
