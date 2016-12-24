#version 330
// fragment shader for phong-model lighting with a single directional light source.
// TODO: extend this shader to
//  - support diffuse textures
//  - support shadow mapping

in vec4 var_Color;
in vec3 var_Normal;
in vec3 var_Position;

uniform vec3 camPos;

uniform vec3 diffColor;
uniform vec3 specColor;
uniform vec3 ambientColor;
uniform float shininess;
uniform float alpha;

// TODO add more uniforms
uniform sampler2D diffuseTex;
uniform sampler2D shadowTex;
uniform mat4 light_VP;

uniform vec3 lightPos;
uniform vec3 lightDiff;

layout(location=0) out vec4 out_Color;

vec4 blinn_phong(vec3 kd) {
    // Implement Blinn-Phong Shading Model
    // 1. Convert everything to world space
    //    and normalize directions
    vec4 pos_world = vec4(var_Position, 1);
    vec3 normal_world = normalize(var_Normal);
    pos_world /= pos_world.w;
    vec3 light_dir = normalize(lightPos);
    vec3 cam_dir = camPos - pos_world.xyz;
    cam_dir = normalize(cam_dir);

    // 2. Compute Diffuse Contribution
    float ndotl = max(dot(normal_world, light_dir), 0.0);
    vec3 diffContrib = lightDiff * kd * ndotl;

    // 3. Compute Specular Contribution
    vec3 R = reflect( -light_dir, normal_world );
    float eyedotr = max(dot(cam_dir, R), 0.0);
    vec3 specContrib = pow(eyedotr, shininess) *
                       specColor * lightDiff;

    // 4. Add specular and diffuse contributions
    return  + vec4(diffContrib + specContrib, alpha);
}

vec3 ndc2tex(vec3 ndc) {
    return vec3((ndc.x + 1.0) / 2.0, (ndc.y + 1.0) / 2.0, (ndc.z + 1.0) / 2.0);
}

vec2 bia(vec2 tex, vec2 bias) {
    vec2 result = tex + bias;
    if (result.x < 0) {
        result.x = 0;
    } else if (result.x > 1) {
        result.x = 1;
    }
    if (result.y < 0) {
        result.y = 0;
    } else if (result.y > 1) {
        result.y = 1;
    }
    return result;
}

void main () {
	// TODO implement texture mapping here
	// TODO implement shadow mapping here
	vec3 kd = texture(diffuseTex, var_Color.xy).xyz;
	vec4 x_ndc = light_VP * vec4(var_Position, 1);
	vec3 x_tex = ndc2tex(x_ndc.xyz);
	float occluder_depth = texture(shadowTex, x_tex.xy).x;
	float count = 25.0;
    for (int i = -2; i < 3; ++i) {
        for (int j = -2; j < 3; ++j) {
            vec2 bias = vec2(i / 4096.0, j / 4096.0);
            occluder_depth = texture(shadowTex, bia(x_tex.xy, bias)).x;
            if (occluder_depth + 0.001 < x_tex.z) {
                count--;
            }
        }
    }
    float visibility = count / 50.0 + 0.5;
    out_Color = vec4(ambientColor + visibility * blinn_phong(kd).xyz, 1);
//	if (occluder_depth + 0.001 < x_tex.z) {
//        out_Color = vec4(ambientColor + visibility * blinn_phong(kd).xyz, 1);
//	} else {
//	    out_Color = vec4(ambientColor + blinn_phong(kd).xyz, 1);
//	}
//    vec3 kd = diffColor;
//    out_Color = var_Color;
}
