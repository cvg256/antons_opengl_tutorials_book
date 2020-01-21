#version 410

in float dist;
out vec4 frag_color;
uniform float blue = 0.0;

void main() {
	frag_color = vec4(1.0, 0.0, blue, 1.0);
	//use z position to shader darker to hep perception of distance
	frag_color.xyz *= dist;
}