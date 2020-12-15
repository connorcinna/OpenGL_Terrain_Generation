#pragma once

#include <string>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <iostream>
#include "../utils/stb_image.h"

class Texture {
	public:
		GLfloat tex_coords[8];
	protected:
		std::string &file_path;
		unsigned int texture_ID; // texture ID itself
		unsigned int width, height; // normalized
		unsigned int x, y; // coordinates(normalized) in the texture. if this texture isn't a sprite sheet, should be (0, 0)
	public:
		Texture(std::string file_name);
		Texture(std::string file_name, std::vector<float> tc);

		void bind();
		void unbind();

		inline unsigned int get_width() const { return width; }
		inline unsigned int get_height() const { return height; }
		inline unsigned int get_ID() const { return texture_ID; }

		inline std::string get_file_path() const { return file_path; }

		GLvoid* get_image_data();
	private:
		unsigned int load();
};