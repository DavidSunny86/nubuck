layout(triangles) in;
layout(line_strip) out;
layout(max_vertices = 4) out;

uniform mat4 uProjection;
uniform mat4 uTransform;

void main() {
	mat4 mvp = uProjection * uTransform;
	
	vec4 v0 = mvp * gl_in[0].gl_Position;
	
	gl_Position = v0;
	EmitVertex();
	
	gl_Position = mvp * gl_in[1].gl_Position;
	EmitVertex();
	
	gl_Position = mvp * gl_in[2].gl_Position;
	EmitVertex();
	
	gl_Position = v0;
	EmitVertex();
	
	EndPrimitive();
}