uniform float uEdgeRadiusSq;

out vec4 fragColor;

// in edge's local space
in mat4     vObjectToWindow; 
in vec4     vPosition;
in float    vHalfHeightSq;
in vec4     vEyePos;
in vec3     vLightDir;

void main() {
    vec4 s = vEyePos;
    vec4 v = normalize(vPosition - s);
    float f = (s.x * v.x + s.y * v.y) / (v.x * v.x + v.y * v.y);
    float d = sqrt((uEdgeRadiusSq - (s.x * s.x + s.y * s.y)) / (v.x * v.x + v.y * v.y) + f * f);
    if(0 <= d) {
        vec4 p = s + (-d - f) * v;
        if(p.z * p.z > vHalfHeightSq) p = s + ( d - f) * v;
        if(p.z * p.z > vHalfHeightSq) discard;
        
        vec3 normal = normalize(vec3(p.x, p.y, 0.0));
        float d = clamp(dot(normal, normalize(vLightDir)), 0.0, 1.0);
        fragColor = d * vec4(1.0, 1.0, 1.0, 1.0);

		vec4 proj = vObjectToWindow * p;
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
    } else discard;
}
