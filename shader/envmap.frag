#version 330 core

in vec3 angle;
out vec3 color;

uniform sampler2D panorama;

void main() {
	vec3 a = normalize(angle);
	float pi = 3.1415926535f;
	vec2 uv = vec2(atan(a.z, a.x) / (2.f * pi), acos(a.y) / pi); 
	color = texture(panorama, uv).rgb;
}