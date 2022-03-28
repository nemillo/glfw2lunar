#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sphere.hpp"
#include "plane.hpp"

#define GL_LOG_FILE "gl.log"
std::ofstream log_file;

std::ostream& operator<<(std::ostream& stream, const std::chrono::system_clock::time_point& point)
{
    auto time = std::chrono::system_clock::to_time_t(point);
    std::tm* tm = std::localtime(&time);
    char buffer[26];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.", tm);
    stream << buffer;
    auto duration = point.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    // remove microsecond cast line if you would like a higher resolution sub second time representation, then just stream remaining.count()
    auto micro = std::chrono::duration_cast<std::chrono::microseconds>(remaining);
    return stream << micro.count();
}

/* start a new log file. put the time and date at the top */
bool restart_gl_log() {
	log_file.open(GL_LOG_FILE);
	auto now = std::chrono::high_resolution_clock::now();
    log_file << GL_LOG_FILE << " Local time : " << now << std::endl;
	log_file.close();
	return true;
}

/* we will tell GLFW to run this function whenever it finds an error */
void glfw_error_callback( int error, const char *description ) {
	
}

// keep track of window size for things like the viewport and the mouse cursor
float g_gl_width = 1024.0;
float g_gl_height = 800.0;

/* we will tell GLFW to run this function whenever the framebuffer size is changed */
void glfw_framebuffer_size_callback( GLFWwindow *window, int width, int height ) {
	g_gl_width = float(width);
	g_gl_height = float(height);
	/* update any perspective matrices used here */
}

