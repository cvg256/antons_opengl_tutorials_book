#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#define GL_LOG_FILE "gl.log"
#define WINDOW_TITLE "VBO"

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

const char *GLTypeToString( GLenum type ) {
	switch ( type ) {
	case GL_BOOL:
		return "bool";
	case GL_INT:
		return "int";
	case GL_FLOAT:
		return "float";
	case GL_FLOAT_VEC2:
		return "vec2";
	case GL_FLOAT_VEC3:
		return "vec3";
	case GL_FLOAT_VEC4:
		return "vec4";
	case GL_FLOAT_MAT2:
		return "mat2";
	case GL_FLOAT_MAT3:
		return "mat3";
	case GL_FLOAT_MAT4:
		return "mat4";
	case GL_SAMPLER_2D:
		return "sampler2D";
	case GL_SAMPLER_3D:
		return "sampler3D";
	case GL_SAMPLER_CUBE:
		return "samplerCube";
	case GL_SAMPLER_2D_SHADOW:
		return "sampler2DShadow";
	default:
		break;
	}
	return "other";
}

void PrintShaderInfoLog(GLuint shader_index) {
	constexpr int max_length = 2048;
	int actual_length = 0;
	char log[max_length];
	glGetShaderInfoLog(shader_index, max_length, &actual_length, log);

	LogWrite("shader info log for GL index %u:\n%s\n", shader_index, log);
}

void PrintProgramInfoLog(GLuint program_index) {
	constexpr int max_length = 2048;
	int actual_length = 0;
	char log[max_length];
	glGetProgramInfoLog(program_index, max_length, &actual_length, log);

	LogWrite("program info log for GL index %u:\n%s\n", program_index, log);
}

void PrintProgramInfoAll(GLuint program_index) {
	LogWrite("-------------------------\nshader program %u info:\n", program_index);
	int params = -1;
	glGetProgramiv(program_index, GL_LINK_STATUS, &params);
	LogWrite("GL_LINK_STATUS = %i\n", params);

	glGetProgramiv(program_index, GL_ATTACHED_SHADERS, &params);
	LogWrite("GL_ATTACHED_SHADERS = %i\n", params);


	int active_attrib_count = 0;
	glGetProgramiv(program_index, GL_ACTIVE_ATTRIBUTES, &active_attrib_count);
	LogWrite("GL_ACTIVE_ATTRIBUTES = %i\n", active_attrib_count);

	for (int i = 0; i < active_attrib_count; ++i){
		constexpr int max_length = 64;
		char name[max_length];
		int actual_length = 0;
		int size = 0;
		GLenum type;

		GLuint attrib_index = (GLuint)i;
		glGetActiveAttrib(program_index, attrib_index, max_length, &actual_length, &size, &type, name);

		if (size > 1) {
			for (int j = 0; j < size; ++j) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);

				int location = glGetAttribLocation(program_index, long_name);
				LogWrite(" %i: type:%s name:%s location:%i\n", i, GLTypeToString(type), long_name, location);
			}
		} 
		else {
			int location = glGetAttribLocation(program_index, name);
			LogWrite(" %i: type:%s name:%s location:%i\n", i, GLTypeToString(type), name, location);
		}
	}

	int active_uniforms_count = 0;
	glGetProgramiv(program_index, GL_ACTIVE_UNIFORMS, &active_uniforms_count);
	LogWrite("GL_ACTIVE_UNIFORMS = %i\n", active_uniforms_count);

	for (int i = 0; i < active_uniforms_count; ++i) {
		constexpr int max_length = 64;
		char name[max_length];
		int actual_length = 0;
		int size = 0;
		GLenum type;

		GLuint uniform_index = (GLuint)i;
		glGetActiveUniform(program_index, uniform_index, max_length, &actual_length, &size, &type, name);

		if (size > 1) {
			for (int j = 0; j < size; ++j) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);

				int location = glGetUniformLocation(program_index, long_name);
				LogWrite(" %i: type:%s name:%s location:%i\n", i, GLTypeToString(type), long_name, location);
			}
		} 
		else {
			int location = glGetUniformLocation(program_index, name);
			LogWrite(" %i: type:%s name:%s location:%i\n", i, GLTypeToString(type), name, location);
		}
	}

	PrintProgramInfoLog(program_index);
}


bool CheckShaderCompileStatus(GLuint shader_index) {
	int params = 0;
	glGetShaderiv(shader_index, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE){
		LogError("ERROR: GL shader index %u did not compile\n", shader_index);
		PrintShaderInfoLog(shader_index);
		return false;
	}

	return true;
}

bool CheckProgramLinkStatus(GLuint program_index) {
	int params = 0;
	glGetProgramiv(program_index, GL_LINK_STATUS, &params);
	if (params != GL_TRUE){
		LogError("ERROR: could not link shader program GL index %u\n", program_index);
		PrintProgramInfoLog(program_index);
		return false;
	}

	return true;
}

bool ValidateProgram(GLuint program_index) {

	glValidateProgram(program_index);

	int params = 0;
	glGetProgramiv(program_index, GL_VALIDATE_STATUS, &params);
	LogWrite("program %u GL_VALIDATE_STATUS  = %i\n", program_index, params);

	if (params != GL_TRUE) {
		PrintProgramInfoLog(program_index);
		return false;
	}

	return true;
}

bool LoadShaderSourceFromFile( const char *file_name, GLchar* shader_str, int max_len ) {
	FILE *file = fopen( file_name, "r" );
	if ( !file ) {
		LogError( "ERROR: opening file for reading: %s\n", file_name );
		return false;
	}
	size_t cnt = fread( shader_str, 1, max_len - 1, file );
	if ( (int)cnt >= max_len - 1 ) {
		LogError( "WARNING: file %s too big - truncated.\n", file_name );
	}
	if ( ferror( file ) ) {
		LogError( "ERROR: reading shader file %s\n", file_name );
		fclose( file );
		return false;
	}

	// append \0 to end of file string
	shader_str[cnt] = 0;

	fclose( file );
	return true;
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

	GLfloat matrix[] = {
		1.0f, 0.0f, 0.0f, 0.0f, //1st column
		0.0f, 1.0f, 0.0f, 0.0f, //2nd column
		0.0f, 0.0f, 1.0f, 0.0f, //3rd column
		0.5f, 0.0f, 0.0f, 1.0f, //4th column
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

	GLint matrix_loc  = glGetUniformLocation(shader_program, "matrix");
	assert(matrix_loc > -1);

	glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);


	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

	//wireframe draw
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW); //GL_CW clock wise, GL_CCW counter clock-wise

	float speed = 1.0f;
	float last_position = 0.0f;

	double previous_seconds = glfwGetTime();

	while(!glfwWindowShouldClose(window)) {

		UpdateFPSCounter(window);


		double current_seconds = glfwGetTime();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		if (fabs(last_position) > 1.0f) {
			speed = -speed;
		}

		float x = elapsed_seconds * speed + last_position;
		last_position = x;

		matrix[12] = x;
		glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);



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