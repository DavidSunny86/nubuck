layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

layout(std140) uniform UniformsLights {
    vec3    uLightVec[3];
    vec4    uLightDiffuseColor[3];
    float   uLambertFactor;
    float   uShininess;
    float   uRoughness;
    float   uFresnel;
    float   uDiffuseReflectance;
    int     uLightingModel;
};

layout(std140) uniform UniformsSkeleton {
    vec4    uColor;
    float   uNodeSize;
};

#include <Shaders\lighting.glsl>

varying vec3    vPosition;
varying vec4    vColor;
varying vec2    vTexCoord0;
varying float   vRadius;

void main() {
	float clip = dot(vTexCoord0, vTexCoord0);
	if(clip <= 1.0) {
		float n = sqrt(1.0 - clip);
		vec3 position = vPosition;
		position.z += vRadius * n;

        vec3 view = -normalize(position);
		vec3 normal = normalize(vec3(vTexCoord0, n));

        gl_FragColor = vec4(lighting(normal, view, vColor.rgb), 1.0);
		
		vec4 proj = uProjection * vec4(position, 1.0);
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
	}
	else discard;
}
