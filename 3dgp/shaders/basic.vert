// VERTEX SHADER

#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colours
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;

in vec3 aVertex;
in vec3 aNormal;

out vec4 color;
out vec4 position;
out vec3 normal;

void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate light - start with pitch black
	color = vec4(0);

	// temporary light and material calculation
	// DELETE THE FOLLOWING THREE LINES BEFORE PROCEEDING!
	vec3 L = normalize(mat3(matrixView) * vec3(-1.0, 0.5, 1.0));
	float NdotL = max(dot(normal, L), 0.0);
	color += vec4(materialDiffuse, 1) * NdotL + vec4(materialAmbient, 1);
}
