#version 330 core
#define NUM_SPHERES	7
#define NUM_PLANES	5
#define NUM_LIGHTS	1
#define MAX_BOUNCE 50
#define SAMPLE_PER_PIXEL 100
#define PI        3.14159265358979323

out vec4 FragColor;
in vec3 pixelPos;

struct Ray{
	vec3 pos;
	vec3 dir;
};

struct Material{
	bool diffuse;
	bool metallic;
	vec3 attenuation;
};

struct HitInfo{
	float t;
	vec3 position;
	vec3 normal;
	Material mtl;
	bool frontFace;
};

struct Sphere{
	vec3 center;
	float radius;
	Material mtl;
};

struct Plane{
	vec3 normal;
	vec3 position;
	float lenght;
	Material mtl;
};

struct Light{
	vec3 position;
	vec3 intensity;
};

uniform Sphere spheres[NUM_SPHERES];
uniform Plane planes[NUM_PLANES];
uniform Light lights[NUM_LIGHTS];
uniform mat4 c2w;
uniform float view_pixel_width;		//width of viewport pixel
uniform float view_pixel_height;		
uniform vec2 randomVector;

Ray GeneratePrimaryRay();
Ray ComputeScatterRay(HitInfo hit, Ray incidentRay);
bool IntersectRay(inout HitInfo hit,Ray ray);
vec3 Shade(vec3 position, vec3 normal, vec3 view, Material mtl);
float rand(vec2 co);

vec2 seed;
vec3 cameraPos;
vec3 errorRay = vec3(2,2,2);

void main(){
	cameraPos = (c2w*vec4(0.0f, 0.0f, 0.0f,1.0f)).xyz;
	vec3 color = vec3(0.0f,0.0f,0.0f);
	seed = gl_FragCoord.xy;
	
	for(int i=0 ; i<SAMPLE_PER_PIXEL ; i++)
	{
		Ray ray = GeneratePrimaryRay();
		vec3 tmpColor = vec3(1.0f,1.0f,1.0f);
		int depth = MAX_BOUNCE;
		
		for(int j = 0 ; j < MAX_BOUNCE ; j++){		
			HitInfo hit;
			if(IntersectRay(hit, ray)){
				tmpColor *= hit.mtl.attenuation;
				ray = ComputeScatterRay(hit, ray);
				if(ray.dir == errorRay){
					//material is not defined
					tmpColor *= vec3(0,0,0);
					break;
				}
			}
			else{	
				//sky color
				float t = 0.5*(ray.dir.y + 1.0);
				tmpColor *= (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
				break;
			}
			depth--;
		}
		if(depth <= 0){	
			//no light path toward the light source with the given depth
			tmpColor *= vec3(0,0,0);
		}
		color+=tmpColor;
	}
	FragColor = vec4(color / SAMPLE_PER_PIXEL, 1.0f);
}

Ray ComputeScatterRay(HitInfo hit, Ray incidentRay)
{
	Ray scatter;
	scatter.pos = hit.position + 1e-3 * hit.normal;

	if(hit.mtl.diffuse){												
		float y =  rand(seed)* 2.0 -1.0;
		float phi = 2*PI*rand(seed);
		float x = sqrt(1-y*y)*cos(phi);
		float z = sqrt(1-y*y)*sin(phi);
		vec3 target = normalize(vec3 (x,y,z));
		if(dot(target,hit.normal) < 0.0f){
			target = -target;
		}
		//target = scatter.pos + target;
		//scatter.dir = normalize(target - scatter.pos);
		scatter.dir = normalize(target);
		return scatter;
	}
	else if(hit.mtl.metallic){
		scatter.dir = 2*dot(-incidentRay.dir,hit.normal)*hit.normal + incidentRay.dir;	//perfect reflection direction
		return scatter;
	}
	else{
		scatter.dir = errorRay;
		return scatter;
	}

}

bool IntersectRay(inout HitInfo hit,Ray ray){
	hit.t = 1e30;
	bool foundHit = false;
	for(int i = 0 ; i < NUM_SPHERES ; i++){									//spheres 
		vec3 center = spheres[i].center;
		vec3 tmp = ray.pos - center;
		float a = dot(ray.dir, ray.dir);
		float b = 2 * dot(ray.dir,tmp);
		float c = dot(tmp, tmp) - spheres[i].radius*spheres[i].radius;
		float delta = b*b - 4*a*c;
		if(delta >= 0.0f){
			float t;
			if(length(ray.pos - center) < spheres[i].radius){				//ray origin is inside the sphere																	
				t = (-b + sqrt(delta))/ 2.0 * a; 
			}
			else{
				t = (-b - sqrt(delta))/ 2.0 * a; 
			}
			if( t < hit.t && t > 0.0f){
				hit.t = t;
				hit.position = ray.pos + t*ray.dir;
				hit.normal = normalize(hit.position - center);
				hit.frontFace = dot(ray.dir,hit.normal) < 0.0f;
				hit.normal = hit.frontFace ? hit.normal : -hit.normal;
				hit.mtl = spheres[i].mtl;
				foundHit = true;
			}
		}
	}
	for(int i = 0 ; i < NUM_PLANES ; i++){									//plane count
		float denominator = dot(ray.dir, planes[i].normal);
		if(denominator != 0.0f){											//plane and ray are not perpendicular			
			float c = dot(planes[i].normal, planes[i].position);
			float t = (c - dot(ray.pos, planes[i].normal)) / denominator;
			vec3 positionOnPlane = ray.pos + t*ray.dir;
			vec3 distance = positionOnPlane - planes[i].position;
			if(abs(distance.x) < planes[i].lenght && abs(distance.y) < planes[i].lenght && abs(distance.z) < planes[i].lenght){
				if( t < hit.t && t > 0.0f ){
					hit.t = t;
					hit.position = positionOnPlane;
					hit.normal = planes[i].normal;
					hit.frontFace = dot(ray.dir,planes[i].normal) < 0.0f;
					hit.normal = hit.frontFace ? hit.normal : -hit.normal;
					hit.mtl = planes[i].mtl;
					foundHit = true;
				}
			}
		}
	}
	return foundHit;
}

Ray GeneratePrimaryRay(){
	float n = rand(seed);													//0, 1
	float y = view_pixel_width * (n-1) + 0.5f*view_pixel_width	;			// -1/2*view_pixel_width , 1/2*view_pixel_width
	float offsetX = y;
	n = rand(seed);															//0, 1
	y = view_pixel_height * (n-1) + 0.5f*view_pixel_height	;				// -1/2*view_pixel_width , 1/2*view_pixel_width
	float offsetY = y;

	Ray ray;
	ray.pos = pixelPos + vec3(offsetX, offsetY, 0.0f);
	ray.dir = normalize(ray.pos - cameraPos);
	return ray;
}

float rand(vec2 co){
	seed-=randomVector;
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}