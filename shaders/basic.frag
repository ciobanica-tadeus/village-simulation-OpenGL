#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragmentPosEye;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
// fog
uniform int isFog;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));
    //compute eye coordinates for normals
    vec3 normalEye = normalize(normalMatrix * fNormal);
    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute the vertex position in eye coordinates
    vec4 vertPosEye = view * model * vec4(fPosition, 1.0f);
    //compute the view (Eye) direction (in eye coordinates, the camera is at the origin)
    vec3 viewDir = normalize(- vertPosEye.xyz);
    ///////----------------------------------------------- PHONG
     //compute view direction
    vec3 viewDirN = normalize(vec3(0.0f) - fragmentPosEye.xyz);
    vec3 halfVector = normalize	(lightDirN + viewDirN);
    
    //compute the light's reflection (the reflect function requires a direction pointing towards the vertex, not away from it
    vec3 reflectDir = normalize(reflect(-lightDir, normalEye));

    //compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
    /////------------------------------------------
    // //compute specular light
    // float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    // specular = specularStrength * specCoeff * lightColor;
}

float computeFog()
{
 float fogDensity = 0.02f;
 float fragmentDistance = length(fragmentPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 
 return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
    computeDirLight();

    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.1f){
        discard;
    }

    ambient *= colorFromTexture.rgb;
    diffuse *= colorFromTexture.rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    //compute final vertex color
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
	float fogFactor = computeFog();
	if (isFog == 1){
		vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		fColor = mix(fogColor, vec4(color,1.0f), fogFactor);
    }
	else 
		fColor = vec4(color, 1.0f);
	
	
}
