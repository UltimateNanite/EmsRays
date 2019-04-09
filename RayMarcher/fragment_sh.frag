R"(
#version 450 core
#define EPSILON 0.00001
out vec4 FragColor;
in vec4 gl_FragCoord;

uniform vec3	camera_pos;
uniform vec3	camera_rot;
uniform float	camera_fov;
uniform vec2	camera_res;

uniform ivec2 resolution;
uniform float threshold = 0.1f;


struct Material
{
	vec4 color;
};

//Types
//1 = Point Light
struct Light {
	int type;
	vec3 pos;
	vec3 direction;
};

//Types
//1 = Sphere
//2 = Box
struct RMObject {
  int type; //Type of object
  vec3 pos; //Position of object
  vec3 size; //Only uses x in case of sphere
  Material mat;
};
	 
uniform RMObject objects[256];
uniform int objects_size;

uniform Light lights[256];// = Light[1](Light(1, vec3(10.f,100.f,200.f), vec3(0.0, -1.0, 0.0)));
uniform int lights_size = 1;

//setCamera is a function to get the camera's rotation matrix.
//ro is the ray's origin.
//ta is the ray's direction.
//cr is the ray's rotation around the camera's direction (roll).
mat3 getCamRotMatrix(in vec3 ro, in vec3 ta, float cr)
{
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
  return mat3( cu, cv, cw );
}


uniform bool celShading = false;
mat4 lookAt(vec3 from, vec3 to) 
{ 
	vec3 tmp = vec3(0, 1, 0);
    vec3 forward = normalize(from - to); 
    vec3 right = cross(normalize(tmp), forward); 
    vec3 up = cross(forward, right); 
 
    mat4 lookatmat; 
	
	lookatmat = mat4(
		vec4(right.x, right.y, right.z, 0),
		vec4(up.x, up.y, up.z, 0),
		vec4(forward.x, forward.y, forward.z, 0),
		vec4(from.x, from.y, from.z, 1)
	);
	
    return lookatmat; 
} 

float roundToNDigit(float num, int digit) {
	float exp = pow(10, digit);
	return ceil(num*exp)/exp;
}

float SDF(RMObject rmobject, vec3 rayPos) {
	vec3 d = abs(rmobject.pos - rayPos) - rmobject.size;
	return	(length(rmobject.pos - rayPos) - rmobject.size.x) * float(rmobject.type == 1) +
			(min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0))) * float(rmobject.type == 2);
}

float SDF(Light light, vec3 rayPos) {
	return(length(light.pos - rayPos) - threshold) * float(light.type == 1);
}

RMObject closestObject(vec3 rayPos) {
	float smallestdist = 10000;
	RMObject closestObject;

	for(int i = 0; i < objects_size; i++) {
		float dist = SDF(objects[i], rayPos);
		if (dist < smallestdist) {
			smallestdist = dist;
			closestObject = objects[i];
		}
	}
	return closestObject;
}

float closestDist(vec3 rayPos) {
	float smallestdist = 10000;
	
	for(int i = 0; i < objects_size; i++) {
		float dist = SDF(objects[i], rayPos);
		if (dist < smallestdist) {
			smallestdist = dist;
		}
	}
	return smallestdist;
}




vec3 estimateNormal(vec3 p) {
    return normalize(vec3(
        closestDist(vec3(p.x + EPSILON, p.y, p.z)) - closestDist(vec3(p.x - EPSILON, p.y, p.z)),
        closestDist(vec3(p.x, p.y + EPSILON, p.z)) - closestDist(vec3(p.x, p.y - EPSILON, p.z)),
        closestDist(vec3(p.x, p.y, p.z  + EPSILON)) - closestDist(vec3(p.x, p.y, p.z - EPSILON))
    ));
}
vec3 calculateNormal(vec3 p) // for function f(p)
{
    const float eps = 0.0001; // or some other value
    const vec2 h = vec2(eps,0);
    return normalize( vec3(closestDist(p+h.xyy) - closestDist(p-h.xyy),
                           closestDist(p+h.yxy) - closestDist(p-h.yxy),
                           closestDist(p+h.yyx) - closestDist(p-h.yyx) ) );
}






bool hardShadows(Light light, vec3 rayhit, float max_length) {
	vec3 dir = normalize(rayhit - light.pos);
	
	vec3 lightRay = light.pos;
	float minDistance = closestDist(lightRay);


	for (float RL = 0; RL < max_length; RL += minDistance) {

		if (minDistance < threshold) {
			return (distance(lightRay, rayhit) > 2.0);
		}
		minDistance = closestDist(lightRay);
		lightRay += dir * minDistance;
	}
	return false;
}

void main() {
	uint x = uint(gl_FragCoord.x);
	uint y = uint(gl_FragCoord.y);

	//Ray
	vec3 rayPos = camera_pos;
	vec3 movementUnitVec;
	movementUnitVec.x = (float(x) - resolution.x / 2) / float(resolution.x);
	movementUnitVec.y = (float(y) - resolution.y / 2) / float(resolution.y);
	movementUnitVec.x *= float(resolution.x) / float(resolution.y);
	movementUnitVec.z = 1;

	//Convert to unit vector
	movementUnitVec = normalize(movementUnitVec);


	mat3 cam_mat = getCamRotMatrix(camera_pos, camera_pos + camera_rot, 0.0);
	movementUnitVec = cam_mat * movementUnitVec;

	float minDistance;
	

	bool hit = false;
	bool timeout = false;

	minDistance = closestDist(rayPos);

	for (float RL = 0; RL < 2500; RL += minDistance) {
		if (minDistance < threshold) {
			hit = true;
			break;
		}
		minDistance = closestDist(rayPos);
		rayPos += movementUnitVec * minDistance;
	}

	if (hit) {
		bool highLight = false;
		int shadows = 0;
		float lightStrength;
		for (int i = 0; i < lights_size; i++) {
			vec3 normal = calculateNormal(rayPos);

			vec3 lightNormal = normalize(rayPos - lights[i].pos);

			float dotProd = -dot(normal, lightNormal);
			
			highLight = (dotProd > 0.9);
			lightStrength += dotProd;

			if (!hardShadows(lights[i], rayPos, 2500))
				shadows++;
		}

		RMObject obj = closestObject(rayPos);
		float r = obj.mat.color.x;
		float g = obj.mat.color.y;
		float b = obj.mat.color.z;

		
		
		if (celShading) {
			r += float(highLight) * 0.7;
			g += float(highLight) * 0.7;
			b += float(highLight) * 0.7;
			r *= 0.3 + min(shadows,1) * 0.7;
			g *= 0.3 + min(shadows,1) * 0.7;
			b *= 0.3 + min(shadows,1) * 0.7;
		} else {
			r *= 0.3 + shadows * 0.7;
			g *= 0.3 + shadows * 0.7;
			b *= 0.3 + shadows * 0.7;
			r *= lightStrength;
			g *= lightStrength;
			b *= lightStrength;
		}
		

		vec4 color = vec4(r, g, b, obj.mat.color.w);

		 
		gl_FragColor = color;

	} else { gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); }
}
)"