int main() {
	GLFWwindow *window;
	const GLubyte *renderer;
	const GLubyte *version;
	int sectorCount = 10;
	int stackCount = 10;
	float fps = 0.0f;
	float frame_time = 0.0f;
	float frame_time_cummulated = 0.0f;
	const float dt = 0.001;
	const float gravity = 9.80;
	const float R = 0.5f;

	Sphere sphere1;
	Sphere sphere2;
	Plane plane1;

	restart_gl_log();
	auto t_start = std::chrono::high_resolution_clock::now();
	
	log_file.open(GL_LOG_FILE,std::ios::app);
	log_file << "t_start: " << t_start << std::endl;
	log_file.close();
	
	GLuint vbo;
	GLuint vao;
	const char *vertex_shader = "#version 410\n"
		"in vec3 vp;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 proj;"
		"void main() {"
		"  gl_PointSize = 10.0;"
		"  gl_Position = proj * view * model * vec4( vp, 1.0 );"
		"}";

	const char *fragment_shader = "#version 410\n"
		"out vec4 frag_colour;"
		"void main() {"
		"  frag_colour = vec4( 0.5, 0.5, 0.5, 1.0 );"
		"}";
	GLuint shader_programme, vs, fs;

	// start GL context and O/S window using the GLFW helper library
	glfwSetErrorCallback( glfw_error_callback );
	if ( !glfwInit() ) {
		return 1;
	}
	// set anti-aliasing factor to make diagonal edges appear less jagged
	glfwWindowHint( GLFW_SAMPLES, 4 );

	/* we can run a full-screen window here */

	/*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
	const GLFWvidmode* vmode = glfwGetVideoMode (mon);
	GLFWwindow* window = glfwCreateWindow (
		vmode->width, vmode->height, "Extended GL Init", mon, NULL
	);*/

	window = glfwCreateWindow( g_gl_width, g_gl_height, "Extended Init", NULL, NULL );
	if ( !window ) {
		glfwTerminate();
		return 1;
	}
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
	//
	glfwMakeContextCurrent( window );

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	renderer = glGetString( GL_RENDERER ); // get renderer string
	version = glGetString( GL_VERSION );	 // version as a string
	
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable( GL_DEPTH_TEST ); // enable depth-testing
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDepthFunc( GL_LESS );		 // depth-testing interprets a smaller value as "closer"
	
	vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &vertex_shader, NULL );
	glCompileShader( vs );
	fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragment_shader, NULL );
	glCompileShader( fs );
	shader_programme = glCreateProgram();
	glAttachShader( shader_programme, fs );
	glAttachShader( shader_programme, vs );
	glLinkProgram( shader_programme );
	glUseProgram( shader_programme );
	
	GLuint vp = glGetAttribLocation(shader_programme, "vp");
	sphere1.init(vp,R);
	sphere1.setMass(1.0f);
	sphere1.setPosition(glm::vec3(1.0f,1.0f,2.0f));
	sphere1.setVelocity(glm::vec3(-1.0f,-0.5f,0.0f));
	sphere1.setAcceleration(glm::vec3(0.0f,0.0f,-gravity));

	sphere2.init(vp,R);
	sphere2.setMass(1.0f);
	sphere2.setPosition(glm::vec3(-1.0f,-1.0f,2.0f));
	sphere2.setVelocity(glm::vec3(0.0f,0.0f,0.0f));
	sphere2.setAcceleration(glm::vec3(0.0f,0.0f,-gravity));

	plane1.init(vp,0.0f);

	GLint uniModel = glGetUniformLocation(shader_programme, "model");

    // Set up projection
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, -5.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    GLint uniView = glGetUniformLocation(shader_programme, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), g_gl_width / g_gl_height, 1.0f, 10.0f);
    GLint uniProj = glGetUniformLocation(shader_programme, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	
	while ( !glfwWindowShouldClose( window ) ) {
		auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
		// wipe the drawing surface clear
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glViewport( 0, 0, g_gl_width, g_gl_height );

		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //sets the uniform matrix model in shader
		plane1.draw();

		sphere1.setVelocity(sphere1.getAcceleration()*frame_time + sphere1.getVelocity());
		sphere1.setPosition(sphere1.getVelocity()*frame_time + sphere1.getPosition());
		if (sphere1.getPosition().z <= R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere1.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.z = -oldVelocity.z;
			sphere1.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere1.getPosition();
			newPosition = oldPosition;
			newPosition.z = R;
			sphere1.setPosition(newPosition);
			
		}

		if (sphere1.getPosition().x <= -2+R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere1.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.x = -oldVelocity.x;
			sphere1.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere1.getPosition();
			newPosition = oldPosition;
			newPosition.x = -2+R;
			sphere1.setPosition(newPosition);
			
		}

		if (sphere1.getPosition().x >= 2-R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere1.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.x = -oldVelocity.x;
			sphere1.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere1.getPosition();
			newPosition = oldPosition;
			newPosition.x = 2-R;
			sphere1.setPosition(newPosition);
			
		}

		if (sphere1.getPosition().y <= -2+R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere1.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.y = -oldVelocity.y;
			sphere1.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere1.getPosition();
			newPosition = oldPosition;
			newPosition.y = -2+R;
			sphere1.setPosition(newPosition);
			
		}

		if (sphere1.getPosition().y >= 2-R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere1.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.y = -oldVelocity.y;
			sphere1.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere1.getPosition();
			newPosition = oldPosition;
			newPosition.y = 2-R;
			sphere1.setPosition(newPosition);
			
		}
		
		sphere2.setVelocity(sphere2.getAcceleration()*frame_time + sphere2.getVelocity());
		sphere2.setPosition(sphere2.getVelocity()*frame_time + sphere2.getPosition());
		if (sphere2.getPosition().z <= R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere2.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.z = -oldVelocity.z;
			sphere2.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere2.getPosition();
			newPosition = oldPosition;
			newPosition.z = R;
			sphere2.setPosition(newPosition);
			
		}

		if (sphere2.getPosition().x <= -2+R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere2.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.x = -oldVelocity.x;
			sphere2.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere2.getPosition();
			newPosition = oldPosition;
			newPosition.x = -2+R;
			sphere2.setPosition(newPosition);
			
		}

		if (sphere2.getPosition().x >= 2-R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere2.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.x = -oldVelocity.x;
			sphere2.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere2.getPosition();
			newPosition = oldPosition;
			newPosition.x = 2-R;
			sphere2.setPosition(newPosition);
			
		}

		if (sphere2.getPosition().y <= -2+R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere2.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.y = -oldVelocity.y;
			sphere2.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere2.getPosition();
			newPosition = oldPosition;
			newPosition.y = -2+R;
			sphere2.setPosition(newPosition);
			
		}

		if (sphere2.getPosition().y >= 2-R){
			glm::vec3 oldVelocity;
			glm::vec3 newVelocity;
			oldVelocity = sphere2.getVelocity();
			newVelocity = oldVelocity;
			newVelocity.y = -oldVelocity.y;
			sphere2.setVelocity(newVelocity);

			glm::vec3 oldPosition;
			glm::vec3 newPosition;
			oldPosition = sphere2.getPosition();
			newPosition = oldPosition;
			newPosition.y = 2-R;
			sphere2.setPosition(newPosition);
			
		}
		
		
		if (glm::distance(sphere1.getPosition(),sphere2.getPosition()) <= 2*R){
			glm::vec3 oldPosition1;
			glm::vec3 oldPosition2;
			glm::vec3 oldVelocity1;
			glm::vec3 oldVelocity2;
			glm::vec3 newPosition1;
			glm::vec3 newPosition2;
			glm::vec3 newVelocity1;
			glm::vec3 newVelocity2;

			oldPosition1 = sphere1.getPosition();
			oldPosition2 = sphere2.getPosition();
			oldVelocity1 = sphere1.getVelocity();
			oldVelocity2 = sphere2.getVelocity();

			/*newVelocity1 = oldVelocity1 + 
			     glm::length(oldPosition2 - oldPosition1)*
				 glm::dot(oldVelocity2,(oldPosition2 - oldPosition1))/
				 glm::dot((oldPosition2 - oldPosition1),(oldPosition2 - oldPosition1)) -
				 glm::length(oldPosition1 - oldPosition2)*
				 glm::dot(oldVelocity1,(oldPosition1 - oldPosition2))/
				 glm::dot((oldPosition1 - oldPosition2),(oldPosition1 - oldPosition2));

			newVelocity2 = oldVelocity2 + 
			     glm::length(oldPosition2 - oldPosition1)*
				 glm::dot(oldVelocity1,(oldPosition2 - oldPosition1))/
				 glm::dot((oldPosition2 - oldPosition1),(oldPosition2 - oldPosition1)) -
				 glm::length(oldPosition1 - oldPosition2)*
				 glm::dot(oldVelocity2,(oldPosition1 - oldPosition2))/
				 glm::dot((oldPosition1 - oldPosition2),(oldPosition1 - oldPosition2));*/

			glm::vec3 vecx = oldPosition1 - oldPosition2;
			glm::normalize(vecx);
			float x1 = glm::dot(vecx,oldVelocity1);
			glm::vec3 vecv1x = vecx * x1;
			glm::vec3 vecv1y = oldVelocity1 - vecv1x;
			float m1 = sphere1.getMass();

			vecx = -vecx;
			float x2 = glm::dot(vecx,oldVelocity2);
			glm::vec3 vecv2x = vecx * x2;
			glm::vec3 vecv2y = oldVelocity2 - vecv2x;
			float m2 = sphere2.getMass();

			newVelocity1 = vecv1x*(m1-m2)/(m1+m2)+vecv2x*(2*m2)/(m1+m2) + vecv1y;
			newVelocity2 = vecv1x*(2*m1)/(m1+m2)+vecv2x*(m2-m1)/(m1+m2) + vecv2y;


			sphere1.setVelocity(newVelocity1);
			sphere2.setVelocity(newVelocity2);
		}

		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::translate(
            model1,
            sphere1.getPosition()
        );
		
        model1 = glm::rotate(
            model1,
			time * glm::radians(90.0f), 
            glm::vec3(0.0f, 0.0f, time * 0.1* 1.0f)
        );

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(
            model2,
            sphere2.getPosition()
        );

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model1)); //sets the uniform matrix model in shader
		sphere1.draw();

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model2)); //sets the uniform matrix model in shader
		sphere2.draw();
		// update other events like input handling
		glfwPollEvents();
		if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( window, 1 );
		}
		
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers( window );
		auto t_after_frame_display = std::chrono::high_resolution_clock::now();
		frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(t_after_frame_display - t_now).count();
		fps = 1/frame_time;
		frame_time_cummulated += frame_time;

		log_file.open(GL_LOG_FILE,std::ios::app);
		log_file << "t_now: " << t_now << std::endl;
		log_file << "t_after_frame_display: " << t_after_frame_display << std::endl;
		log_file << "frame_time: " << frame_time << std::endl;
		log_file << "fps: " << fps << std::endl;
		log_file << "time: " << time << std::endl;
		log_file.close();
		if (frame_time_cummulated >= 1.0f){
			std::string fps_title[] = {"OpenGL @ FPS: "};
			fps_title[0].append(std::to_string(fps));
			glfwSetWindowTitle( window,  fps_title[0].c_str() );
			frame_time_cummulated = 0.0f;
		}
		
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	sphere1.cleanup();
	plane1.cleanup();
	return 0;
}