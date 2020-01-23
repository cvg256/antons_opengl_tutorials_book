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


#define WINDOW_TITLE "Phong"

int main() {

	//-----------START OPENGL------------------
	LogRestart();
	StartGL(WINDOW_TITLE);

	//-----------CREATE GEOMETRY------------------
	GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };

	float normals[] = {
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	};

	GLuint points_vbo;
	glGenBuffers( 1, &points_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), points, GL_STATIC_DRAW );

	GLuint normals_vbo;
	glGenBuffers( 1, &normals_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, normals_vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), normals, GL_STATIC_DRAW );

	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glBindBuffer( GL_ARRAY_BUFFER, normals_vbo );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

	//-----------CREATE SHADERS------------------
	GLuint shader_program = CreateProgramFromFiles("test_vs.glsl", "test_fs.glsl");
	glUseProgram(shader_program);

	int view_mat_location = glGetUniformLocation( shader_program, "view_mat" );
	int proj_mat_location =glGetUniformLocation( shader_program, "projection_mat" );
	int model_mat_location = glGetUniformLocation( shader_program, "model_mat" );


	//-----------CREATE CAMERA------------------
	// input variables
	float near = 0.1f;									// clipping plane
	float far = 100.0f;									// clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
	float aspect = (float)g_window_width / (float)g_window_height; // aspect ratio
	// matrix components
	float inverse_range = 1.0f / tan( fov * 0.5f );
	float Sx = inverse_range / aspect;
	float Sy = inverse_range;
	float Sz = -( far + near ) / ( far - near );
	float Pz = -( 2.0f * far * near ) / ( far - near );
	GLfloat proj_mat[] = { 
		Sx, 0.0f, 0.0f, 0.0f,
		0.0f, Sy, 0.0f, 0.0f,
		0.0f, 0.0f, Sz, -1.0f,
		0.0f, 0.0f, Pz, 0.0f};

	// create VIEW MATRIX
	float cam_pos[] = { 0.0f, 0.0f, 2.0f }; // don't start at zero, or we will be too close
	float cam_yaw = 0.0f;// y-rotation in degrees
	mat4 T =
		translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
	mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );
	mat4 view_mat = R * T;

	/* matrix for moving the triangle */
	mat4 model_mat = identity_mat4();

	//-----------SET RENDERING DEFAULTS------------------
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable( GL_DEPTH_TEST ); // enable depth-testing
	glDepthFunc( GL_LESS );		 // depth-testing interprets a smaller value as "closer"
	glEnable( GL_CULL_FACE ); // cull face
	glCullFace( GL_BACK );		// cull back face
	glFrontFace( GL_CW );			// GL_CCW for counter clock-wise

	//-----------RENDERING LOOP------------------
	glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
	glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat );
	glUniformMatrix4fv( model_mat_location, 1, GL_FALSE, model_mat.m );

	while ( !glfwWindowShouldClose( g_window ) ) {
		UpdateFPSCounter( g_window, WINDOW_TITLE );
		double current_seconds = glfwGetTime();

		// wipe the drawing surface clear
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glViewport( 0, 0, g_window_width, g_window_height );

		glUseProgram( shader_program );

		model_mat.m[12] = sinf( current_seconds ); //for periodical movement
		glUniformMatrix4fv( model_mat_location, 1, GL_FALSE, model_mat.m );

		glBindVertexArray( vao );
		// draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays( GL_TRIANGLES, 0, 3 );
		// update other events like input handling
		glfwPollEvents();
		if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( g_window, 1 );
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers( g_window );
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}
