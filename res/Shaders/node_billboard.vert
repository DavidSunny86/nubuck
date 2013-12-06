layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

attribute(0) vec4 aPosition;
attribute(2) vec4 aColor;
attribute(3) vec2 aTexCoord0;

varying vec3 vPosition;
varying vec4 vColor;
varying vec2 vTexCoord0;

void main() {
	vPosition = aPosition.xyz;
    vColor = aColor;
	vTexCoord0 = aTexCoord0;
	gl_Position = uProjection * (mat4(1.0) * aPosition);
}
