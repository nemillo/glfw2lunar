#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


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
	GLfloat vertices[332];
	int sectorCount = 10;
	int stackCount = 10;
	float radius = 1.0f;
	float fps = 0.0f;
	float frame_time_cummulated = 0.0f;

	restart_gl_log();
	auto t_start = std::chrono::high_resolution_clock::now();
	
	log_file.open(GL_LOG_FILE,std::ios::app);
	log_file << "t_start: " << t_start << std::endl;
	log_file.close();
	
	for(int i = 0; i <= stackCount; ++i)
	{
    	for(int j = 0; j <= sectorCount; ++j)
   		{
        	vertices[3*sectorCount*i+3*j] = radius * cosf(-glm::pi<float>()/2+glm::pi<float>()/stackCount * j) * cosf(2*glm::pi<float>()/sectorCount * i);             // r * cos(u) * cos(v)
        	vertices[3*sectorCount*i+3*j+1] = radius * cosf(-glm::pi<float>()/2+glm::pi<float>()/stackCount * j) * sinf(2*glm::pi<float>()/sectorCount * i);             // r * cos(u) * sin(v)
			vertices[3*sectorCount*i+3*j+2] = radius*sinf(-glm::pi<float>()/2+glm::pi<float>()/stackCount*j); //r* sin (u)
		}
	}	
	/*vertices[0] = 0.0f;
	vertices[1]= 0.5f;
	vertices[2]= 0.0f;
	vertices[3]= 0.5f;
	vertices[4]= -0.5f;
	vertices[5]= 0.0f;
	vertices[6]= -0.5f;
	vertices[7]= -0.5f;
	vertices[8]= 0.0f;*/
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
		"  frag_colour = vec4( 0.5, 0.0, 0.5, 1.0 );"
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
	
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

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
	glBindVertexArray( vao );

	GLint uniModel = glGetUniformLocation(shader_programme, "model");

    // Set up projection
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 5.0f, 5.0f),
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
        model = glm::rotate(
            model,
			time * glm::radians(90.0f), 
            glm::vec3(0.0f, 0.0f, time * 0.1* 1.0f)
        );
		
		model = glm::translate(
            model,
            glm::vec3(0.0f, 0.0f, -time * 0.1* 1.0f)
        );
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		
		// draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays( GL_POINTS, 0, 332);
		// update other events like input handling
		glfwPollEvents();
		if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( window, 1 );
		}
		
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers( window );
		auto t_after_frame_display = std::chrono::high_resolution_clock::now();
		float frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(t_after_frame_display - t_now).count();
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
	return 0;
}