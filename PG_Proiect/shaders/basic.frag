#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightLaserDir;
uniform vec3 lightLaserDir2;
uniform vec3 lightColor;
uniform vec3 lightLaserColor;

uniform vec3 neonPos[5];
uniform vec3 neonColor[5];

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
vec3 diffuse;
vec3 specular;
vec3 pointAmbient;
vec3 pointDiffuse;
vec3 pointSpecular;

vec4 fPosEye;

float specularStrength = 0.5f;
float shininess = 32.0f;
float ambientStrength = 0.2f;

// 3250 1.0 0.0014 0.000007
float constant = 1.0f;
float linear = 0.045f;
float quadratic = 0.0075f;

float bulbIntensity = 5.0f;
uniform float spotIntensity[5] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
uniform float lightIntensity = 1.0f;

void computeLightComponents()
{		
    //compute eye space coordinates
    fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor * lightIntensity;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * lightIntensity;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor * lightIntensity;
	
	// compute pointlight 
	lightDirN = normalize(lightLaserDir - fPosition.xyz);
	float dist = length(lightLaserDir - fPosition.xyz);
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist)) * bulbIntensity;
	ambient += att * ambientStrength * lightLaserColor;
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightLaserColor;

	lightDirN = normalize(lightLaserDir2 - fPosition.xyz);
	dist = length(lightLaserDir2 - fPosition.xyz);
	att = 1.0f / (constant + linear * dist + quadratic * (dist * dist)) * bulbIntensity;
	ambient += att * ambientStrength * lightLaserColor;
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightLaserColor;

	// compute spotlight 
	for (int i = 0; i < 5; i++) {
		lightDirN = normalize(neonPos[i] - fPosition.xyz);
		dist = length(neonPos[i] - fPosition.xyz);
		att = 1.0f / (constant + linear * dist + quadratic * (dist * dist)) * spotIntensity[i];
		
		ambient += att * ambientStrength * neonColor[i];
		diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * neonColor[i];
	}
}

float computeShadow() {
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5f + 0.5f;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	return shadow;
}

float computeFog() {
	float fogDensity = 0.01f;
 	float fragmentDistance = length(fPosEye);
 	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {
    //computeDirLight();
	//computeLaserLight();
	
	computeLightComponents();
	vec4 baseColor=texture(diffuseTexture, fTexCoords);
	if(baseColor.a<0.1f)
		discard;
	ambient *= baseColor.rgb;
	diffuse *= baseColor.rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;
	
	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	fColor = vec4((mix(fogColor, vec4(color, 1.0f), fogFactor)).rgb, baseColor.a);
}
