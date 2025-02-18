#version 330

in vec4 ambientColor;
in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;

out vec4 outColor;

// Uniforms for directional light
uniform vec3 lightDirection;
uniform vec3 lightColor;

// Uniform for texture
uniform sampler2D textureSampler;

void main(void) 
{
    // Normalize the normal
    vec3 norm = normalize(fragNormal);

    // Calculate the diffuse component
    float diff = max(dot(norm, normalize(lightDirection)), 0.0);
    vec3 diffuse = diff * lightColor;

    // Sample the texture
    vec4 texColor = texture(textureSampler, fragTexCoord);

    // Combine the ambient, diffuse components, and texture color
    vec3 result = (ambientColor.rgb + diffuse) * texColor.rgb;

    // Output the final color with alpha
    outColor = vec4(result, texColor.a);
}