#version 330 core
layout(location=0) in vec3 recPos;

uniform mat4 proj;
uniform mat4 c2w;	
out vec3 pixelPos;

void main(){
	gl_Position = proj * vec4(recPos, 1.0);
	pixelPos = (c2w * vec4(recPos,1.0f)).xyz;
}