#pragma once
#include <cstdlib>
#include <ctime>
#include <noise/noise.h>
#include <algorithm>
#include <iostream>
#include <vector>

const double pi = 3.1415926535;
using namespace noise;

class HeightMap
{
private:
	//constants for use in the Perlin Noise generation
	const float AMP = 1;
	const int OCT = 3;
	float roughness = 0.2f;
	int seed;

	float min_x, max_x;
	float min_z, max_z;

	float noise_width;
	float noise_height;
	std::vector<std::vector<float>> noises;
	std::vector<std::vector<float>> heights;
	//using a module for the Perlin algorithm
	module::Perlin perlin_module;
public:
	//generate the heightmap object
	HeightMap(float noise_width, float noise_height, float min_x, float max_x, float min_z, float max_z)
		: noise_width(noise_width), noise_height(noise_height), min_x(min_x), max_x(max_x), min_z(min_z), max_z(max_z) {
		perlin_module.SetFrequency(0.3f);
		this->noise_width = 10;
		this->noise_height = 10;
		noises.resize(this->noise_width + 1);
		noises.resize(this->noise_height + 1);
		for (int i = 0; i < this->noise_width; i++) {
			noises[i].resize(this->noise_height + 1);
			heights[i].resize(this->noise_height + 1);
		}

		build();
	}
	float generate_height(int x, int z) {
		return get_interpolated_noise(x, z);
	}
	float get_smooth_noise(int x, int z) {
		float corners = get_noise(x - 1, z - 1) + get_noise(x + 1, z - 1) + get_noise(x - 1, z + 1) + get_noise(x + 1, z + 1) / 16.0f;
		float sides = get_noise(x - 1, z) + get_noise(x + 1, z) + get_noise(x, z - 1) + get_noise(x, z + 1) / 8.0f;
		float center = get_noise(x, z) / 4.0f;
		return corners + sides + center;
	}
	float get_interpolated_noise(int x, int z) {
		return get_height(x, z);
	}
	float get_height(int x, int z) { //return the height at a certain point on the coordinate system
		if (x < 0 || x >= noise_width || z < 0 || z >= noise_height) return 0;
		else return heights.at(x).at(z);
	}
	float interpolate(float a, float b, float blend) { //formula for interpolating between two values
		double theta = blend * pi;
		float f = (float)(1.0f - cos(theta)) * 0.5f;
		return a * (1.0f - f) + b * f;
	}
	float get_noise(int x, int z) {
		return perlin_module.GetValue(x, 0, z);
	}
	float calcNoiseMapValue(float x, float z) {
		double x_extent = max_x - min_x + 1.1234f;
		double z_extent = max_z - min_z + 3.234f;
		double x_delta = x_extent / (double)noise_width; // 
		double z_delta = z_extent / (double)noise_height;
		double x_current = min_x;
		double z_current = min_z;

		x_current += x_delta * x;
		z_current += z_delta * z;

		double sw_val, se_val, nw_val, ne_val;
		sw_val = perlin_module.GetValue(x_current, 0, z_current);
		se_val = perlin_module.GetValue(x_current + x_extent, 0, z_current);
		nw_val = perlin_module.GetValue(x_current, 0, z_current + z_extent);
		ne_val = perlin_module.GetValue(x_current + x_extent, 0, z_current + z_extent);

		double xBlend = 1.0 - ((x_current - min_x) / x_extent);
		double zBlend = 1.0 - ((z_current - min_z) / z_extent);

		double z0 = linear_interp(sw_val, se_val, xBlend);
		double z1 = linear_interp(nw_val, ne_val, xBlend);
		float finalValue = (float)linear_interp(z0, z1, zBlend);

		return finalValue;
	}
private:
	void build() {
		double x_extent = max_x - min_x + 0.3f;
		double z_extent = max_z - min_z + 0.3f;
		double x_delta = x_extent / (double)noise_width; // 
		double z_delta = z_extent / (double)noise_height;
		double x_current = min_x;
		double z_current = min_z;

		for(int z = 0; z < noise_height; z++) {
			x_current = min_x;
			for(int x = 0; x < noise_width; x++) {
				double sw_val, se_val, nw_val, ne_val;
				sw_val = perlin_module.GetValue(x_current,			  0, z_current);
				se_val = perlin_module.GetValue(x_current + x_extent, 0, z_current);
				nw_val = perlin_module.GetValue(x_current,			  0, z_current + z_extent);
				ne_val = perlin_module.GetValue(x_current + x_extent, 0, z_current + z_extent);
				double x_blend = 1.0 - ((x_current - min_x) / x_extent);
				double z_blend = 1.0 - ((z_current - min_z) / z_extent);
				double z0 = linear_interp(sw_val, se_val, x_blend);
				double z1 = linear_interp(nw_val, ne_val, x_blend);
				float final_value = (float)linear_interp(z0, z1, z_blend);

				heights[x][z] = final_value;

				x_current += x_delta;
			}

			z_current += z_delta;
		}
	}
	double linear_interp(double n0, double n1, double a) {
		return ((1.0 - a) * n0 + (a * n1));
	}
};

