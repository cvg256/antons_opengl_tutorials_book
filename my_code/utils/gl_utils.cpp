#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "gl_utils.h"

#define GL_LOG_FILE "gl.log"

static double g_previous_seconds = 0;


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

void UpdateFPSCounter(GLFWwindow *window, const char* title) {

	static int frame_count = 0;

	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - g_previous_seconds;

	//limit text updates to 4 per second
	if (elapsed_seconds > 0.25) {
		g_previous_seconds = current_seconds;

		char tmp[256];
		double fps = (double)frame_count / elapsed_seconds;
		double seconds_per_frame = elapsed_seconds / (double)frame_count;

		sprintf(tmp, "%s @ fps: %.2f, spf: %.4f", title, fps, seconds_per_frame);
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
