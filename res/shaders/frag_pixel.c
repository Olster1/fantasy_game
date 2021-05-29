uniform sampler2D tex;

uniform sampler2D shadowMapSampler; //for the sun
uniform mat4 worldToSunSpace; 
uniform vec3 sunDirection; //moving towards the sun 

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
    float innerRadius;
    float outerRadius;
};

uniform int lightCount;
uniform Light lights[16];

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
        if(dist < l.outerRadius) {

            vec3 lightDir = vec3(normalize(fragPos - l.pos));
            float diff = max(dot(normalInWorldSpace, lightDir), 0.0);
            diffTotal += diff;
            /////////////////////////

            vec3 viewDir = normalize(fragPos - eye_worldspace);
            vec3 halfwayVector = normalize(viewDir + lightDir);
            float spec = pow(max(dot(normalInWorldSpace, halfwayVector), 0.0), specularConstant);

            //////////////////

            float attenuate = 1.0 - ((dist - l.innerRadius) / (l.outerRadius - l.innerRadius));
            lightInfluence += diff*attenuate*l.color;

            // specFactor += spec;     

        }
    }




    shadowMapSampler; //for the sun
    vec4 clipSpace = worldToSunSpace * vec4(fragPos, 1.0);

    //divide by z to convert from clip space to NDC space
    vec3 ndcSpace = clipSpace.xyz / clipSpace.w;

    //convert from ndc space to texture uv space -1 - 1 -> 0 - 1
    vec3 uv_shadowSpace = 0.5*ndcSpace + 0.5;

    //the closest thing in in view
    float closestDepth = texture(shadowMapSampler, uv_shadowSpace.xy).r;

    float shadowFactor = 1.0;

    vec4 timeOfDayColor_ = timeOfDayColor;
    //the bias is due to shadow acne where more than one texel is mapping to the same spot on the shadow map. This doesn't matter when the light is parrallel to the surface normal, but matters alot when they are perpindicular to the normal - dotProduct approaches 0 
    
    float shadowBias = max(0.00005*(1.0f - dot(normalInWorldSpace, sunDirection)), 0.0005);
    if((uv_shadowSpace.z - shadowBias) > closestDepth) {
        // //in shadow
        shadowFactor = 0;
        timeOfDayColor_ = vec4(0.4, 0.4, 0.4, 1);
    }  

    vec4 texColor = texture(tex, uv);

	vec4 preMultAlphaColor = colorOut;

    // preMultAlphaColor *= vec4(lightInfluence, 1);
	
    float alpha = preMultAlphaColor.w;
    preMultAlphaColor.w = 1;

    vec4 sampleColor = preMultAlphaColor*texColor;
    vec4 ambientColor = timeOfDayColor_*sampleColor;
    vec4 specularColor = specFactor*vec4(1, 1, 1, 0);
    vec4 diffuseColor = shadowFactor*vec4(lightInfluence, 0)*sampleColor;

	vec4 c = vec4(ambientColor + diffuseColor + specularColor);//timeOfDayColor*preMultAlphaColor*texColor + specFactor*vec4(1, 1, 1, 1) + diff*preMultAlphaColor*texColor;

	c *= alpha;

	if(c.a == 0.0) {
        discard;//gl_FragDepth = 1.0;   else gl_FragDepth = gl_FragCoord.z; 
    } 

    color = c;
}