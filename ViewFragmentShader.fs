#version 460 core

out vec4 FragColor;

uniform sampler2D resultTexture;
uniform uint count;
uniform uint width;
uniform uint height;

void main(){
	vec3 tc = texture(resultTexture, vec2(gl_FragCoord.x / float(width), gl_FragCoord.y / float(height))).rgb;
	FragColor = vec4(tc / count, 1.0f);
}