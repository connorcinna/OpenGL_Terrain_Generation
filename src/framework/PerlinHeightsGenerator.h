#pragma once

#include <noise/noise.h>

using namespace noise;

class PerlinHeightsGenerator {
	public:
		module::Perlin perlin_module;
	public:
		PerlinHeightsGenerator() {
			
		}
		//x and z have 0.5 added to them for floor
		float getHeight(int x, int z) {
			return perlin_module.GetValue(x + 0.5f, 0, z + 0.5f);
		}
};