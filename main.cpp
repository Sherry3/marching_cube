#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "file_utils.h"
#include "math_utils.h"

typedef struct offmodel {
	Vector3f *offVertices;			//Vertex coordiantes
	int numberOfVertices;			
 	int numberOfPolygons;
	int *indices;					//Indices values for solid(i.e. triangles)
	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
}OffModel;

/********************************************************************/
/*   Variables */

char * theProgramTitle = "Visualization of marching squares";
int theWindowWidth = 700, theWindowHeight = 700;
int theWindowPositionX = 40, theWindowPositionY = 40;
bool isFullScreen = false;
bool isAnimating = false;
bool boolGrid = true;
bool boolSurface = true;
bool boolOutput = true;
bool boolE = false;
bool boolLine = false;
float rotation = 0.0f;
int speed = 1;

float value=0.1f;
int n=0;

GLuint gWorldLocation;
GLuint uflag1;
float gflag1=1.0;
GLuint utrans1;
Matrix4f mtrans1;
GLuint utheta;
Matrix4f mtheta;
GLuint uscale1;
Matrix4f mscale1;
GLuint uworldTheta;
Matrix4f mworldTheta;
GLuint usurfaceScale;
Matrix4f msurfaceScale;
GLuint utrackBall;
Matrix4f mtrackBall;
Matrix4f mtrackBallTemp;

GLuint uworldTrans;
Matrix4f mworldTrans;

OffModel *cylinder;
OffModel *sphere;
OffModel *flor;
OffModel *output;

int kvalue[8000];
int boxSpeed=0;
int boxPos;
int toDraw=0;
float wholeTX=0.0f;
float wholeTY=0.0f;
float wholeTZ=0.0f;

bool boolTrackBall = true;

Vector3f TBV1, TBV2;

GLuint V, C;

/* Methos signatures*/
void fillOutputVBO();
Vector3f getIntersection(int flag, float a, float b, float c1, float c2);
void initMatrices();
void drawE();
void drawOutput();
void drawGrid();
void drawBox();
void drawVertices();
void drawSuface();
void drawFlor();
void wholeTrans(float x, float y, float z);
void createCylinder();
void createSphere(int shape, float radius);
OffModel* readOffFile(char * OffFile);
int FreeOffModel(OffModel *model);

/* Constants */
const int ANIMATION_DELAY = 0; /* milliseconds between rendering */
const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

/********************************************************************
  Utility functions
 */

/* post: compute frames per second and display in window's title bar */
void computeFPS() {
	static int frameCount = 0;
	static int lastFrameTime = 0;
	static char * title = NULL;
	int currentTime;

	if (!title)
		title = (char*) malloc((strlen(theProgramTitle) + 20) * sizeof (char));
	frameCount++;
	currentTime = glutGet((GLenum) (GLUT_ELAPSED_TIME));
	if (currentTime - lastFrameTime > 1000) {
		sprintf(title, "%s [ FPS: %4.2f ]",
			theProgramTitle,
			frameCount * 1000.0 / (currentTime - lastFrameTime));
		glutSetWindowTitle(title);
		lastFrameTime = currentTime;
		frameCount = 0;
	}
}

