#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#define GL_LOG_FILE "gl.log"
#define WINDOW_TITLE "Extended Hello Triangle"

int g_window_width = 640;
int g_window_height = 480;

int g_framebuffer_width = g_window_width;
int g_framebuffer_height = g_window_height;

double g_previous_seconds = 0;

bool LogRestart() {
	FILE* file = fopen(GL_LOG_FILE, "w");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE log file for writing: %s\n", GL_LOG_FILE);
		return false;
	}

	time_t now = time(NULL);
	char* date = ctime(&now);
	fprintf(file, "GL_LOG_FILE log. local time %s\n", date);

	fclose(file);

	return true;
}

bool LogWrite(const char* message, ...) {
	va_list argptr;

	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE log file for appending: %s\n", GL_LOG_FILE);
		return false;
	}

	va_start (argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);

	va_start (argptr, message);
	vfprintf(stdout, message, argptr);
	va_end(argptr);

	fclose(file);
	return true;
}


bool LogError(const char* message, ...) {
	va_list argptr;

	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE log file for appending: %s\n", GL_LOG_FILE);
		return false;
	}

	va_start (argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);

	va_start (argptr, message);
	vfprintf(stderr, message, argptr);
	va_end(argptr);

	fclose(file);
	return true;
}

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

void LogGLParams() {
	GLenum params[] = {
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, //0
		GL_MAX_CUBE_MAP_TEXTURE_SIZE, //1
		GL_MAX_DRAW_BUFFERS, //2
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, //3
		GL_MAX_TEXTURE_IMAGE_UNITS, //4
		GL_MAX_TEXTURE_SIZE, //5
		GL_MAX_VARYING_FLOATS, //6
		GL_MAX_VERTEX_ATTRIBS, //7
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, //8
		GL_MAX_VERTEX_UNIFORM_COMPONENTS, //9
		GL_MAX_VIEWPORT_DIMS, //10
		GL_STEREO, //11
	};
	const char *names[] = {
		"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
		"GL_MAX_CUBE_MAP_TEXTURE_SIZE",
		"GL_MAX_DRAW_BUFFERS",
		"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
		"GL_MAX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_TEXTURE_SIZE",
		"GL_MAX_VARYING_FLOATS",
		"GL_MAX_VERTEX_ATTRIBS",
		"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_VERTEX_UNIFORM_COMPONENTS",
		"GL_MAX_VIEWPORT_DIMS",
		"GL_STEREO",
	};
	LogWrite( "GL Context Params:\n" );
	// integers - only works if the order is 0-10 integer return types
	for (int i = 0; i < 10; i++) {
		int v = 0;
		glGetIntegerv( params[i], &v );
		LogWrite( "%s %i\n", names[i], v );
	}
	
	// others
	int v[2];
	v[0] = v[1] = 0;

	//GL_MAX_VIEWPORT_DIMS
	glGetIntegerv( params[10], v );
	LogWrite( "%s %i %i\n", names[10], v[0], v[1] );
	
	//GL_STEREO
	unsigned char s = 0;
	glGetBooleanv( params[11], &s );
	LogWrite( "%s %i\n", names[11], (unsigned int)s );
	
	LogWrite( "-----------------------------\n" );
}

void UpdateFPSCounter(GLFWwindow *window) {

	static int frame_count = 0;

	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - g_previous_seconds;

	//limit text updates to 4 per second
	if (elapsed_seconds > 0.25) {
		g_previous_seconds = current_seconds;

		char tmp[256];
		double fps = (double)frame_count / elapsed_seconds;
		double seconds_per_frame = elapsed_seconds / (double)frame_count;

		sprintf(tmp, "%s @ fps: %.2f, spf: %.4f", WINDOW_TITLE, fps, seconds_per_frame);
		glfwSetWindowTitle(window, tmp);

		frame_count = 0;
	}

	frame_count++;
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

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	const char* vertex_shader = 
	"#version 410\n" //use version 150 for OpenGL 3.2, 330 - OpenGL 3.3
	"in vec3 vp;\n"
	"void main() {\n"
	"	gl_Position = vec4(vp, 1.0);"
	"}\n";

	const char* fragment_shader = 
	"#version 410\n" //use version 150 for OpenGL 3.2, 330 - OpenGL 3.3
	"out vec4 frag_color;\n"
	"void main() {\n"
	"	frag_color = vec4(0.5, 0.0, 0.5, 1.0);"
	"}\n";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, fs);
	glAttachShader(shader_program, vs);
	glLinkProgram(shader_program);

	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

	while(!glfwWindowShouldClose(window)) {

		UpdateFPSCounter(window);

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