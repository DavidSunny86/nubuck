// lighting.glsl

#define BRDF_BLINN_PHONG    1
#define BRDF_COOK_TORRANCE  2

vec3 brdf_lambert(vec3 diffuse) {
    return diffuse / uLambertFactor;
}

// light, view must be normalized
vec3 brdf_blinn_phong(vec3 light, vec3 view, vec3 normal, float shininess)
{
    float h = pow(clamp(dot(normal, normalize(view + light)), 0.0, 1.0), shininess);
    h /= dot(normal, light);
    return vec3(h);
}

// from http://ruh.li/GraphicsCookTorrance.html
vec3 brdf_cook_torrance(vec3 light, vec3 view, vec3 normal, float roughness, float fresnel, float diffuseReflectance) {
    float NdotL = max(dot(normal, light), 0.0);
    
    float specular = 0.0;
    if(NdotL > 0.0)
    {

        // calculate intermediary values
        vec3 halfVector = normalize(light + view);
        float NdotH = max(dot(normal, halfVector), 0.0); 
        float NdotV = max(dot(normal, view), 0.0); // note: this could also be NdotL, which is the same value
        float VdotH = max(dot(view, halfVector), 0.0);
        float mSquared = roughness * roughness;
        
        // geometric attenuation
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * NdotL) / VdotH;
        float G = min(1.0, min(g1, g2));
     
        // roughness (or: microfacet distribution function)
        // beckmann distribution function
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float D = r1 * exp(r2);
        
        // fresnel
        // Schlick approximation
        float F = pow(1.0 - VdotH, 5.0);
        F *= (1.0 - fresnel);
        F += fresnel;
        
        specular = (F * G * D) / (NdotV * NdotL * 3.14);
    }
    
    float k = diffuseReflectance;
    return vec3(1.0) * (k + specular * (1.0 - k));
}

vec3 lighting(vec3 normal, vec3 view, vec3 matDiffuse) {
    vec3 diff = vec3(0.0);
    vec3 spec = vec3(0.0);

    for(int i = 0; i < 3; ++i) {
        vec3 light = normalize(uLightVec[i]);

        float d = clamp(dot(normal, light), 0.0, 1.0);
        diff += brdf_lambert(matDiffuse) * uLightDiffuseColor[i].rgb * d;

        vec3 s_brdf = vec3(0.0);
        if(BRDF_BLINN_PHONG == uLightingModel) {
            s_brdf = brdf_blinn_phong(light, view, normal, uShininess);
        }
        else if(BRDF_COOK_TORRANCE == uLightingModel) {
            s_brdf = brdf_cook_torrance(light, view, normal, uRoughness, uFresnel, uDiffuseReflectance);
        }
        spec += s_brdf * uLightDiffuseColor[i].rgb * d;
    }

    return diff + spec;
}
