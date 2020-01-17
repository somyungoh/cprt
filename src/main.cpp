/************************************************************
 * 															*
 * 		VIZA659/CSCE649 Physically Based Modeling			*
 * 		Fall 2019											*
 * 															*
 * 		Homework1 - Ball Bouncing							*
 * 															*
 * 		Somyung Oh, 525009483								*
 * 		Department of Visualization							*
 * 															*
 ************************************************************/

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <omp.h>

#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "solver.h"

#define WIDTH   	1280
#define HEIGHT  	720
#define TEXT_HEIGHT 10
#define FPS			30

// Simulation solver class
Solver solver;

// OpenGL Things
Shader shader;
Icosphere* sphere;
Cube* cube;
Camera cam(glm::vec3(0, 0, 3), glm::vec3(0, 1, 0));
std::vector<glm::vec3> palette;
void *font = GLUT_BITMAP_8_BY_13;

// simulation controls
bool  isSimulate;
float timestep;
glm::vec3 gravity;
glm::vec3 wind;

// camera manipulation
int rotateon;
int lastx, lasty;
int xchange, ychange;
int lastxchange, lastychange;
float spin, spinup;

// timing
float deltaTime;
float lastFrame;


void load_shaders(char* vshaderPath, char* fshaderPath){

	shader = Shader(vshaderPath, fshaderPath);
}

//----------------------------------------------------
//
//  GL and GLUT callbacks
//
//----------------------------------------------------

void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    //glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    //glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_LIGHTING);
    glPopAttrib();
}

void cbDisplayText(float fps) {
   // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    //gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1); // set to orthogonal projection

	float color_Y[4] = {1, 1, 0, 1};
    float color_B[4] = {0.2, 0.2, 1, 1};
    float color_W[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);

    ss << "FPS: " << fps << std::ends;
    drawString(ss.str().c_str(), 1, HEIGHT-TEXT_HEIGHT, color_Y, font);
    ss.str("");

    ss << "Timestep: " << timestep << std::ends;
    drawString(ss.str().c_str(), 1, HEIGHT-(2*TEXT_HEIGHT), color_W, font);
    ss.str("");

    ss << "Gravity: " << gravity.x << "\t" << gravity.y << "\t" << gravity.z << "\t" << std::ends;
    drawString(ss.str().c_str(), 1, HEIGHT-(3*TEXT_HEIGHT), color_W, font);
    ss.str("");

    ss << "Wind: " << wind.x << "\t" << wind.y << "\t" << wind.z << "\t" << std::ends;
    drawString(ss.str().c_str(), 1, HEIGHT-(4*TEXT_HEIGHT), color_W, font);
    ss.str("");

    ss << "Elasticity: " << solver.colliders[0]->Kelasity << std::ends;
    drawString(ss.str().c_str(), 1, HEIGHT-(5*TEXT_HEIGHT), color_W, font);
    ss.str("");

	ss << "Friction: " << solver.colliders[0]->Kfriction << std::ends;
    drawString(ss.str().c_str(), 1, HEIGHT-(6*TEXT_HEIGHT), color_W, font);
    ss.str("");

    ss << "Press UP/DOWN to change timestep"  << std::ends;
    drawString(ss.str().c_str(), 1, 1, color_B, font);
    ss.str("");

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}

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
	sprintf(title, "VIZA659 Homework1 \t %.5f fps", currentFPS);
	glutSetWindowTitle(title);
	cbDisplayText(currentFPS);
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

	shader.use();
	
	// ***  transformations   *** //

	glm::mat4 model 	 = glm::mat4(1.0f);	// identity	
	glm::mat4 view 		 = glm::mat4(1.0f);	
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(cam.Zoom), (float)WIDTH/(float)HEIGHT, 0.1f, 1000.f);
	view	   = cam.GetViewMatrix();

	// GLSL Setup
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setVec3("camPos", glm::vec3(cam.Position));

	// lighting setup
	shader.setVec3("lightPos[0]", glm::vec3(0, 0.3, 0));
	shader.setVec3("lightColor[0]", glm::vec3(1, 1, 1) * 5.f);
	
	// fun stuff
	//shader.setVec3("lightPos[0]", solver.colliders[0]->p);
	//shader.setVec3("lightPos[1]", solver.colliders[1]->p);
	//shader.setVec3("lightPos[2]", solver.colliders[2]->p);
	//shader.setVec3("lightPos[3]", solver.colliders[3]->p);
	//shader.setVec3("lightPos[4]", solver.colliders[4]->p);
	//shader.setVec3("lightColor[0]", palette[0]);
	//shader.setVec3("lightColor[1]", palette[1]);
	//shader.setVec3("lightColor[2]", palette[2]);
	//shader.setVec3("lightColor[3]", palette[3]);
	//shader.setVec3("lightColor[4]", palette[4]);

	// ***   draw sphere colliders   *** //
	for(int i=0; i < solver.colliders.size(); i++){
		if(solver.colliders[i]->type == Collider::SPHERE){
			glm::mat4 model_sphere = model;
			model_sphere = glm::translate(model_sphere, solver.colliders[i]->p);
			model_sphere = glm::scale(model_sphere, glm::vec3(dynamic_cast<sphereCollider*>(solver.colliders[i])->R()));
			sphere->draw(GL_TRIANGLES, shader, model_sphere, palette[i]);
		}
	}

	// ***   cube rotation   *** //

	int dxchange = xchange - lastxchange;
	int dychange = ychange - lastychange;
	
	spin = dxchange / 250.0;
	spinup = dychange / 250.0;
	
	// transform display cube
	glm::mat4 model_cube = cube->model;
	model_cube = glm::rotate(model_cube, spinup, glm::vec3(1, 0, 0));
	model_cube = glm::rotate(model_cube, spin, glm::vec3(0, 1, 0));
	cube->model = model_cube;

	// transform collider
	for(Collider* c : solver.colliders){
		if(c->type == Collider::PLANE)
			dynamic_cast<planeCollider*>(c)->transform(cube->model);
	}

	lastxchange = xchange;
	lastychange = ychange;	


	// ***   draw cube   *** //

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);  

	cube->draw(GL_TRIANGLES, shader, cube->model, glm::vec3(1, 1, 1));

	glDisable(GL_CULL_FACE);
	glutSwapBuffers();
}

