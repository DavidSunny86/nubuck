uniform mat4 uTransform;
uniform mat4 uWorldToObject;
out vec4 fragColor;

in vec4 vPosition;
in mat4 vWorldToObject;
in mat4 vObjectToWorld;

void main() {
    float r = 1.0;
    vec4 s = uWorldToObject * inverse(uTransform) * vec4(0.0, 0.0, 0.0, 1.0);
    vec4 v = normalize(vPosition - s);
    float f = (s.x * v.x + s.y * v.y) / (v.x * v.x + v.y * v.y);
    float d = sqrt((r * r - (s.x * s.x + s.y * s.y)) / (v.x * v.x + v.y * v.y) + f * f);
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    if(0 <= d) {
        vec4 p = s + (-d - f) * v;
        if(p.z * p.z > 25) p = s + ( d - f) * v;
        if(p.z * p.z > 25) discard;
        
        vec3 normal = vec3(p.x, p.y, 0.0);
        vec3 lightDir = (uWorldToObject * inverse(uTransform) * vec4(0.0, 0.0, 1.0, 0.0)).xyz;
        float d = clamp(dot(normal, normalize(lightDir)), 0.0, 1.0);
        fragColor = d * vec4(1.0, 1.0, 1.0, 1.0);
    } else discard;
}
