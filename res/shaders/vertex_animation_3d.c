in vec3 vertex;
in vec3 normal;
in vec2 texUV;	
in vec4 boneIndex; //can have up to 4 bones
in vec4 boneWeight; //the weights for the 4 bones

//Instanced variables
in mat4 M;
in mat4 V;

in vec4 color;
in vec4 uvAtlas;	

uniform mat4 projection;
uniform mat4 Bones[50]; //max 50 bones


//outgoing variables
out vec4 color_frag; 
out vec3 normal_frag;
out vec2 uv_frag;
out vec3 fragPos;
out vec3 fragPosInViewSpace;

void main() {
	int index;
	///
	int index = int(boneIndex.x); // Cast to int
    newVertex = (Bones[index] * vertex) * boneWeight.x;
    newNormal = (Bone[index] * vec4(Normal, 0.0)) * Weight.x;
    


    gl_Position = projection * V * M * vec4(vertex, 1);


    color_frag = color;
    normal_frag = mat3(transpose(inverse(M))) * normal; //transpose(inverse(M))
    fragPos = vec3(M * vec4(vertex, 1));
    fragPosInViewSpace = vec3(V*M * vec4(vertex, 1));

    uv_frag = texUV;//vec2(mix(uvAtlas.x, uvAtlas.y, texUV.x), mix(uvAtlas.z, uvAtlas.w, texUV.y));
}