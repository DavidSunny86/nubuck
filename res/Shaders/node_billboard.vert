layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

attribute(0) vec4 aPosition;
attribute(1) vec3 aNormal;
attribute(2) vec4 aColor;

varying vec3    vPosition;
varying vec4    vColor;
varying vec2    vTexCoord0;
varying float   vRadius;

void main() {
    vColor = aColor;
	vTexCoord0 = aNormal.xy;
    vRadius = aNormal.z;
    vec4 off = vec4(vRadius * vTexCoord0, 0.0, 0.0);
    vec4 pos = uTransform * aPosition + off;
    vPosition = pos.xyz;
	gl_Position = uProjection * pos;
}
