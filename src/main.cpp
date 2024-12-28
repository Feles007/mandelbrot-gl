#include <iostream>
#include <fstream>
#include <iterator>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "fstd/num.hpp"


const bool use_double = false;

int window_width = 1280;
int window_height = 720;

f64 center_x = 0;
f64 center_y = 0;
f64 scale = 1;
u32 iterations = 1024;

f64 previous_time = 0;
f64 delta_time = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	const f64 scaler = 1 + delta_time;

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		scale *= scaler;
	} else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		scale /= scaler;
	}

	const f64 movement = delta_time * (1 / scale);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		center_x -= movement;
	} else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		center_x += movement;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		center_y += movement;
	} else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		center_y -= movement;
	}
}

class ShaderLoader {
private:
	unsigned vertex_shader;
	unsigned fragment_shader;

	unsigned shader_program;

public:
	
	ShaderLoader(const char *vertex_shader_path, const char *fragment_shader_path) {

		
		std::string vertex_shader_source;
		std::string fragment_shader_source;
		
		std::ifstream file;

		file.open(vertex_shader_path);
		vertex_shader_source = std::string(
			std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()
		);
		file.close();

		file.open(fragment_shader_path);
		fragment_shader_source = std::string(
			std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()
		);
		file.close();

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		
		const char *source[1];
		int length[1];
		
		source[0] = vertex_shader_source.data();
		length[0] = static_cast<int>(vertex_shader_source.length());
		glShaderSource(vertex_shader, 1, source, length);
		source[0] = fragment_shader_source.data();
		length[0] = static_cast<int>(fragment_shader_source.length());
		glShaderSource(fragment_shader, 1, source, length);

		int result;
		char message_buffer[512];

		glCompileShader(vertex_shader);
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
		if(!result) {
			glGetShaderInfoLog(vertex_shader, sizeof(message_buffer), NULL, message_buffer);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << message_buffer << std::endl;
		}
		glCompileShader(fragment_shader);
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
		if(!result) {
			glGetShaderInfoLog(fragment_shader, sizeof(message_buffer), NULL, message_buffer);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << message_buffer << std::endl;
		}

		shader_program = glCreateProgram();

		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);
		glLinkProgram(shader_program);
	}
	~ShaderLoader() {
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);  
	}

	void use() {
		glUseProgram(shader_program);
	}
	unsigned get_shader_program() {
		return shader_program;
	}
};

template<typename T>
f64 f64_div(T a, T b) {
	return f64(a) / f64(b);
}

extern "C" {
	_declspec(dllexport) i32 NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "mandelbrot explorer (f32)", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(0);

	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, window_width, window_height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	f32 vertices[] = {
		-1, 3, 0,
		-1, -1, 0,
		3,  -1, 0
	};
	unsigned VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	auto shader_loader = ShaderLoader("shaders/shader.vert", use_double ? "shaders/double.frag" : "shaders/float.frag");

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned VAO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 3. then set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	const int uniform_window_width = glGetUniformLocation(shader_loader.get_shader_program(), "window_width");
	const int uniform_window_height = glGetUniformLocation(shader_loader.get_shader_program(), "window_height");
	const int uniform_iterations = glGetUniformLocation(shader_loader.get_shader_program(), "iterations");
	const int uniform_extents = glGetUniformLocation(shader_loader.get_shader_program(), "extents");

	glBindVertexArray(VAO);
	
	while(!glfwWindowShouldClose(window)) {
		f64 current_time = glfwGetTime();
		delta_time = current_time - previous_time;
		previous_time = current_time;

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		const f64 x_offset = -0.5;

		const f64 vmin = -1.15 * (1 / scale);
		const f64 vmax =  1.15 * (1 / scale);
		const f64 hmin = vmin * f64_div(window_width, window_height) + x_offset + center_x;
		const f64 hmax = vmax * f64_div(window_width, window_height) + x_offset + center_x;

		const f64 offset_vmin = vmin + center_y;
		const f64 offset_vmax = vmax + center_y;

		shader_loader.use();
		glUniform1ui(uniform_window_width, window_width);
		glUniform1ui(uniform_window_height, window_height);
		glUniform1ui(uniform_iterations, iterations);
		if (use_double) {
			glUniform4d(uniform_extents, offset_vmin, offset_vmax, hmin, hmax);
		} else {
			glUniform4f(uniform_extents, offset_vmin, offset_vmax, hmin, hmax);
		}

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	glfwTerminate();
}
