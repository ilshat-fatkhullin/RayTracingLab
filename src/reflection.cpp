#include "reflection.h"

Reflection::Reflection(short width, short height) :ShadowRays(width, height)
{
}

Reflection::~Reflection()
{
}

Payload Reflection::Hit(const Ray& ray, const IntersectableData& data, const MaterialTriangle* triangle, const unsigned int raytrace_depth) const
{
	if (raytrace_depth <= 0)
	{
		return Miss(ray);
	}
	if (triangle == nullptr) return Miss(ray);
	Payload payload;
	payload.color = triangle->emissive_color;

	float3 X = ray.position + ray.direction * data.t;
	float3 N = triangle->GetNormal(data.baricentric);

	if (triangle->reflectiveness)
	{
		float3 reflection_direction = ray.direction - 2.0f * dot(N, ray.direction) * N;
		Ray reflection_ray(X, reflection_direction);
		return TraceRay(reflection_ray, raytrace_depth - 1);
	}

	for (auto const& light : lights) {
		Ray toLight(X, light->position - X);
		float toLightDist = linalg::length(light->position - X);

		float traceShadow = TraceShadowRay(toLight, toLightDist);
		if (std::fabs(traceShadow - toLightDist) > 0.001f) {
			continue;
		}

		payload.color += light->color * triangle->diffuse_color
			* std::max(0.0f, linalg::dot(N, toLight.direction));

		float3 reflectionDir = 2.0f * linalg::dot(N, toLight.direction) * N - toLight.direction;
		payload.color += light->color * triangle->specular_color
			* std::powf(std::max(0.0f, linalg::dot(ray.direction, reflectionDir)), triangle->specular_exponent);
	}

	return payload;
}
