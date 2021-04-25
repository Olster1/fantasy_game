uniform sampler2D tex;
uniform float time;
uniform vec3 playerPos_inWorldP;

in vec4 colorOut;
in vec2 texUV_out;
in vec3 fragPos;
in mat3 modelToWorldMatrix;

out vec4 color;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

float innerRadius = 3;
float outerRadius = 6;

void main (void) {
	vec4 texColor = texture(tex, texUV_out);

    // float dist = length(fragPos - playerPos_inWorldP);
    // float attenuate = ((dist - innerRadius) / (outerRadius - innerRadius));

			
	float fValue = fbm(texUV_out*5.0 + vec2(0.05*time));

    vec4 c = vec4(colorOut.r*fValue, colorOut.g*fValue, colorOut.b*fValue, colorOut.a*mix(0, clamp(fValue, 0.0, 1.0), texColor.a));

    c.rgb *= c.a;

    color = c;
}