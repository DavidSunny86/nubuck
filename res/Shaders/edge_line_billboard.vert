layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

attribute(0) vec4 aPosition;
attribute(2) vec3 aColor;

varying vec3 vPosition;
varying vec3 vColor;

void main() {
	vPosition = aPosition.xyz;
    vColor = aColor;
    float eps = 2.0;
	gl_Position = uProjection * (mat4(1.0) * aPosition);
}
