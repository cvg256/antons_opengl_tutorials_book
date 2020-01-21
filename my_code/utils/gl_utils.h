#pragma once


extern GLFWwindow *g_window;
extern int g_window_width;
extern int g_window_height;
extern int g_framebuffer_width;
extern int g_framebuffer_height;


bool LogRestart();
bool LogWrite(const char* message, ...);
bool LogError(const char* message, ...);
void LogGLParams();
void UpdateFPSCounter(GLFWwindow *window, const char* title);
const char *GLTypeToString( GLenum type );
void PrintShaderInfoLog(GLuint shader_index);
void PrintProgramInfoLog(GLuint program_index);
void PrintProgramInfoAll(GLuint program_index);
bool CheckShaderCompileStatus(GLuint shader_index);
bool CheckProgramLinkStatus(GLuint program_index);
bool ValidateProgram(GLuint program_index);
bool LoadShaderSourceFromFile( const char *file_name, GLchar* shader_str, int max_len);
bool StartGL(const char* window_title);

