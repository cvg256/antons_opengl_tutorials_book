#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "../utils/maths_funcs.h"
#include "../utils/gl_utils.h"
#include "../utils/obj_parser.h"


#define WINDOW_TITLE "Ray Picking"

#define NUM_SPHERES 4
#define MESH_FILE "sphere.obj"

mat4 g_view_mat;
mat4 g_proj_mat;
vec3 g_cam_pos( 0.0f, 0.0f, 5.0f );

// a world position for each sphere in the scene
vec3 g_sphere_pos_world[]  = {vec3( -2.0, 0.0, 0.0 ), 
							vec3( 2.0, 0.0, 0.0 ), 
							vec3( -2.0, 0.0, -2.0 ), 
							vec3( 1.5, 1.0, -1.0 ) };
const float g_sphere_radius = 1.0f;



void UpdatePerspective() {
  // input variables
  float near   = 0.1f;                                                           // clipping plane
  float far    = 100.0f;                                                         // clipping plane
  float fovy   = 67.0f;                                                          // 67 degrees
  float aspect = (float)g_framebuffer_width / (float)g_framebuffer_height; // aspect ratio
  g_proj_mat     = perspective( fovy, aspect, near, far );
}

int main() {
	LogRestart();
	StartGL(WINDOW_TITLE);

	//-----------CREATE GEOMETRY------------------
	GLfloat* vp = NULL; // vertex points
	GLfloat* vn = NULL; // vertex normals
	GLfloat* vt = NULL; // vertex texture coordinates

	int point_count = 0;
	if(!load_obj_file(MESH_FILE, vp, vt, vn, point_count)) {
		LogError("ERROR: Can't load mesh file '%s'\n", MESH_FILE);
		return 1;
	}

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint points_vbo = 0;
	if (vp != NULL) {
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * point_count * sizeof(GLfloat), vp, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	}



	//-----------CREATE SHADERS------------------
	//TODO: move to separate function CreateProgrammFromFiles(vs, fs)
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
	glDeleteShader(vs);
	glDeleteShader(fs);

	PrintProgramInfoAll(shader_program);

	glUseProgram(shader_program);


	GLint model_mat_loc  = glGetUniformLocation(shader_program, "model");
	assert(model_mat_loc > -1);

	GLint proj_mat_loc  = glGetUniformLocation(shader_program, "proj");
	assert(proj_mat_loc > -1);

	GLint view_mat_loc  = glGetUniformLocation(shader_program, "view");
	assert(view_mat_loc > -1);

	GLint blue_loc  = glGetUniformLocation(shader_program, "blue");
	assert(blue_loc > -1);


	//-----------CREATE CAMERA------------------
	UpdatePerspective();

	float cam_speed         = 3.0f;  // 1 unit per second
	float cam_heading_speed = 50.0f; // 30 degrees per second
	float cam_heading       = 0.0f;  // y-rotation in degrees
	mat4 T                  = translate( identity_mat4(), vec3( -g_cam_pos.v[0], -g_cam_pos.v[1], -g_cam_pos.v[2] ) );
	mat4 R                  = rotate_y_deg( identity_mat4(), -cam_heading );
	versor q                = quat_from_axis_deg( -cam_heading, 0.0f, 1.0f, 0.0f );
	g_view_mat              = R * T;
	// keep track of some useful vectors that can be used for keyboard movement
	vec4 fwd( 0.0f, 0.0f, -1.0f, 0.0f );
	vec4 rgt( 1.0f, 0.0f, 0.0f, 0.0f );
	vec4 up( 0.0f, 1.0f, 0.0f, 0.0f );

	//-----------SET RENDERING DEFAULTS------------------

	mat4 model_mats[NUM_SPHERES];
	for (int i = 0; i < NUM_SPHERES; ++i) {
		model_mats[i] = translate(identity_mat4(), g_sphere_pos_world[i]);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	bool wireframe_mode = true;

	if (wireframe_mode) {
		//wireframe draw
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW); //GL_CW clock wise, GL_CCW counter clock-wise
	}


	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);


	//-----------RENDERING LOOP------------------
	double previous_seconds = glfwGetTime();

	// glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_mat);
	glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, g_view_mat.m);
	glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, g_proj_mat.m);


	while(!glfwWindowShouldClose(g_window)) {

		double current_seconds = glfwGetTime();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;
		UpdateFPSCounter(g_window, WINDOW_TITLE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_framebuffer_width, g_framebuffer_height);

		glUseProgram(shader_program);
		glUniformMatrix4fv( view_mat_loc, 1, GL_FALSE, g_view_mat.m );
		glUniformMatrix4fv( proj_mat_loc, 1, GL_FALSE, g_proj_mat.m );
		glBindVertexArray(vao);

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		for (int i = 0; i < NUM_SPHERES; ++i) {
			glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_mats[i].m);
			glUniform1f(blue_loc, 0.0);

			glDrawArrays(GL_TRIANGLES, 0, point_count);
		}

		glfwPollEvents();

		// control keys
		bool cam_moved = false;
		vec3 move( 0.0, 0.0, 0.0 );
		float cam_yaw   = 0.0f; // y-rotation in degrees
		float cam_pitch = 0.0f;
		float cam_roll  = 0.0;
		
		if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
			move.v[0] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		
		if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
			move.v[0] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_Q ) ) {
			move.v[1] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_E ) ) {
			move.v[1] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
			move.v[2] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
			move.v[2] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
			cam_yaw += cam_heading_speed * elapsed_seconds;
			cam_moved    = true;
			versor q_yaw = quat_from_axis_deg( cam_yaw, up.v[0], up.v[1], up.v[2] );
			q            = q_yaw * q;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
			cam_yaw -= cam_heading_speed * elapsed_seconds;
			cam_moved    = true;
			versor q_yaw = quat_from_axis_deg( cam_yaw, up.v[0], up.v[1], up.v[2] );
			q            = q_yaw * q;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_UP ) ) {
			cam_pitch += cam_heading_speed * elapsed_seconds;
			cam_moved      = true;
			versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
			q              = q_pitch * q;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_DOWN ) ) {
			cam_pitch -= cam_heading_speed * elapsed_seconds;
			cam_moved      = true;
			versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
			q              = q_pitch * q;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_Z ) ) {
			cam_roll -= cam_heading_speed * elapsed_seconds;
			cam_moved     = true;
			versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
			q             = q_roll * q;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_C ) ) {
			cam_roll += cam_heading_speed * elapsed_seconds;
			cam_moved     = true;
			versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
			q             = q_roll * q;
		}
		// update view matrix
		if ( cam_moved ) {
			// re-calculate local axes so can move fwd in dir cam is pointing
			R   = quat_to_mat4( q );
			fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
			rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
			up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );

			g_cam_pos = g_cam_pos + vec3( fwd ) * -move.v[2];
			g_cam_pos = g_cam_pos + vec3( up ) * move.v[1];
			g_cam_pos = g_cam_pos + vec3( rgt ) * move.v[0];
			mat4 T  = translate( identity_mat4(), vec3( g_cam_pos ) );

			g_view_mat = inverse( R ) * inverse( T );
		}

		if(GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(g_window, 1);
		}

		glfwSwapBuffers(g_window);

	}


	// close GL context and any other GLFW resources
	glfwTerminate();

	LogWrite("GLFW terminated\n");

	return 0;
}