static void CreateVertexBuffer() {
	
	//Cylinder
	glGenVertexArrays(1, &(cylinder->VAO));
	glBindVertexArray(cylinder->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(cylinder->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (cylinder->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*cylinder->numberOfVertices, cylinder->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &(cylinder->IBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (cylinder->IBO));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (int)*cylinder->numberOfPolygons * 3, cylinder->indices, GL_STATIC_DRAW);


	//Sphere
	glGenVertexArrays(1, &(sphere->VAO));
	glBindVertexArray(sphere->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(sphere->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (sphere->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*sphere->numberOfVertices, sphere->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &(sphere->IBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (sphere->IBO));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (int)*sphere->numberOfPolygons * 3, sphere->indices, GL_STATIC_DRAW);

	//Floor	
	flor = (OffModel*)malloc(sizeof(OffModel));

	flor->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*4);

	flor->offVertices[0]=Vector3f(-1.0f, -1.0f, -1.0f);
	flor->offVertices[1]=Vector3f(-1.0f, -0.8f, 1.0f);
	flor->offVertices[2]=Vector3f(1.0f, -1.0f, -1.0f);
	flor->offVertices[3]=Vector3f(1.0f, -0.8f, 1.0f);

	glGenVertexArrays(1, &(flor->VAO));
	glBindVertexArray(flor->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(flor->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (flor->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*4, flor->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Output
	fillOutputVBO();
}

void fillOutputVBO()
{
	toDraw=0;
	boxPos=400;
	
	output = (OffModel*)malloc(sizeof(OffModel));
	Vector3f Vertex[8000];
	int points=0;
	Vector3f vertlist[12];
	int nuit;

	for(float k=0;k<20;k++)
	{
		for(float j=0;j<20;j++)
		{
			for(float i=0;i<20;i++)
			{	
				int rv=0;
				for(int m=0;m<8;m++)				
				{	
					float distance = sqrt( pow((m%2)*(2.0/20.0)-1.0+i/10.0, 2) + 
										   pow(-((m%4)/2)*(2.0/20.0)+1.0-j/10.0, 2) + 
		                                   pow((m/4)*(2.0/20.0)-1.0+k/10.0, 2) );
		
					if( distance < value ) 			//in condition
					{
						if(m==0)
							rv+=pow(2, 7);
						else if(m==1)
							rv+=pow(2, 6);
						else if(m==2)
							rv+=pow(2, 3);
						else if(m==3)
							rv+=pow(2, 2);
						else if(m==4)
							rv+=pow(2, 4);
						else if(m==5)
							rv+=pow(2, 5);
						else if(m==6)
							rv+=pow(2, 0);
						else if(m==7)
							rv+=pow(2, 1);
					}
				}

				if(rv==0 || rv==255)
					continue;

			//	if(points==0)				
			//		printf("rv = %d\n", rv);

				if (lookUp1[rv] & 1)
				  vertlist[0] = getIntersection(3, 0.9-j/10, -0.9+k/10, -1.0+i/10, -0.9+i/10);
				if (lookUp1[rv] & 2)
				  vertlist[1] = getIntersection(1, -0.9+i/10, 0.9-j/10, -1.0+k/10, -0.9+k/10);
				if (lookUp1[rv] & 4)
				  vertlist[2] = getIntersection(3, 0.9-j/10, -1.0+k/10, -1.0+i/10, -0.9+i/10);
				if (lookUp1[rv] & 8)
				  vertlist[3] = getIntersection(1, -1.0+i/10, 0.9-j/10, -1.0+k/10, -0.9+k/10);
				if (lookUp1[rv] & 16)
				  vertlist[4] =getIntersection(3, 1.0-j/10, -0.9+k/10, -1.0+i/10, -0.9+i/10);
				if (lookUp1[rv] & 32)
				  vertlist[5] =getIntersection(1, -0.9+i/10, 1.0-j/10, -1.0+k/10, -0.9+k/10);
				if (lookUp1[rv] & 64)
				  vertlist[6] =getIntersection(3, 1.0-j/10, -1.0+k/10, -1.0+i/10, -0.9+i/10);
				if (lookUp1[rv] & 128)
				  vertlist[7] =getIntersection(1, -1.0+i/10, 1.0-j/10, -1.0+k/10, -0.9+k/10);
				if (lookUp1[rv] & 256)
				  vertlist[8] =getIntersection(2, -1.0+i/10, -0.9+k/10, 0.9-j/10, 1.0-j/10);
				if (lookUp1[rv] & 512)
				  vertlist[9] =getIntersection(2, -0.9+i/10, -0.9+k/10, 0.9-j/10, 1.0-j/10);
				if (lookUp1[rv] & 1024)
				  vertlist[10] =getIntersection(2, -0.9+i/10, -1.0+k/10, 0.9-j/10, 1.0-j/10);
				if (lookUp1[rv] & 2048)
				  vertlist[11] =getIntersection(2, -1.0+i/10, -1.0+k/10, 0.9-j/10, 1.0-j/10);

				for(int l=0;lookUp2[rv][l]!=-1;l++) 
				{
					if(points%3==0)
						kvalue[points/3]=(k*400+j*20+i);

					Vertex[points++] = vertlist[lookUp2[rv][l]];
					
					if(rv==39)
					{
						//Vertex[points-1].Print();
						//printf("\n i:%f, j:%f, k:%f\n", i, j, k);
					}
				}
			}
		}
	}

	output->numberOfVertices=points;
	output->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*output->numberOfVertices);

	//printf("%d %d\n", points, output->numberOfVertices);

	for(int i=0;i<points;i++)
		output->offVertices[i]=Vertex[i];

	glGenVertexArrays(1, &(output->VAO));
	glBindVertexArray(output->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(output->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (output->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*output->numberOfVertices, output->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

Vector3f getIntersection(int flag, float a, float b, float c1, float c2)
{
	if(flag==1)
	{
		return Vector3f(a, b, (sqrt(value*value-a*a-b*b)>=c1 && sqrt(value*value-a*a-b*b)<=c2)?sqrt(value*value-a*a-b*b):-sqrt(value*value-a*a-b*b));	
	}
	if(flag==2)
	{
		return Vector3f(a, (sqrt(value*value-a*a-b*b)>=c1 && sqrt(value*value-a*a-b*b)<=c2)?sqrt(value*value-a*a-b*b):-sqrt(value*value-a*a-b*b), b);	
	}
	if(flag==3)
	{
		return Vector3f((sqrt(value*value-a*a-b*b)>=c1 && sqrt(value*value-a*a-b*b)<=c2)?sqrt(value*value-a*a-b*b):-sqrt(value*value-a*a-b*b), a, b);	
	}
	
	return NULL;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar * p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

using namespace std;

static void CompileShaders() {
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	}

	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	}

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof (ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof (ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	V = glGetAttribLocation(ShaderProgram, "Position");   //Link V to Position of shader program
	//C = glGetAttribLocation(ShaderProgram, "Color");

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	uworldTheta = glGetUniformLocation(ShaderProgram, "worldTheta");
	uflag1 = glGetUniformLocation(ShaderProgram, "flag1");
	utrans1 = glGetUniformLocation(ShaderProgram, "trans1");
	utheta = glGetUniformLocation(ShaderProgram, "theta");
	uscale1 = glGetUniformLocation(ShaderProgram, "scale1");
	usurfaceScale = glGetUniformLocation(ShaderProgram, "surfaceScale");
	uworldTrans = glGetUniformLocation(ShaderProgram, "worldTrans");
	utrackBall = glGetUniformLocation(ShaderProgram, "trackball");
}

/********************************************************************
 Callback Functions
 These functions are registered with the glut window and called 
 when certain events occur.
 */

void onInit(int argc, char * argv[])
/* pre:  glut window has been initialized
   post: model has been initialized */ {
	/* by default the back ground color is black */
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CompileShaders();
	CreateVertexBuffer();

	mworldTheta.InitIdentity();	
	mworldTrans.InitIdentity();
	mtrackBall.InitIdentity();
	mtrackBallTemp.InitIdentity();

	TBV1=Vector3f(-2.0f, 0.0f, 0.0f);
	TBV2=Vector3f(0.0f, 0.0f, 0.0f);

/*
	Matrix4f World;
	
	World.m[0][0] = 1.0f;	 World.m[0][1] = 0.0f; 		   	   	  World.m[0][2] = 0.0f; 		   			World.m[0][3] = 0.0f;
	World.m[1][0] = 0.0f; 	 World.m[1][1] = cosf(ToRadian(25));  World.m[1][2] = -sinf(ToRadian(25)); 		World.m[1][3] = 0.0f;
	World.m[2][0] = 0.0f; 	 World.m[2][1] = sinf(ToRadian(25));  World.m[2][2] = cosf(ToRadian(25)); 		World.m[2][3] = 0.0f;
	World.m[3][0] = 0.0f; 	 World.m[3][1] = 0.0f;            	  World.m[3][2] = 0.0f; 		   			World.m[3][3] = 1.0f;

	mworldTheta = World;
*/
/*
	World.m[0][0] = cosf(ToRadian(15));  World.m[0][1] = 0.0f; 		World.m[0][2] = -sinf(ToRadian(15)); 	World.m[0][3] = 0.0f;
	World.m[1][0] = 0.0f; 				 World.m[1][1] = 1.0f; 		World.m[1][2] = 0.0f; 					World.m[1][3] = 0.0f;
	World.m[2][0] = sinf(ToRadian(15));  World.m[2][1] = 0.0f; 		World.m[2][2] = cosf(ToRadian(15)); 	World.m[2][3] = 0.0f;
	World.m[3][0] = 0.0f; 				 World.m[3][1] = 0.0f; 		World.m[3][2] = 0.0f; 		   			World.m[3][3] = 1.0f;
	
	mworldTheta = World * mworldTheta;
*/

	/* set to draw in window based on depth  */
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_BLEND); 
}

static void onDisplay() {

	if(boxPos>=400*20 || boxPos<0)
	{
		toDraw=0;
		boxPos=0;
	}

	//printf("%d\n", boxPos);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Matrix4f World;

	World.m[0][0] = cosf(rotation); World.m[0][1] = 0.0f; 		   World.m[0][2] = -sinf(rotation); 		   World.m[0][3] = 0.0f;
	World.m[1][0] = 0.0f; World.m[1][1] = 1.0;  World.m[1][2] = 0.0; World.m[1][3] = 0.0f;
	World.m[2][0] = sinf(rotation); World.m[2][1] = 0.0;  World.m[2][2] = cosf(rotation); World.m[2][3] = 0.0f;
	World.m[3][0] = 0.0f; World.m[3][1] = 0.0f;            World.m[3][2] = 0.0f; 		   World.m[3][3] = 1.0f;

	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]);
	glUniformMatrix4fv(uworldTheta, 1, GL_TRUE, &mworldTheta.m[0][0]);
	glUniformMatrix4fv(usurfaceScale, 1, GL_TRUE, &msurfaceScale.m[0][0]);
	glUniformMatrix4fv(uworldTrans, 1, GL_TRUE, &mworldTrans.m[0][0]);
	glUniformMatrix4fv(utrackBall, 1, GL_TRUE, &mtrackBall.m[0][0]);

	initMatrices();

	//Draw here

	if(boolE)
	{
		drawE();
	}
	else
	{
		//Output
		if(boolOutput)
			drawOutput();

		//Surface
		if(boolSurface)
			drawSuface();

		//Vertices
		drawVertices();

		//Box	
		drawBox();

		//Grid
		if(boolGrid)
			drawGrid();

		//Floor
		//drawFlor();
	}

	/* check for any errors when rendering */
	GLenum errorCode = glGetError();
	if (errorCode == GL_NO_ERROR) {
		/* double-buffering - swap the back and front buffers */
		glutSwapBuffers();
	} else {
		fprintf(stderr, "OpenGL rendering error %d\n", errorCode);
	}
}

void initMatrices()
{
	mscale1.InitIdentity();
	mtrans1.InitIdentity();
	mtheta.InitIdentity();
	gflag1=1;

	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);
	glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
	glUniform1f(uflag1, gflag1);
}

void drawE()
{

	int flag=0;
	glBindVertexArray(sphere->VAO);

	float tx=float((boxPos%(400))%20)/10;
	float ty=float((boxPos%(400))/20)/10;
	float tz=float(boxPos/400)/10;

	mtheta.InitIdentity();
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

	mscale1.InitScaleTransform(0.03f, 0.03f, 0.03f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	mtrans1.InitIdentity();		

	for(int i=0;i<8;i++)
	{
		float distance = sqrt( pow((i%2)*(2.0/20.0)-1.0+tx, 2) + pow(-((i%4)/2)*(2.0/20.0)+1.0-ty, 2) + pow((i/4)*(2.0/20.0)-1.0+tz, 2) );
		//printf("distance - %d - %f %f\n", i, distance, value);

		if( distance < value ) 			//in-out condition
		{
			flag=1;
			gflag1=3.5f;
			//printf("in - %d - %d\n", i, boxPos);
		}
		else
		{
			gflag1=3.0f;
		}

		glUniform1f(uflag1, gflag1);
		mtrans1.InitTranslationTransform((i%2)*(2.0/4.0)-0.25, -((i%4)/2)*(2.0/4.0)+0.25, (i/4)*(2.0/4.0)-0.25);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, sphere->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}


	glBindVertexArray(cylinder->VAO);

	gflag1=2.0f;
	glUniform1f(uflag1, gflag1);

	mscale1.InitScaleTransform(1.0f, 1.0f, 0.25f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	for(int i=0;i<2;i++)
	{
		mtrans1.InitTranslationTransform(-0.25, 0.25-i*0.5, -0.0);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}


	for(int i=0;i<2;i++)
	{
		mtrans1.InitTranslationTransform(0.25, 0.25-i*0.5, -0.0);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}


	for(int j=0;j<2;j++)
	{
		mtheta.InitRotateTransform(90.0f, 0.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

		for(int i=0;i<2;i++)
		{
			mtrans1.InitTranslationTransform(0.25-0.5*i, 0, 0.25-j*0.5);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}

		mtheta.InitRotateTransform(0.0f, 90.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
		for(int i=0;i<2;i++)
		{
			mtrans1.InitTranslationTransform(-0.0, -0.25+0.5*i, 0.25-j*0.5);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}
	}

	initMatrices();
	glBindVertexArray(output->VAO);
	
	gflag1=6.0f;
	glUniform1f(uflag1, gflag1);

	mscale1.InitScaleTransform(5.0f, 5.0f, 5.0f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	if(toDraw<=0 || flag==0)
		return;

	Vector3f min=output->offVertices[toDraw];

	mtrans1.InitTranslationTransform(-0.05-(tx-1), 0.05-(1-ty), -0.05-(tz-1));
	glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);	

	if(boolLine)	
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, toDraw*3, n*3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void drawOutput()
{
	initMatrices();
	glBindVertexArray(output->VAO);
	
	gflag1=6.0f;
	glUniform1f(uflag1, gflag1);

	while(toDraw>0 && kvalue[toDraw-1]>=boxPos)
		toDraw--;

	while(kvalue[toDraw]==boxPos)
		toDraw++;
			
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, toDraw*3);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void drawGrid()
{
	glBindVertexArray(cylinder->VAO);
	
	mscale1.InitIdentity();
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);
	
	gflag1=1.0f;
	glUniform1f(uflag1, gflag1);

	float tz=float(boxPos/400)/10;	

	for(int j=0;j<2;j++)
	{		

		mtheta.InitRotateTransform(90.0f, 0.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

		for(int i=0;i<21;i++)
		{
			mtrans1.InitTranslationTransform(i*(2.0/20.0)-1.0, 0.0f, tz+j*0.1f-1.0);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}

		mtheta.InitRotateTransform(0.0f, 90.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
		for(int i=0;i<21;i++)
		{
			mtrans1.InitTranslationTransform(0.0f, i*(2.0/20.0)-1.0, tz+j*0.1f-1.0);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void drawBox()
{
	glBindVertexArray(cylinder->VAO);

	gflag1=2.0f;
	glUniform1f(uflag1, gflag1);

	float tx=float((boxPos%(400))%20)/10;
	float ty=float((boxPos%(400))/20)/10;
	float tz=float(boxPos/400)/10;

	mscale1.InitScaleTransform(1.1f, 1.1f, 0.05f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	for(int i=0;i<2;i++)
	{
		mtrans1.InitTranslationTransform(tx-0.90, -i*(2.0/20.0)+1.0-ty, tz+0.05-1.0);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}

	for(int i=0;i<2;i++)
	{
		mtrans1.InitTranslationTransform(tx-1.0, -i*(2.0/20.0)+1.0-ty, tz+0.05-1.0);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}

	for(int j=0;j<2;j++)
	{
		mtheta.InitRotateTransform(90.0f, 0.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

		for(int i=0;i<2;i++)
		{
			mtrans1.InitTranslationTransform(i*(2.0/20.0)-1.0+tx, 0.95-ty, j*(2.0/20.0)+tz-1.0);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}

		mtheta.InitRotateTransform(0.0f, 90.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
		for(int i=0;i<2;i++)
		{
			mtrans1.InitTranslationTransform(tx-0.95, -i*(2.0/20.0)+1.0-ty, j*(2.0/20.0)+tz-1.0);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void drawVertices()
{
	glBindVertexArray(sphere->VAO);

	float tx=float((boxPos%(400))%20)/10;
	float ty=float((boxPos%(400))/20)/10;
	float tz=float(boxPos/400)/10;

	mtheta.InitIdentity();
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

	mscale1.InitScaleTransform(0.015f, 0.015f, 0.015f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	mtrans1.InitIdentity();		

	for(int i=0;i<8;i++)
	{
		float distance = sqrt( pow((i%2)*(2.0/20.0)-1.0+tx, 2) + pow(-((i%4)/2)*(2.0/20.0)+1.0-ty, 2) + pow((i/4)*(2.0/20.0)-1.0+tz, 2) );
		//printf("distance - %d - %f %f\n", i, distance, value);

		if( distance < value ) 			//in-out condition
		{
			gflag1=3.5f;
			//printf("in - %d - %d\n", i, boxPos);
		}
		else
		{
			gflag1=3.0f;
		}

		glUniform1f(uflag1, gflag1);
		mtrans1.InitTranslationTransform((i%2)*(2.0/20.0)-1.0+tx, -((i%4)/2)*(2.0/20.0)+1.0-ty, (i/4)*(2.0/20.0)-1.0+tz);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, sphere->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}
}

void drawSuface()
{
	glBindVertexArray(sphere->VAO);

	gflag1 = 4.0;
	glUniform1f(uflag1, gflag1);
	msurfaceScale.InitScaleTransform(9.9*value/10, 9.9*value/10, 9.9*value/10);
	glUniformMatrix4fv(usurfaceScale, 1, GL_TRUE, &msurfaceScale.m[0][0]);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, sphere->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void drawFlor()
{
	initMatrices();

	glBindVertexArray(flor->VAO);

	gflag1 = 5.0;
	glUniform1f(uflag1, gflag1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void wholeTrans(float x, float y, float z)
{
	if(x==-1000)
		wholeTX=0.0f;
	else
		wholeTX=x;

	if(y==-1000)
		wholeTY=0.0f;
	else
		wholeTY=y;

	if(z==-1000)
		wholeTZ=0.0f;
	else
		wholeTZ=z;

	mworldTrans.InitTranslationTransform(wholeTX, wholeTY, wholeTZ);
}


/* pre:  glut window has been resized
 */
static void onReshape(int width, int height) {
	glViewport(0, 0, width, height);
	if (!isFullScreen) {
		theWindowWidth = width;
		theWindowHeight = height;
	}
	// update scene based on new aspect ratio....
}

/* pre:  glut window is not doing anything else
   post: scene is updated and re-rendered if necessary */
static void onIdle() {
	static int oldTime = 0;
	if (isAnimating) {
		int currentTime = glutGet((GLenum) (GLUT_ELAPSED_TIME));
		/* Ensures fairly constant framerate */
		if (currentTime - oldTime > ANIMATION_DELAY) {
			// do animation....
			boxSpeed+=speed;

			if(boxSpeed>=100)
			{
				boxPos++;
				boxSpeed=0;
			}
			else if(boxSpeed<=-100)
			{
				boxPos--;
				boxSpeed=0;
			}

			//if(rotation>=0)
			{
				//rotation=0;
				//boxPos++;
			}

			oldTime = currentTime;
			/* compute the frame rate */
			computeFPS();
			/* notify window it has to be repainted */
			glutPostRedisplay();
		}
	}
}

/* pre:  mouse is dragged (i.e., moved while button is pressed) within glut window
   post: scene is updated and re-rendered  */
static void onMouseMotion(int x, int y) {

	if(boolTrackBall)
	{
		if(x>=theWindowWidth || x<=0 || y>=theWindowHeight || y<=0)
		{
			TBV1.x=-2.0;
			return;
		}
		
		if(TBV1.x==-2.0)
		{
			float x1=float(2*x-theWindowWidth)/theWindowWidth;
			float y1=float(theWindowHeight-y*2)/theWindowHeight;
			float z1=sqrt(1-x1*x1+y1*y1);

			TBV1=Vector3f(x1, y1, z1);	
			TBV1.Normalize();
		}
		else
		{	
			float x2=float(2*x-theWindowWidth)/theWindowWidth;
			float y2=float(theWindowHeight-y*2)/theWindowHeight;
			float z2=sqrt(1-x2*x2+y2*y2);

			TBV2=Vector3f(x2, y2, z2);
			TBV2.Normalize();

			if(TBV1.x!=TBV2.x || TBV1.y!=TBV2.y ||TBV1.z!=TBV2.z)
			{
				Vector3f axis=TBV2.Cross(TBV1);
				float angle= asin(axis.length());

				//printf("%f %f %f :: %f\n", axis.x, axis.y, axis.z, angle);

				axis.Normalize();
				mtrackBallTemp.InitAxisRotateTransform(axis, angle);

				mtrackBall = mtrackBallTemp * mtrackBall;
				mtrackBallTemp.InitIdentity();

				TBV1=TBV2;
			}
		}
	}
	
	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  mouse button has been pressed while within glut window
   post: scene is updated and re-rendered */
static void onMouseButtonPress(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		// Left button pressed
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		// Left button un pressed
		TBV1.x=-2.0;
	}
	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  key has been pressed
   post: scene is updated and re-rendered */
static void onAlphaNumericKeyPress(unsigned char key, int x, int y) {
	
	if(key=='f')
	{
		boolLine=!boolLine;
	}

	if(key=='e')
	{
		boolE=!boolE;

		if(boolE)
		{
			isAnimating = false;
			while(toDraw>0 && kvalue[toDraw]>=boxPos)
			{
				toDraw--;
				n++;
			}

			n--;
			toDraw++;

			//printf("%d\n", n);
		}
		else
		{
			toDraw+=n;
			n=0;
		}
	}

	if(!boolE)
	{
		switch (key) {
				/* toggle animation running */
			case 'a':
				isAnimating = !isAnimating;
				break;
				/* reset */
			case 'r':
				rotation = 0;
				break;
			case 'g':
				boolGrid=!boolGrid;
				break;
			case 'c':
				boolSurface=!boolSurface;
				break;
			case 'x':
				value-=0.01;
				fillOutputVBO();
				break;
			case 'v':
				value+=0.01;
				fillOutputVBO();
				break;
			case 'o':
				boolOutput=!boolOutput;
				break;
			case 't':
				boolTrackBall = !boolTrackBall;
				TBV1=Vector3f(-2.0f, 0.0f, 0.0f);
				TBV2=Vector3f(0.0f, 0.0f, 0.0f);
				break;
			case 'T':
				mtrackBall.InitIdentity();
				break;		
			case 'd':
				boxPos++;
				break;
			case 's':
				boxPos--;
				break;
			case 'D':
				{
					boxPos+=400;
					int boxp=boxPos-400;

					while(boxp<=boxPos)
					{			
						while(kvalue[toDraw]==boxp)
							toDraw++;

						boxp++;
					}

					break;
				}
			case 'S':
				{
					int boxp=boxPos+400;

					while(boxp>=boxPos)
					{			
						while(toDraw>0 && kvalue[toDraw-1]>=boxp)
							toDraw--;

						boxp--;
					}

					boxPos-=400;
					break;	
				}
			case '+':
				speed+=0.002;
				break;
			case '-':
				speed-=0.002;
				break;
				/* quit! */
			case 'Q':
			case 'q':
			case 27:
				exit(0);
		}
	}
	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  arrow or function key has been pressed
   post: scene is updated and re-rendered */
static void onSpecialKeyPress(int key, int x, int y) {
	/* please do not change function of these keys */
	switch (key) {
			/* toggle full screen mode */
		case GLUT_KEY_F1:
			isFullScreen = !isFullScreen;
			if (isFullScreen) {
				theWindowPositionX = glutGet((GLenum) (GLUT_WINDOW_X));
				theWindowPositionY = glutGet((GLenum) (GLUT_WINDOW_Y));
				glutFullScreen();
			} else {
				glutReshapeWindow(theWindowWidth, theWindowHeight);
				glutPositionWindow(theWindowPositionX, theWindowPositionY);
			}
			break;
		case GLUT_KEY_UP:
			wholeTrans(wholeTX, wholeTY, wholeTZ-0.02);				
			break;
		case GLUT_KEY_DOWN:
			wholeTrans(wholeTX, wholeTY, wholeTZ+0.02);				
			break;
	}

	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  glut window has just been iconified or restored 
   post: if window is visible, animate model, otherwise don't bother */
static void onVisible(int state) {
	if (state == GLUT_VISIBLE) {
		/* tell glut to show model again */
		glutIdleFunc(onIdle);
	} else {
		glutIdleFunc(NULL);
	}
}

static void InitializeGlutCallbacks() {
	/* tell glut how to display model */
	glutDisplayFunc(onDisplay);
	/* tell glutwhat to do when it would otherwise be idle */
	glutIdleFunc(onIdle);
	/* tell glut how to respond to changes in window size */
	glutReshapeFunc(onReshape);
	/* tell glut how to handle changes in window visibility */
	glutVisibilityFunc(onVisible);
	/* tell glut how to handle key presses */
	glutKeyboardFunc(onAlphaNumericKeyPress);
	glutSpecialFunc(onSpecialKeyPress);
	/* tell glut how to handle the mouse */
	glutMotionFunc(onMouseMotion);
	glutMouseFunc(onMouseButtonPress);
}

void createCylinder()
{
	int tempV=37*2;
	int tempP=2*tempV;

	cylinder = (OffModel*)malloc(sizeof(OffModel));
	cylinder->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*(tempV));
	cylinder->indices=(int*)malloc(sizeof(int)*tempP*3);
	
	float i=0, j=0;
	float t1=0;

	int count=0;
	
	while(t1<=360)
	{
		i=0.005*cos(ToRadian(t1));			
		j=0.005*sin(ToRadian(t1));
		cylinder->offVertices[count] = Vector3f(i, j, 1.0f);
		count++;
		cylinder->offVertices[count] = Vector3f(i, j, -1.0f);
		count++;
		t1+=10;
	}

	for(int j=0, ii=0; ii<count-2; ii++,j+=3)
	{
		cylinder->indices[j] = ii;
		cylinder->indices[j+1] = ii+1;
		cylinder->indices[j+2] = ii+2;
	}

	cylinder->numberOfPolygons=tempP;
	cylinder->numberOfVertices=tempV;
}


void createSphere(int shape, float radius)
{
	int tempc=(18/shape+1)*36;

	sphere = (OffModel*)malloc(sizeof(OffModel));
	sphere->numberOfVertices=tempc;
	sphere->numberOfPolygons=tempc*2;
	sphere->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*(sphere->numberOfVertices));
	sphere->indices=(int*)malloc(sizeof(int)*(sphere->numberOfPolygons)*3);

	float i=0, j=0, k=0;
	float t1=0, t2=0;

	int count=0;
	
	while(t2<360)
	{
		while(t1<=180)
		{
			i=radius*cos(ToRadian(t2))*sin(ToRadian(t1));			
			j=radius*sin(ToRadian(t2))*sin(ToRadian(t1));
			k=radius*cos(ToRadian(t1));
			sphere->offVertices[count] = Vector3f(i, j, k);
			count++;
			t1+=10*shape;
		}
		t1=0;
		t2+=10;
	}

	for(int j=0, ii=0; ii<count-(18/shape+1); ii++,j+=6)
	{
		sphere->indices[j] = ii;
		sphere->indices[j+1] = ii+1;
		sphere->indices[j+2] = ii+(18/shape+1);

		sphere->indices[j+3] = ii;
		sphere->indices[j+4] = ii+(18/shape+1)-1;
		sphere->indices[j+5] = ii+(18/shape+1);
	}

	for(int j=6*(count-(18/shape+1)), ii=count-(18/shape+1); ii<count; ii++,j+=6)
	{
		sphere->indices[j] = ii;
		sphere->indices[j+1] = ii+1;
		sphere->indices[j+2] = ii-(count-(18/shape+1));

		sphere->indices[j+3] = ii;
		sphere->indices[j+4] = ii+(18/shape+1)-1-count;
		sphere->indices[j+5] = ii-(count-(18/shape+1));
	}

	for(int j=0, ii=0; ii<count-(18/shape+1); ii++,j+=6)
	{
		//A triangle
		sphere->indices[j] = ii;
		sphere->indices[j+1] = ii+1;
		sphere->indices[j+2] = ii+(18/shape+1);

		//Another triangle
		sphere->indices[j+3] = ii;
		sphere->indices[j+4] = ii+(18/shape+1)-1;
		sphere->indices[j+5] = ii+(18/shape+1);
	}

}

/************************************************************/
//OFF file reading code

OffModel* readOffFile(char * OffFile) {
	FILE * input;
	char type[3]; 
	int noEdges;
	int i;
	float x,y,z;
	int n, v;
	int nv, np;
	OffModel *model;
	input = fopen(OffFile, "r");
	
	int neveruseit = fscanf(input, "%s", type);
	/* First line should be OFF */
	if(strcmp(type,"OFF")) {
		printf("Not a OFF file");
		exit(1);
	}

	/* Read the no. of vertices, faces and edges */
	neveruseit = fscanf(input, "%d", &nv);
	neveruseit = fscanf(input, "%d", &np);
	neveruseit = fscanf(input, "%d", &noEdges);

	model = (OffModel*)malloc(sizeof(OffModel));
	model->numberOfVertices = nv;
	model->numberOfPolygons = np;
	
	/* allocate required data */
	model->offVertices = (Vector3f *) malloc(nv * sizeof(Vector3f));
	//model->offColors = (Vector3f *) malloc(nv * sizeof(Vector3f));
	model->indices = (int *) malloc(3 * sizeof(int) * np);
	
	float dummy;
	/* Read the vertices' location*/	
	for(i = 0;i < nv;i ++) {
		neveruseit = fscanf(input, "%f %f %f %f", &x,&y,&z,&dummy);
		model->offVertices[i] = Vector3f(x, y, z);
	}

	/* Read the Polygons */	
	for(i=0 ; i < 3*np ; i++) {
		/* No. of sides of the polygon (Eg. 3 => a triangle) */
		if(i%3==0)
		{
			neveruseit = fscanf(input, "%d", &n);
			neveruseit = fscanf(input, "%d", &v);
			model->indices[i]=v;
		}
		else
		{
		/* read the vertices that make up the polygon */
			neveruseit = fscanf(input, "%d", &v);
			model->indices[i]=v;
		}
	}

	fclose(input);
	return model;
}

int FreeOffModel(OffModel *model)
{
	if( model == NULL )
		return 0;
	free(model->offVertices);
	free(model->indices);
	free(model);
	return 1;
}

void menu()
{
	float f=18;
	
	//printf("Value :: ");
	//scanf("%f", &f);

	msurfaceScale.InitScaleTransform(9.9/f, 9.9/f, 9.9/f);
	value=10/f;
}

int main(int argc, char** argv) {

	menu();

	createCylinder();
	createSphere(1, 1);

	glutInit(&argc, argv);

	/* request initial window size and position on the screen */
	glutInitWindowSize(theWindowWidth, theWindowHeight);
	glutInitWindowPosition(theWindowPositionX, theWindowPositionY);
	/* request full color with double buffering and depth-based rendering */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	/* create window whose title is the name of the executable */
	glutCreateWindow(theProgramTitle);

	InitializeGlutCallbacks();

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	/* initialize model */
	onInit(argc, argv);

	/* give control over to glut to handle rendering and interaction  */
	glutMainLoop();

	/* program should never get here */

	return 0;
}

