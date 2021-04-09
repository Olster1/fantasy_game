uniform sampler2D tex;

in vec4 colorOut;
in vec2 texUV_out;
in mat3 modelToWorldMatrix;

out vec4 color;
void main (void) {

    
    vec2 size = textureSize(tex, 0);

    vec2 uv = texUV_out * size;
    vec2 duv = fwidth(uv);
    uv = floor(uv) + 0.5 + clamp(((fract(uv) - 0.5 + duv)/duv), 0.0, 1.0);
    uv /= size;

    vec4 texColor = texture(tex, uv);

	vec4 preMultAlphaColor = colorOut;

	vec4 c = preMultAlphaColor*texColor;

	c.rgb *= preMultAlphaColor.a;

	// if(c.a == 0.0) discard; 

    color = c;
}