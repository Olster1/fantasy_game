uniform sampler2D tex;
uniform vec4 timeOfDayColor;

uniform sampler2D normal_tex;

uniform vec3 eye_worldspace;

in vec4 colorOut;
in vec2 texUV_out;
in vec3 fragPos;
in mat3 modelToWorldMatrix;

out vec4 color;

struct Light {
    vec3 pos;
    vec3 color;
};

uniform int lightCount;
uniform Light lights[16];

float outerRadius = 9.0f;
float innerRadius = 0.2f;

float specularConstant = 256;

void main (void) {

    vec2 size = textureSize(tex, 0);

    vec2 uv = texUV_out * size;
    vec2 duv = fwidth(uv);
    uv = floor(uv) + 0.5 + clamp(((fract(uv) - 0.5 + duv)/duv), 0.0, 1.0);
    uv /= size;

    vec3 lightInfluence = vec3(0, 0, 0);

    vec4 normalInModelSpace_ = texture(normal_tex, uv);

    vec3 normalInModelSpace = vec3(normalInModelSpace_.x, normalInModelSpace_.y, normalInModelSpace_.z);

    vec3 normalInWorldSpace = normalize(modelToWorldMatrix * normalInModelSpace);

    float specFactor = 0.0f;
    float diffTotal = 0.0f;

    for(int i = 0; i < lightCount; ++i) {
        Light l = lights[i];

        float dist = length(fragPos - l.pos);
        if(dist < outerRadius) {

            vec3 lightDir = vec3(normalize(fragPos - l.pos));
            float diff = max(dot(normalInWorldSpace, lightDir), 0.0);
            diffTotal += diff;
            /////////////////////////

            vec3 viewDir = normalize(fragPos - eye_worldspace);
            vec3 halfwayVector = normalize(viewDir + lightDir);
            float spec = pow(max(dot(normalInWorldSpace, halfwayVector), 0.0), specularConstant);

            //////////////////

            float attenuate = 1.0 - ((dist - innerRadius) / (outerRadius - innerRadius));
            lightInfluence += diff*attenuate*l.color;

            specFactor += spec;     

        }
    }
    
    

    vec4 texColor = texture(tex, uv);

	vec4 preMultAlphaColor = colorOut;

    // preMultAlphaColor *= vec4(lightInfluence, 1);
	
    float alpha = preMultAlphaColor.w;
    preMultAlphaColor.w = 1;

    vec4 sampleColor = preMultAlphaColor*texColor;
    vec4 ambientColor = timeOfDayColor*sampleColor;
    vec4 specularColor = specFactor*vec4(1, 1, 1, 0);
    vec4 diffuseColor = vec4(lightInfluence, 0)*sampleColor;

	vec4 c = vec4(ambientColor + diffuseColor + specularColor);;//timeOfDayColor*preMultAlphaColor*texColor + specFactor*vec4(1, 1, 1, 1) + diff*preMultAlphaColor*texColor;

	c *= alpha;


	// if(c.a == 0.0) discard; 

    color = c;
}