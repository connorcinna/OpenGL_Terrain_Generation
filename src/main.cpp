#if 1

#define GLEW_STATIC 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "utils/stb_image.h"
#include "framework/Camera.h"
#include "framework/Shader.h"
#include "framework/Texture.h"
#include "framework/Light.h"
#include "framework/HeightMap.h"
#include "framework/PerlinHeightsGenerator.h"
#include <noise/noise.h>
#include "utils/noiseutils.h"
#include "utils/ImageLoader.h"

using namespace noise;
//constants
#define SCREEN_WIDTH 2560.0f
#define SCREEN_HEIGHT 1080.0f

#define FACTOR 0.45 // Increase to make flatter
#define AMPLITUDE 300 / FACTOR

void create_height_map(float noiseWidth, float noiseHeight, float vertWidth, float vertHeight);

float get_height(int x, int z);
glm::vec3 calculate_normal(int x, int z, int upperBounds);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input_camera(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int load_texture(const char* path);
unsigned char* read_BMP(char* filename);

Camera camera(glm::vec3(20.0f, 100.0f, 3.0));

float delta = 0.0f; // Time between current frame and last frame
float last_frame = 0.0f; // Time of last frame

float mouse_last_X = 0.0f;
float mouse_last_Y = 0.0f;
bool first_mouse = true; // If the mouse has not entered the window yet

bool wire_frame = false;

module::Perlin perlin_module;

utils::NoiseMap height_map;
utils::NoiseMapBuilderPlane height_map_builder;
float heights[50][50];

unsigned char* data;
Image* image;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Terrain-Generation", NULL, NULL);
	if(window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if(glewInit() != GLEW_OK) {
		std::cout << "Error! " << std::endl;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Tell GLFW that this is the function we want ran when the window changes size
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Tell GLFW that we want the cursor to be disabled
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// So the window doesn't open in an awkward place
	glfwSetWindowPos(window, 0, 0);

	// Tell GLFW that we want the mouse_callback function to called when the cursor pos changes
	glfwSetCursorPosCallback(window, mouse_callback);

	// Tell GLFW that we want scroll_callback to be called when the scroll wheel is acted upon
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);

	// Create the height map before loading in the texture
	create_height_map(512.0, 512.0, 2, 2);

	Shader *terrain_shader = new Shader("src/shaders/terrain.vert", "src/shaders/terrain.frag");
	Texture *texture = new Texture("res/grass.png");
	Texture *height_map = new Texture("res/heightmap.bmp");
	Light *light = new Light(glm::vec3(20000, 20000, 20000), glm::vec3(1, 1, 1));

	/********************************/
	//  LOAD TERRAIN MESH/VERTICES
	/*******************************/

	const float MAX_PIXEL_COLOR = 256 * 256 * 256;

	const float SIZE = 10000;
	const float VERTEX_COUNT = 2000; // Default 2000

	// Map of [VERTEX_COUNT, VERTEX_COUNT]
	std::vector<float> vertices(VERTEX_COUNT * VERTEX_COUNT * 3);
	std::vector<float> texture_coords(VERTEX_COUNT * VERTEX_COUNT * 2);
	std::vector<unsigned int> indices(6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1));

	int vertex_pointer = 0;
	for(float i = 0; i < VERTEX_COUNT; i++) { // z
		for(float j = 0; j < VERTEX_COUNT; j++) { // x
			vertices[vertex_pointer * 3] = (float)j / ((float)VERTEX_COUNT - 1) * SIZE;
			vertices[vertex_pointer * 3 + 1] = 0;
			vertices[vertex_pointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * SIZE;

			texture_coords[vertex_pointer * 2] = (float)j / ((float)VERTEX_COUNT - 1);
			texture_coords[vertex_pointer * 2 + 1] = (float)i / ((float)VERTEX_COUNT - 1);
			vertex_pointer++;
		}
	}

	int pointer = 0;
	for(int gz = 0; gz < VERTEX_COUNT - 1; gz++) {
		for(int gx = 0; gx < VERTEX_COUNT - 1; gx++) {
			int top_left = (gz * VERTEX_COUNT) + gx;
			int top_right = top_left + 1;
			int bottom_left = ((gz + 1) * VERTEX_COUNT) + gx;
			int bottom_right = bottom_left + 1;
			indices[pointer++] = top_left;
			indices[pointer++] = bottom_left;
			indices[pointer++] = top_right;
			indices[pointer++] = top_right;
			indices[pointer++] = bottom_left;
			indices[pointer++] = bottom_right;
		}
	}

	unsigned int vao, pVBO, nVBO, tcVBO, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &pVBO);
	glGenBuffers(1, &nVBO);
	glGenBuffers(1, &tcVBO);
	glGenBuffers(1, &ibo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Positions
	glBindBuffer(GL_ARRAY_BUFFER, pVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Tex coords
	glBindBuffer(GL_ARRAY_BUFFER, tcVBO);
	glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(float), texture_coords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	terrain_shader->use();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, height_map->get_ID());
	terrain_shader->set_texture("height_map", 1);

	terrain_shader->set_float("AMPLITUDE", AMPLITUDE);

	glm::mat4 projection;
	
	// Render loop
	while(!glfwWindowShouldClose(window)) {
		// Per frame time logic
		float current_frame = glfwGetTime();
		delta = current_frame - last_frame;
		last_frame = current_frame;

		// Check for inputs, etc
		process_input_camera(window);

		// Rendering here
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // State-Setting function
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // State-using function

		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100000.0f);
		glm::mat4 view = camera.get_view_matrix();
		glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));

		// Set up Shader
		terrain_shader->use();
		terrain_shader->set_mat4("projection", projection);
		terrain_shader->set_mat4("view", view);
		terrain_shader->set_mat4("model", model);
		terrain_shader->set_vec3("lightPosition", light->position);
		terrain_shader->set_vec3("lightColor", light->color);
		terrain_shader->set_float("shineDamper", 1);
		terrain_shader->set_float("reflectivity", 0);

		// Bind the mountain/grass texture
		terrain_shader->set_int("tex", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->get_ID());

		// Draw the vertices/indices (send them to the graphics card to be processed)
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		// Checks if any events are triggered(like keyboard input or mouse movement events), 
		// updates window states, calls corresponding functions
		glfwPollEvents();

		// Swaps the color buffer that has been used to draw in during this iteration and show it as output to the screen
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void create_height_map(float noiseWidth, float noiseHeight, float vertWidth, float vertHeight) {
	// Produces 3D ridged multifractal noise, similar to mountains
	module::RidgedMulti base_mountain_terrain;
	module::Voronoi plateau_terrain;

#if 1
	// Generates "Billowy" noise suitable for clouds and rocks
	module::Billow base_flat_terrain;
	base_flat_terrain.SetFrequency(2.0);
#endif

#if 0
	module::Spheres base_flat_terrain;
	base_flat_terrain.SetFrequency(2.0);

#endif

#if 0
	// Produces polygon-like formations
	module::Voronoi base_flat_terrain;
	base_flat_terrain.SetFrequency(2.0);
	base_flat_terrain.SetDisplacement(0.25);
#endif

	// Applies a scaling factor to the output value from the source module
	// Scales the flat terrain, adds noise to it
	module::ScaleBias flat_terrain;
	flat_terrain.SetSourceModule(0, base_flat_terrain);
	flat_terrain.SetScale(1.000); // Default is 1
	flat_terrain.SetBias(-0.75); // Default is 0
	
	module::Perlin terrain_type;

	module::Select terrain_selector;
	terrain_selector.SetSourceModule(0, flat_terrain);
	terrain_selector.SetSourceModule(1, base_mountain_terrain);
	terrain_selector.SetControlModule(terrain_type);
	terrain_selector.SetBounds(0.0, 500); //1000
	terrain_selector.SetEdgeFalloff(0.125); // .125

	// pseudo-random displacement of the input value
	module::Turbulence final_terrain;
	final_terrain.SetSourceModule(0, terrain_selector);
	final_terrain.SetFrequency(2.0); // How rapidly the displacement changes
	final_terrain.SetPower(0.125); // The scaling factor that is applied to the displacement amount

	// Output the noise map
	utils::NoiseMap height_map;
	utils::NoiseMapBuilderPlane height_map_builder;
	height_map_builder.SetSourceModule(final_terrain);
	height_map_builder.SetDestNoiseMap(height_map);
	height_map_builder.SetDestSize(noiseWidth, noiseHeight);
	height_map_builder.SetBounds(0, vertWidth, 0, vertHeight);
	height_map_builder.Build();

	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(height_map);
	renderer.SetDestImage(image);
	renderer.Render();

	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename("res/heightmap.bmp");
	writer.WriteDestFile();
}

// MARK: 
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void process_input_camera(GLFWwindow* window) {
	float cameraSpeed = 800.0f;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.process_keyboard(FORWARD, delta);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.process_keyboard(BACKWARD, delta);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.process_keyboard(LEFT, delta);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.process_keyboard(RIGHT, delta);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.process_keyboard(UP, delta);
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.process_keyboard(DOWN, delta);

	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.movement_speed = cameraSpeed * 4;
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
		camera.movement_speed = cameraSpeed;

	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if(first_mouse) {
		mouse_last_X = xpos;
		mouse_last_Y = ypos;
		first_mouse = false;
	}

	float xOffset = xpos - mouse_last_X;
	float yOffset = mouse_last_Y - ypos; // Reversed b/c y-coordinates range from bottom to top

	mouse_last_X = xpos;
	mouse_last_Y = ypos;

	camera.process_mouse_movement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.process_mouse_scroll(yoffset);
}
#endif