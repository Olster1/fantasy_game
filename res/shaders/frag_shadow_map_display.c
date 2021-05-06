uniform sampler2D tex;

in vec4 colorOut;
in vec2 texUV_out;

out vec4 color;

void main (void) {
    float c = texture(tex, texUV_out).r;
   color = vec4(c, c, c, 1);
}