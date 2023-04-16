#version 410 core

in vec3 textureCoordinates;
in vec3 fPosition;
out vec4 fColor;

uniform mat4 model;
uniform mat4 view;
uniform samplerCube skybox;

float computeFog()
{
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	float fogDensity = 0.05f;
 	float fragmentDistance = length(fPosEye);
 	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 	return clamp(fogFactor, 0.0f, 1.0f);
}


void main() {
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 0.8f);
    vec4 color = texture(skybox, textureCoordinates);
	fColor = mix(fogColor, color, fogFactor);
}
