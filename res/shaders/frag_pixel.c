uniform sampler2D tex;
uniform vec4 timeOfDayColor;

in vec4 colorOut;
in vec2 texUV_out;
in vec3 fragPos;

out vec4 color;

struct Light {
    vec3 pos;
    vec3 color;
};

uniform int lightCount;
uniform Light lights[16];

float outerRadius = 7.0f;
float innerRadius = 0.2f;

void main (void) {

    vec3 lightInfluence = vec3(timeOfDayColor.x, timeOfDayColor.y, timeOfDayColor.z);

    for(int i = 0; i < lightCount; ++i) {
        Light l = lights[i];

        float dist = length(fragPos - l.pos);
        if(dist < outerRadius) {

            float attenuate = 1.0 - ((dist - innerRadius) / (outerRadius - innerRadius));
            lightInfluence += attenuate*l.color;     
        }
    }
    
    vec2 size = textureSize(tex, 0);

    vec2 uv = texUV_out * size;
    vec2 duv = fwidth(uv);
    uv = floor(uv) + 0.5 + clamp(((fract(uv) - 0.5 + duv)/duv), 0.0, 1.0);
    uv /= size;

    vec4 texColor = texture(tex, uv);

	vec4 preMultAlphaColor = colorOut;

    preMultAlphaColor *= vec4(lightInfluence, 1);
	
	vec4 c = preMultAlphaColor*texColor;

	c.rgb *= preMultAlphaColor.a;

	// if(c.a == 0.0) discard; 

    color = c;
}