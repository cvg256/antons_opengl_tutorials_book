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

// indicates which sphere is selected
int g_selected_sphere = -1;

// takes mouse position on screen and return ray in world coords
vec3 GetRayFromMousePos( float mouse_x, float mouse_y ) {
  // screen space (viewport coordinates)
  float x = ( 2.0f * mouse_x ) / g_window_width - 1.0f;
  float y = 1.0f - ( 2.0f * mouse_y ) / g_window_height;
  float z = 1.0f;

  // normalised device space
  vec3 ray_nds = vec3( x, y, z );
  
  // clip space
  vec4 ray_clip = vec4( ray_nds.v[0], ray_nds.v[1], -1.0, 1.0 );
  
  // eye space
  vec4 ray_eye = inverse( g_proj_mat ) * ray_clip;
  ray_eye      = vec4( ray_eye.v[0], ray_eye.v[1], -1.0, 0.0 );
  
  // world space
  vec3 ray_world = vec3( inverse( g_view_mat ) * ray_eye );
  
  // don't forget to normalise the vector at some point
  ray_world = normalise( ray_world );
  return ray_world;
}


// check if a ray and a sphere intersect. if not hit, returns false. it rejects
// intersections behind the ray caster's origin, and sets intersection_distance to
// the closest intersection
bool RaySphere( 
	vec3 ray_origin_wor, 
	vec3 ray_direction_wor, 
	vec3 sphere_centre_wor, 
	float sphere_radius, 
	float* intersection_distance ) 
{
  // work out components of quadratic
  vec3 dist_to_sphere     = ray_origin_wor - sphere_centre_wor;
  float b                 = dot( ray_direction_wor, dist_to_sphere );
  float c                 = dot( dist_to_sphere, dist_to_sphere ) - sphere_radius * sphere_radius;
  float b_squared_minus_c = b * b - c;
  // check for "imaginary" answer. == ray completely misses sphere
  if ( b_squared_minus_c < 0.0f ) { return false; }
  // check for ray hitting twice (in and out of the sphere)
  if ( b_squared_minus_c > 0.0f ) {
    // get the 2 intersection distances along ray
    float t_a              = -b + sqrt( b_squared_minus_c );
    float t_b              = -b - sqrt( b_squared_minus_c );
    *intersection_distance = t_b;
    // if behind viewer, throw one or both away
    if ( t_a < 0.0 ) {
      if ( t_b < 0.0 ) { return false; }
    } else if ( t_b < 0.0 ) {
      *intersection_distance = t_a;
    }

    return true;
  }
  // check for ray hitting once (skimming the surface)
  if ( 0.0f == b_squared_minus_c ) {
    // if behind viewer, throw away
    float t = -b + sqrt( b_squared_minus_c );
    if ( t < 0.0f ) { return false; }
    *intersection_distance = t;
    return true;
  }
  // note: could also check if ray origin is inside sphere radius
  return false;
}


void GLFWMouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
  // Note: could query if window has lost focus here
  if ( GLFW_PRESS == action ) {
    double xpos, ypos;
    glfwGetCursorPos( g_window, &xpos, &ypos );
    // work out ray
    vec3 ray_world = GetRayFromMousePos( (float)xpos, (float)ypos );
    // check ray against all spheres in scene
    int closest_sphere_clicked = -1;
    float closest_intersection = 0.0f;
    for ( int i = 0; i < NUM_SPHERES; i++ ) {
      float t_dist = 0.0f;
      if ( RaySphere( g_cam_pos, ray_world, g_sphere_pos_world[i], g_sphere_radius, &t_dist ) ) {
        // if more than one sphere is in path of ray, only use the closest one
        if ( -1 == closest_sphere_clicked || t_dist < closest_intersection ) {
          closest_sphere_clicked = i;
          closest_intersection   = t_dist;
        }
      }
    } // endfor
    g_selected_sphere = closest_sphere_clicked;
    printf( "sphere %i was clicked\n", closest_sphere_clicked );
  }
}

void UpdatePerspective() {
  // input variables
  float near   = 0.1f;                                                           // clipping plane
  float far    = 100.0f;                                                         // clipping plane
  float fovy   = 67.0f;                                                          // 67 degrees
  float aspect = (float)g_framebuffer_width / (float)g_framebuffer_height; // aspect ratio
  g_proj_mat     = perspective( fovy, aspect, near, far );
}

int main() {

	//-----------START OPENGL------------------
	LogRestart();
	StartGL(WINDOW_TITLE);
	glfwSetMouseButtonCallback(g_window, GLFWMouseClickCallback);

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
	GLuint shader_program = CreateProgramFromFiles("test_vs.glsl", "test_fs.glsl");
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

	bool wireframe_mode = false;

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

		for (int i = 0; i < NUM_SPHERES; ++i) {

			GLfloat blue = 0.0;
			if (g_selected_sphere == i) {
				blue = 1.0f;
			}

			glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_mats[i].m);
			glUniform1f(blue_loc, blue);
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