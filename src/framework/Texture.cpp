#include "texture.h"

Texture::Texture(std::string file_name) : file_path(file_name) {
	this->texture_ID = load();

	// Set default texture coordinates
	tex_coords[0] = 0; tex_coords[1] = 0; // bottom left
	tex_coords[2] = 0; tex_coords[3] = 1; // top left
	tex_coords[4] = 1; tex_coords[5] = 1; // top right
	tex_coords[6] = 1; tex_coords[7] = 0; // bottom right

	this->x = 0;
	this->y = 0;
}

Texture::Texture(std::string file_name, std::vector<float> tc) : file_path(file_name) {
	this->texture_ID = load();

	// Set default texture coordinates
	tex_coords[0] = tc[0]; tex_coords[1] = tc[1]; // bottom left
	tex_coords[2] = tc[2]; tex_coords[3] = tc[3]; // top left
	tex_coords[4] = tc[4]; tex_coords[5] = tc[5]; // top right
	tex_coords[6] = tc[6]; tex_coords[7] = tc[7]; // bottom right
}

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, texture_ID);
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Texture::load() {
	unsigned int id;
	glGenTextures(1, &id);

	int width, height, num_components;
	unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 0);
	if(data) {
		GLenum format;
		if(num_components == 1)
			format = GL_RED;
		else if(num_components == 3)
			format = GL_RGB;
		else if(num_components == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); 

		// Set the class variables to the respective size of the image retrieved form stbi lib
		this->width = width;
		this->height = height;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	} else {
		std::cout << "Texture failed to load at path: " << file_path << std::endl;
	}

	return id;
}

GLvoid* Texture::get_image_data() {
	int width, height, num_components;
	unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 0);

	return data;
}