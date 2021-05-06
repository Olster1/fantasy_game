uniform sampler2D tex;

in vec4 colorOut;
in vec2 texUV_out;

out vec4 fragColor;

void main (void) {

    vec2 size = textureSize(tex, 0);

    vec2 uv = texUV_out * size;
    vec2 duv = fwidth(uv);
    uv = floor(uv) + 0.5 + clamp(((fract(uv) - 0.5 + duv)/duv), 0.0, 1.0);
    uv /= size;

    vec4 texColor = texture(tex, uv);

	vec4 preMultAlphaColor = colorOut;

    float alpha = preMultAlphaColor.w;
    preMultAlphaColor.w = 1;

    vec4 sampleColor = preMultAlphaColor*texColor;

	sampleColor *= alpha;

	if(sampleColor.a == 0.0) discard; 


 //    //do nothing, this is what it's doing but it does this automatically
    // gl_FragDepth = 0.5;//gl_FragCoord.z;
}