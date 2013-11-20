layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

layout(location = 0) in vec4 aPosition;
layout(location = 2) in vec3 aColor;

out vec3 vPosition;
out vec3 vColor;

void main() {
	vPosition = aPosition.xyz;
    vColor = aColor;
    float eps = 2.0;
    mat4 projection = uProjection;
    projection[2][2] *= (1.0 - 0.0001);
	gl_Position = projection * (mat4(1.0) * aPosition);
}
