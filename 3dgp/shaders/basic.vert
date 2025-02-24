#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colors
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;

// Light declarations
struct AMBIENT {
    vec3 color;
};

uniform AMBIENT lightAmbient;

uniform float fogDensity;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;
in vec3 aTangent;
in vec3 aBiTangent;

out vec4 ambientColor;
out vec3 fragNormal;
out vec3 fragPosition;
out vec2 fragTexCoord;
out mat3 matrixTangent;
out float fogFactor;

vec4 AmbientLight(AMBIENT light) {
    // Calculate Ambient Light
    return vec4(materialAmbient * light.color, 1);
}

void main(void) 
{
    // calculate position
    vec4 position = matrixModelView * vec4(aVertex, 1.0);
    gl_Position = matrixProjection * position;

    // pass the normal and position to the fragment shader
    fragNormal = normalize(mat3(matrixModelView) * aNormal);
    fragPosition = vec3(position);

    // pass the texture coordinates to the fragment shader
    fragTexCoord = aTexCoord;

    // calculate tangent local system transformation
    vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
    vec3 biTangent = normalize(mat3(matrixModelView) * aBiTangent);
    matrixTangent = mat3(tangent, biTangent, fragNormal);

    // calculate ambient light
    ambientColor = AmbientLight(lightAmbient);

    // calculate fog factor
    fogFactor = exp2(-fogDensity * length(position));
}
