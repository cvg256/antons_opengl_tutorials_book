#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "../utils/maths_funcs.h"
#include "../utils/gl_utils.h"

#define WINDOW_TITLE "Virtual Camera"

static int g_window_width = 640;
static int g_window_height = 480;

static int g_framebuffer_width = g_window_width;
static int g_framebuffer_height = g_window_height;

void GLFWErrorCallback(int error, const char* description) {
	LogError("GLFW ERROR: code %i msg: %s\n", error, description);

}

void GLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
	LogWrite("GLFWWindowSizeBack: %i x %i\n", width, height);
	g_window_width = width;
	g_window_height = height;

}

void GLFWFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	LogWrite("GLFWFramebufferSizeCallback: %i x %i\n", width, height);
	g_framebuffer_width = width;
	g_framebuffer_height = height;

}

int main() {

	if (!LogRestart()){
		//quit?
	}

	LogWrite("starting GLFW %s\n", glfwGetVersionString());

	glfwSetErrorCallback(GLFWErrorCallback);

	if(!glfwInit()) {
		LogError("ERROR: could not start GLFW3\n");
		return 1;
	}

	//this is needed for macOS
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//

	GLFWwindow* window = glfwCreateWindow(g_window_width, g_window_height, WINDOW_TITLE, NULL, NULL);

	if (!window)
	{
		LogError("ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}

	glfwSetWindowSizeCallback(window, GLFWWindowSizeCallback);
	glfwSetFramebufferSizeCallback(window, GLFWFramebufferSizeCallback);

	glfwMakeContextCurrent(window);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	LogGLParams();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);

	LogWrite("Renderer: %s\n", renderer);
	LogWrite("OpenGL version supported: %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLfloat points[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};

	GLfloat colors[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};

	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	GLuint colors_vbo = 0;
	glGenBuffers(1, &colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);


	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	constexpr int max_vertex_source_size = 1024 * 256;
	GLchar vertex_shader[max_vertex_source_size];
	GLchar fragment_shader[max_vertex_source_size];

	LoadShaderSourceFromFile("test_vs.glsl", vertex_shader, max_vertex_source_size);
	LoadShaderSourceFromFile("test_fs.glsl", fragment_shader, max_vertex_source_size);

	const GLchar *p = NULL;

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	p = (const GLchar *)vertex_shader;
	glShaderSource(vs, 1, &p, NULL);
	glCompileShader(vs);
	CheckShaderCompileStatus(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	p = (const GLchar *)fragment_shader;
	glShaderSource(fs, 1, &p, NULL);
	glCompileShader(fs);
	CheckShaderCompileStatus(fs);

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, fs);
	glAttachShader(shader_program, vs);
	glLinkProgram(shader_program);

	CheckProgramLinkStatus(shader_program);
	ValidateProgram(shader_program);

	PrintProgramInfoAll(shader_program);

	glUseProgram(shader_program);

	GLint model_matrix_loc  = glGetUniformLocation(shader_program, "model");
	assert(model_matrix_loc > -1);

	GLint proj_matrix_loc  = glGetUniformLocation(shader_program, "proj");
	assert(proj_matrix_loc > -1);

	GLint view_matrix_loc  = glGetUniformLocation(shader_program, "view");
	assert(view_matrix_loc > -1);

	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

	//wireframe draw
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW); //GL_CW clock wise, GL_CCW counter clock-wise


	float cam_speed = 1.0f;
	float cam_yaw_speed = 10.0f;
	float cam_pos[] = {0.0f, 0.0f, 2.0f}; //don't start at zero or we will be too close
	float cam_yaw = 0.0f;

	mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
	mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
	mat4 view_mat = R * T;

	float near = 0.1; //clipping plane
	float far = 100.f; //clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; //in radians
	float aspect = (float)g_framebuffer_width/(float)g_framebuffer_height;

	float range = tan(fov * 0.5f) * near;
	float Sx = (2.0f * near) / (range * aspect + range * aspect);
	float Sy = near / range;
	float Sz = -(far + near) / (far - near);
	float Pz = -(2.0f * far * near) / (far - near);

	float proj_mat[] = {
		  Sx, 0.0f, 0.0f,  0.0f,
		0.0f,   Sy, 0.0f,  0.0f,
		0.0f, 0.0f,   Sz, -1.0f,
		0.0f, 0.0f,   Pz,  0.0f,
	};

	float model_mat[] = {
		1.0f, 0.0f, 0.0f, 0.0f, //1st column
		0.0f, 1.0f, 0.0f, 0.0f, //2nd column
		0.0f, 0.0f, 1.0f, 0.0f, //3rd column
		0.5f, 0.0f, 0.0f, 1.0f, //4th column
	};


	float move_speed = 0.7f;
	constexpr float move_range = 0.5f;

	float last_position = 0.0f;

	double previous_seconds = glfwGetTime();

	glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_mat);
	glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(proj_matrix_loc, 1, GL_FALSE, proj_mat);


	while(!glfwWindowShouldClose(window)) {

		UpdateFPSCounter(window, WINDOW_TITLE);


		double current_seconds = glfwGetTime();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		if (fabs(last_position) > move_range) {
			move_speed = -move_speed;
		}

		float x = elapsed_seconds * move_speed + last_position;
		last_position = x;

		model_mat[12] = x;
		glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_mat);


		{
			bool cam_moved = false;
			if(glfwGetKey(window, GLFW_KEY_A)) {
				cam_pos[0] -= cam_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_D)) {
				cam_pos[0] += cam_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_PAGE_UP)) {
				cam_pos[1] += cam_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN)) {
				cam_pos[1] -= cam_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_W)) {
				cam_pos[2] -= cam_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_S)) {
				cam_pos[2] += cam_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_LEFT)) {
				cam_yaw += cam_yaw_speed * elapsed_seconds;
				cam_moved = true;
			}

			if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
				cam_yaw -= cam_yaw_speed * elapsed_seconds;
				cam_moved = true;
			}

			if (cam_moved) {
				mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
				mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
				mat4 view_mat = R * T;

				glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_mat.m);

			}

		}


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_framebuffer_width, g_framebuffer_height);

		glUseProgram(shader_program);
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwPollEvents();
		glfwSwapBuffers(window);

		if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(window, 1);
		}
	}


	// close GL context and any other GLFW resources
	glfwTerminate();

	LogWrite("GLFW terminated\n");

	return 0;
}