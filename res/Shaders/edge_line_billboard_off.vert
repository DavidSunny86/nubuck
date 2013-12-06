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
    mat4 projection = uProjection;
    projection[2][2] *= (1.0 - 0.0001);
	gl_Position = projection * (mat4(1.0) * aPosition);
}
