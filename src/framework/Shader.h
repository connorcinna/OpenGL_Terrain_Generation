#ifndef SHADER_H 
#define SHADER_H 

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>

#include "texture.h"

class Shader {
	public:
		// Program ID
		unsigned int id;

		Shader() : id(0) {
			
		}

		// Constructor reads and builds the shader
		Shader(const char* vertex_path, const char* fragment_path) {
			// Retrieve the vertex/fragment source code from filePath
			std::string vertex_code;
			std::string fragment_code;
			std::ifstream v_shader_file;
			std::ifstream f_shader_file;

			// Ensure ifstream objects can throw exceptions
			v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try {
				// Open files
				v_shader_file.open(vertex_path);
				f_shader_file.open(fragment_path);
				std::stringstream v_shader_stream, f_shader_stream;

				// Read file's buffer contents into streams
				v_shader_stream << v_shader_file.rdbuf();
				f_shader_stream << f_shader_file.rdbuf();

				// Close file handlers
				v_shader_file.close();
				f_shader_file.close();

				// Convert stream into string
				vertex_code = v_shader_stream.str();
				fragment_code = f_shader_stream.str();
			} 
			catch(std::ifstream::failure e) {
				std::cout << "error: shader file not successfully read" << std::endl;
			}

			// Compile the shaders

			const char* v_shader_code = vertex_code.c_str();
			const char* f_shader_code = fragment_code.c_str();

			unsigned int vertex, fragment;

			// Set up vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &v_shader_code, NULL);
			glCompileShader(vertex);
			check_compile_errors(vertex, "VERTEX");

			// Set up fragment shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &f_shader_code, NULL);
			glCompileShader(fragment);
			check_compile_errors(fragment, "FRAGMENT");

			// Link both shader objects into a shader program that we can use for rendering
			// Shader program object is the final linked version of multiple shaders combined
			id = glCreateProgram(); // Create the ID
			glAttachShader(id, vertex); // Attach the vertex shader
			glAttachShader(id, fragment); // Attach the fragment shader
			glLinkProgram(id); // Link the shaders altogether
			check_compile_errors(id, "PROGRAM");

			// Delete the shaders we created, the useful part is already in the shader program
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			std::cout << "Shader " << id << " created successfully from " << vertex_path << " and " << fragment_path << std::endl;
		}

		Shader(const char* vertex_path, const char* tess_control_path, const char* tess_eval_path, const char* geomPath, const char* fragPath) {
			unsigned int vertex, tess_control, tess_eval, geometry, fragment;
			vertex = compile_shader(GL_VERTEX_SHADER, vertex_path);
			tess_control = compile_shader(GL_TESS_CONTROL_SHADER, tess_control_path);
			tess_eval = compile_shader(GL_TESS_EVALUATION_SHADER, tess_eval_path);
			geometry = compile_shader(GL_GEOMETRY_SHADER, geomPath);
			fragment = compile_shader(GL_FRAGMENT_SHADER, fragPath);

			id = glCreateProgram();
			glAttachShader(id, vertex);
			glAttachShader(id, tess_control);
			glAttachShader(id, tess_eval);
			glAttachShader(id, geometry);
			glAttachShader(id, fragment);
			glLinkProgram(id);
			check_compile_errors(id, "PROGRAM");

			glDeleteShader(vertex);
			glDeleteShader(tess_control);
			glDeleteShader(tess_eval);
			glDeleteShader(geometry);
			glDeleteShader(fragment);

			std::cout << "Tessellation Shader Created" << std::endl;
		}

		Shader(const char* vertex_path, const char* tess_control_path, const char* tess_eval_path, const char* fragPath) {
			unsigned int vertex, tess_control, tess_eval, fragment;
			vertex = compile_shader(GL_VERTEX_SHADER, vertex_path);
			tess_control = compile_shader(GL_TESS_CONTROL_SHADER, tess_control_path);
			tess_eval = compile_shader(GL_TESS_EVALUATION_SHADER, tess_eval_path);
			fragment = compile_shader(GL_FRAGMENT_SHADER, fragPath);

			id = glCreateProgram();
			glAttachShader(id, vertex);
			glAttachShader(id, tess_control);
			glAttachShader(id, tess_eval);
			glAttachShader(id, fragment);
			glLinkProgram(id);
			check_compile_errors(id, "PROGRAM");

			glDeleteShader(vertex);
			glDeleteShader(tess_control);
			glDeleteShader(tess_eval);
			glDeleteShader(fragment);

			std::cout << "Tessellation Shader Created" << std::endl;
		}

