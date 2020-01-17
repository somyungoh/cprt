/************************************************************
 * 															*
 * 		GPU Accelerated (Compute Shader)					*
 * 		Raytracer											*
 * 															*
 * 															*
 * 		Implemented by Somyung Oh							*
 * 		Texas A&M University, 2020							*
 * 															*
 ************************************************************/

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
// #include <omp.h>
#include <unistd.h>

// #include "shader.h"
#include "camera.h"
// #include "mesh.h"

#define WIDTH   	1280
#define HEIGHT  	720
#define TEXT_HEIGHT 10
#define FPS			30


// OpenGL Things
Camera cam(glm::vec3(0, 0, 3), glm::vec3(0, 1, 0));
std::vector<glm::vec3> palette;

// timing
float deltaTime;
float lastFrame;


void load_shaders(char* vshaderPath, char* fshaderPath){

	// shader = Shader(vshaderPath, fshaderPath);
}

//----------------------------------------------------
//
//  GL and GLUT callbacks
//
//----------------------------------------------------


void cbTimer(int v)
{
	glutPostRedisplay();
	glutTimerFunc(1000.f/(float)FPS, cbTimer, 0);
 	
	// compute time counts
	float currentFrame = 0.001f * glutGet(GLUT_ELAPSED_TIME); // since it is in milliseconds
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;	
	
	// display current fps
	char title[32];
	float currentFPS = 1.f / deltaTime;
	sprintf(title, "CPRT \t %.5f fps", currentFPS);
	glutSetWindowTitle(title);
}

void cbReshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glutPostRedisplay();
}

void cbDisplay(void)
{

	glClearColor(0.1, 0.1, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	


	glDisable(GL_CULL_FACE);
	glutSwapBuffers();
}

// animate and display new result
void cbIdle()
{
	// idle function
}

void cbOnKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		cam.ProcessKeyboard(FORWARD, deltaTime);
		break;
	case 's':
		cam.ProcessKeyboard(BACKWARD, deltaTime);
		break;
	case 'a':
		cam.ProcessKeyboard(LEFT, deltaTime);
		break;
	case 'd':
		cam.ProcessKeyboard(RIGHT, deltaTime);
		break;
	default:
		break;
	}
}

void cbSPKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		break;
	case GLUT_KEY_DOWN:
		break;
	default:
		break;
	}
}

void cbMouseDown(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
		}
		else if (state == GLUT_UP) {
		}
		break;

	default:
		break;
	}
}

void cbMouseMove(int x, int y)
{

}



void PrintUsage()
{
	printf("==============================================================\n");
	printf(" GPU Accelerated (Compute Shader) Ray-Tracer				  \n");
	printf("															  \n");
	printf(" Developed by Somyung Oh	  								  \n");
	printf(" Texas A&M University, 2020									  \n");
	printf(" 				  						  					  \n");
	printf(" 	[Keyboard Controls]					  					  \n");
	printf(" 				  						  					  \n");
	printf("==============================================================\n");
}

void init(){

	// init variables
	deltaTime	= 0.f;
	lastFrame	= 0.f;

	// set palette
	palette.push_back(glm::vec3(1, 0, 0.2));
	palette.push_back(glm::vec3(1, 0.67, 0));
	palette.push_back(glm::vec3(1, 0.47, 1));
	palette.push_back(glm::vec3(0.67, 0.87, 0.13));
	palette.push_back(glm::vec3(0, 0.53, 1));
	palette.push_back(glm::vec3(0.6, 0.06, 0.67));

	
	PrintUsage();
}

int main(int argc, char** argv) {

	//omp_set_num_threads(20);


	// parse command line
	int opt; 
      
    // put ':' in the starting of the 
    // string so that program can  
    //distinguish between '?' and ':'  
    while((opt = getopt(argc, argv, "hp:t:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'h':
				std::cout << "Options:" << std::endl;
				std::cout << "  -h:\thelp." << std::endl;
				std::cout << "  -p:\ttotal number of particles." << std::endl;
				std::cout << "  -t:\ttimestep." << std::endl;
				return 0;
			break;
            case 'p':
				// nparticles = std::stoi(optarg);
				std::cout << "particle" << optarg << std::endl;
			break;
            case 't':
				// timestep = std::stof(optarg);
				std::cout << "timestep"  << optarg << std::endl;
			break;
        }  
    }
      
    // optind is for the extra arguments 
    // which are not parsed 
    for(; optind < argc; optind++){      
        printf("extra arguments: %s\n", argv[optind]);  
    } 



	// GLUT routines
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("CPRT");
	glutDisplayFunc(&cbDisplay);
	glutReshapeFunc(cbReshape);
	glutKeyboardFunc(&cbOnKeyboard);
	glutSpecialFunc(&cbSPKeyboard);
	glutMouseFunc(&cbMouseDown);
	glutMotionFunc(&cbMouseMove);
	glutIdleFunc(&cbIdle);
	glutTimerFunc(1000.f/(float)FPS, cbTimer, 0);
	
	// load glew
	glewExperimental = GL_TRUE; 
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("GLEW Error: %s\n", glewGetErrorString(err));
	}

	load_shaders("shaders/vertex.vs", "shaders/fragment.fs");
	init();
	
	glutMainLoop();

    return 0;
}