// animate and display new result
void cbIdle()
{
	// RUN SIMULATION HERE
	if(isSimulate)
		solver.sim_one_timestep(timestep);
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
	case 32: // space bar
		isSimulate = !isSimulate;
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
		timestep += 0.0000001;
		std::cout << "timestep:" << timestep << std::endl;
		break;
	case GLUT_KEY_DOWN:
		timestep -= 0.0000001;
		std::cout << "timestep:" << timestep << std::endl;
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
			lastx = -xchange + x;
			lasty = -ychange + y;
			//xchange = 0;
			//ychange = 0;
			rotateon = 1;
		}
		else if (state == GLUT_UP) {
			//xchange = 0;
			//ychange = 0;
			rotateon = 0;
		}
		break;

	default:
		break;
	}
}

void cbMouseMove(int x, int y)
{
	xchange = x - lastx;
	ychange = y - lasty;
}



void PrintUsage()
{
	printf("==============================================================\n");
	printf("		Homework1 : Bouncing Ball							  \n");
	printf(" [Initial State]											  \n");
	printf(" stepsize: %f	  						  					  \n", timestep);
	printf(" gravity : %.3f %.3f %.3f				  					  \n", gravity.x, gravity.y, gravity.z);
	printf(" wind    : %.3f %.3f %.3f				  					  \n", wind.x, wind.y, wind.z);
	printf(" elasticity : %f						  					  \n", solver.colliders[0]->Kelasity);
	printf(" friction   : %f						  					  \n", solver.colliders[0]->Kfriction);
	printf("==============================================================\n");
}