		// Use/activate the shader
		void use() {
			glUseProgram(id);
		}

		// Utility uniform functions
		void set_bool(const std::string &name, bool value) const {
			glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
		}
		void set_int(const std::string &name, int value) const {
			glUniform1i(glGetUniformLocation(id, name.c_str()), value);
		}

		void set_float(const std::string &name, float value) const {
			glUniform1f(glGetUniformLocation(id, name.c_str()), value);
		}

		void set_vec2(const std::string &name, glm::vec2 vec) const {
			glUniform2f(glGetUniformLocation(id, name.c_str()), vec.x, vec.y);
		}

		void set_vec2(const std::string &name, float x, float y) const {
			glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
		}

		void set_vec3(const std::string &name, glm::vec3 vec) {
			glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(vec));
		}

		void set_vec3(const std::string &name, float x, float y, float z) const {
			glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
		}

		void set_vec4(const std::string &name, glm::vec4 vec) {
			glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(vec));
		}

		void set_mat3(const std::string &name, glm::mat4 mat) const {
			glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
		}

		void set_mat4(const std::string &name, glm::mat4 mat) const {
			glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
		}

		// Basically just setSampler
		void set_texture(const std::string &name, unsigned int textureLocation) {
			glUniform1i(glGetUniformLocation(id, name.c_str()), textureLocation);
		}

	private:
		unsigned int compile_shader(GLenum type, const char* path) {
			std::string code;
			std::ifstream shader_file;

			shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try {
				// Open file
				shader_file.open(path);
				std::stringstream shader_stream;

				// Read file's buffer contents into streams
				shader_stream << shader_file.rdbuf();

				// Close file handler
				shader_file.close();

				// Convert stream into string
				code = shader_stream.str();
			} catch(std::ifstream::failure e) {
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ at path " << path << std::endl;
			}

			// Compile the shader
			const char* shaderCode = code.c_str();

			unsigned int shader;

			shader = glCreateShader(type);
			glShaderSource(shader, 1, &shaderCode, NULL);
			glCompileShader(shader); 
			//unsigned int shader;
			//shader = test_compile_shader(type, path);
			switch(type) {
				case GL_VERTEX_SHADER:
					check_compile_errors(shader, "VERTEX");
					break;
				case GL_TESS_CONTROL_SHADER:
					check_compile_errors(shader, "TESS_CONTROL");
					break;
				case GL_TESS_EVALUATION_SHADER:
					check_compile_errors(shader, "TESS_EVAL");
					break;
				case GL_GEOMETRY_SHADER:
					check_compile_errors(shader, "GEOMETRY");
					break;
				case GL_FRAGMENT_SHADER:
					check_compile_errors(shader, "FRAGMENT");
					break;
			}

			return shader;
		}

		unsigned int test_compile_shader(GLenum type, std::string fileName) {
			FILE* fp = fopen(fileName.c_str(), "rt");
			if(!fp)
				return 0;
			// Get all lines from a file
			std::vector<std::string> s_lines;
			char s_line[255];
			while(fgets(s_line, 255, fp))
				s_lines.push_back(s_line);
			fclose(fp);

			const char** s_program = new const char*[s_lines.size()];
			for(int i = 0; i < s_lines.size(); i++) {
				s_program[i] = s_lines[i].c_str();
			}

			unsigned int shader;
			shader = glCreateShader(type);
			glShaderSource(shader, sizeof(s_program), (const GLchar**)s_program, NULL);
			glAttachShader(id, shader);
			glCompileShader(shader);

			return shader;
		}

		void check_compile_errors(unsigned int shader, std::string type) {
			int success;
			char info_log[1024];
			if(type != "PROGRAM") {
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if(!success) {
					glGetShaderInfoLog(shader, 1024, NULL, info_log);
					std::cout << "SHADER COMPILATION ERROR of type: " << type << "\n" << info_log << "\n -- ---------------------------------------------------- -- " << std::endl;
				}
			} else {
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if(!success) {
					glGetProgramInfoLog(shader, 1024, NULL, info_log);
					std::cout << "PROGRAM LINKING ERROR of type " << type << "\n" << info_log << "\n -- -------------------------------------------------------- -- " << std::endl;
				}
			}
		}
};

#endif