void init(){

	// init variables
	isSimulate 	= false;
	timestep  	= 0.0000032;
	gravity		= glm::vec3(0, -0.098f, 0);
	wind		= glm::vec3(0.2, 0, 0);
	deltaTime	= 0.f;
	lastFrame	= 0.f;

	// set palette
	palette.push_back(glm::vec3(1, 0, 0.2));
	palette.push_back(glm::vec3(1, 0.67, 0));
	palette.push_back(glm::vec3(1, 0.47, 1));
	palette.push_back(glm::vec3(0.67, 0.87, 0.13));
	palette.push_back(glm::vec3(0, 0.53, 1));
	palette.push_back(glm::vec3(0.6, 0.06, 0.67));

	// setup scene
	solver.setGravity(gravity);
	solver.setAir(wind, 0.1);
	
	sphereCollider* ball1 = new sphereCollider(glm::vec3(0.1, 0, 0), 0.04);
	ball1->v = glm::vec3(0.2, 0, 0);
	ball1->Kelasity = 0.75;
	ball1->Kfriction = 0.5;
	sphereCollider* ball2 = new sphereCollider(glm::vec3(-0.1, 0, 0), 0.04);
	ball2->v = glm::vec3(-0.2, 0, 0);
	ball2->Kelasity = 0.75;
	ball2->Kfriction = 0.5;
	sphereCollider* ball3 = new sphereCollider(glm::vec3(0, 0.1, 0), 0.04);
	ball3->v = glm::vec3(0, 0.5, 0);
	ball3->Kelasity = 0.75;
	ball3->Kfriction = 0.5;
	sphereCollider* ball4 = new sphereCollider(glm::vec3(0, -0.1, 0), 0.04);
	ball4->v = glm::vec3(0, -0.2, 0);
	ball4->Kelasity = 0.75;
	ball4->Kfriction = 0.5;

	// fun stuff
	//sphereCollider* ball1 = new sphereCollider(glm::vec3(-0.35, 0, 0), 0.04);
	//ball1->v = glm::vec3(0.5, 0.5, 0.5);
	//sphereCollider* ball2 = new sphereCollider(glm::vec3(-0.17, 0, 0), 0.04);
	//ball2->v = glm::vec3(0.8, 0.3, -0.7);
	//sphereCollider* ball3 = new sphereCollider(glm::vec3(0, 0, 0), 0.04);
	//ball3->v = glm::vec3(-0.5, 0.5, 0.2);
	//sphereCollider* ball4 = new sphereCollider(glm::vec3(0.17, 0, 0), 0.04);
	//ball4->v = glm::vec3(0.2, 0.5, 0.8);
	//sphereCollider* ball5 = new sphereCollider(glm::vec3(0.35, 0, 0), 0.04);
	//ball5->v = glm::vec3(-0.7, 0.5, -0.5);

	solver.addCollider(ball1);
	solver.addCollider(ball2);
	solver.addCollider(ball3);
	solver.addCollider(ball4);
	//solver.addCollider(ball5);

	// cube
	solver.addCollider(new planeCollider(glm::vec3(0, -0.5, 0), glm::vec3(0, 1, 0)));
	solver.addCollider(new planeCollider(glm::vec3(0.5, 0, 0), glm::vec3(-1, 0, 0)));
	solver.addCollider(new planeCollider(glm::vec3(0, 0.5, 0), glm::vec3(0, -1, 0)));
	solver.addCollider(new planeCollider(glm::vec3(-0.5, 0, 0), glm::vec3(1, 0, 0)));
	solver.addCollider(new planeCollider(glm::vec3(0, 0, 0.5), glm::vec3(0, 0, -1)));
	solver.addCollider(new planeCollider(glm::vec3(0, 0, -0.5), glm::vec3(0, 0, 1)));
	
	sphere = new Icosphere(1.f, 3, false);
	cube   = new Cube(1.0f);

	PrintUsage();
}

int main(int argc, char** argv) {

	omp_set_num_threads(20);

	// GLUT routines
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("VIZA659 Homework1");
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