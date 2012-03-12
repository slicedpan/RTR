#include <cstdlib>
#include <GL/glew.h>
#include <glut.h>
#include <cstdio>
#include <cstring>
#include "Shader.h"
#include "CubeMap.h"
#include <vector>
#include <svl\SVL.h>
#include "glm\glm.h"

#ifndef M_PI
#define M_PI 3.141529
#endif

double rotAngle = 10; // a global variable

/***************************************************************
 ** init function ***
 
 set up some default OpenGL values here
 ***************************************************************/

void DrawSkyBox();

void* glutFonts[7] = {
    GLUT_BITMAP_9_BY_15,
    GLUT_BITMAP_8_BY_13,
    GLUT_BITMAP_TIMES_ROMAN_10,
    GLUT_BITMAP_TIMES_ROMAN_24,
    GLUT_BITMAP_HELVETICA_10,
    GLUT_BITMAP_HELVETICA_12,
    GLUT_BITMAP_HELVETICA_18
};

float yaw = 0.0f;
float yawVel = 0.0f;
float pitch = 0.0f;
float pitchVel;

Vec4 lPos(5, 5, 5, 0);

Vec3 camPos(0.0, 0.0, 0.0);

GLUquadric* nQ;

void glutPrint(float x, float y, void* font, char* text, float r, float g, float b, float a)
{
    if(!text || !strlen(text)) return;
    bool blending = false;
    if(glIsEnabled(GL_BLEND)) blending = true;
    glEnable(GL_BLEND);
    glColor4f(r,g,b,a);
    glRasterPos2f(x,y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
    if(!blending) glDisable(GL_BLEND);
}

Shader* toon;
Shader* gooch;
Shader* phong;
Shader* blinnPhong;
Shader* minnaert;

char errorLog[1024];

CubeMap* cubeMap;
Texture* normalMap;
Texture* diffuseMap;

int shaderIndex;

GLMmodel* model1;

std::map<std::string, Shader*> shaders;
typedef std::pair<std::string, Shader*> shaderEntry;
typedef std::map<std::string, Shader*>::iterator shaderIterator;

shaderIterator current;

Shader* bg;
Mat4 cameraTransform;
Mat4 teapotTransform;
Mat4 teapotBaseTransform;
Mat4 sphereBaseTransform;
Mat4 dodecTransform;

bool printMatrices = false;
bool printUniforms = false;

std::vector<Texture*> textures;

char nameBuf[64];

void PrintMatrix(float* values)
{
	for (int i = 0; i < 16; ++i)
	{
		if (*(values + i) < 0.001f)
			*(values + i) = 0.0f;
	}
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			printf("%2.2g\t", *(values + (i * 4) + j));
		}
		printf("\n");
	}
}

void init()
{	
	glewInit();

	bg = new Shader("Shaders/bg.vert", "Shaders/bg.frag", "background");
	if (bg->Compile())
		printf("%s", bg->GetErrorLog());

	shaders.insert(shaderEntry("toon", new Shader("Shaders/base.vert", "Shaders/toon.frag", "Toon")));
	shaders.insert(shaderEntry("gooch", new Shader("Shaders/base.vert", "Shaders/gooch.frag", "Gooch")));
	shaders.insert(shaderEntry("phong", new Shader("Shaders/base.vert", "Shaders/phong.frag", "Phong")));
	shaders.insert(shaderEntry("minnaert", new Shader("Shaders/base.vert", "Shaders/minnaert2.frag", "Minnaert")));
	shaders.insert(shaderEntry("blinn-phong", new Shader("Shaders/blinnphong.vert", "Shaders/blinnphong.frag", "Blinn-Phong")));
	shaders.insert(shaderEntry("oren-nayar", new Shader("Shaders/base.vert", "Shaders/orennayar.frag", "Oren-Nayar")));
	shaders.insert(shaderEntry("owen", new Shader("Shaders/base.vert", "Shaders/owen.frag", "Owen")));	
	shaders.insert(shaderEntry("envmap", new Shader("Shaders/envmap.vert", "Shaders/envmap.frag", "Environment Map")));
	shaders.insert(shaderEntry("fresnel", new Shader("Shaders/fresnel.vert", "Shaders/fresnel.frag", "Fresnel")));
	shaders.insert(shaderEntry("chromatic", new Shader("Shaders/fresnel.vert", "Shaders/chromatic.frag", "Chromatic Dispersion")));
	shaders.insert(shaderEntry("normal", new Shader("Shaders/base.vert", "Shaders/normalmap.frag", "Normal Map")));
	shaders.insert(shaderEntry("bg", bg));

	current = shaders.begin();

	model1 = glmReadOBJ("Models/bunny.obj");
	glmFacetNormals(model1);
	glmVertexNormals(model1, 90, false);
	glmSpheremapTexture(model1);
	glmScale(model1, 0.5f);

	cubeMap = new CubeMap("Images/PalmTrees", 256, MULTITEXTURE);
	cubeMap->Load();	
	normalMap = new Texture("Images/Metal_NM.png");
	normalMap->Load();
	diffuseMap = new Texture("Images/metal-texture-11.jpg");
	diffuseMap->Load();

	textures.push_back(cubeMap);
	textures.push_back(normalMap);
	textures.push_back(diffuseMap);

	for (shaderIterator shaderIter = shaders.begin(); shaderIter != shaders.end(); ++shaderIter)
	{
		Shader* shader = shaderIter->second;
		printf("%s: ", shader->GetName());
		if (!shader->Compile())
		{
			printf("\n%s\n", shader->GetErrorLog());
		}
		else
		{
			printf("compiled, %d active uniforms\n", shader->GetNumberOfUniforms());
		}
		UniformList activeUniforms = shader->GetActiveUniforms();
		for (uniformIterator iter = activeUniforms.begin(); iter != activeUniforms.end(); ++iter)
		{
			printf("%s: %d\n", iter->first.c_str(), iter->second);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->GetId());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, diffuseMap->GetId());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalMap->GetId());

	glUseProgram(bg->GetId());
	bg->Uniforms("cubeMap").SetValue(0);	

	shaderIndex = 0;

	glClearColor(1, 1, 1, 0); //background color
	glClearDepth(1.0);	//background depth value
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.33333, 1, 1000);  //setup a perspective projection

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	glEnable(GL_DEPTH_TEST); //enable z-buffer hidden surface removal

	glEnable(GL_LIGHTING); //enable lighting
	glEnable(GL_LIGHT0); //enable

	glEnable(GL_POLYGON_SMOOTH);
	
	teapotTransform.MakeDiag();
	dodecTransform.MakeDiag();	

	nQ = gluNewQuadric();
	gluQuadricTexture(nQ, true);

	teapotBaseTransform = HTrans4(Vec3(0, -2.5, 0));
}


/***************************************************************
 ** display callback function **
 - get's called each time application needs to redraw itself
 - most opengl work is done through this function
 ***************************************************************/
int tickCount = 0;



void display()
{	
	yawVel *= 0.9f;
	yaw += yawVel;
	pitchVel *= 0.9f;
	pitch += pitchVel;	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1280.0 / 720.0, 1, 1000);  //setup a perspective projection

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	glLightfv(GL_LIGHT0, GL_POSITION, lPos.Ref());

	glMultMatrixf(cameraTransform.Ref());

	glClear(
		GL_COLOR_BUFFER_BIT  //clear the frame buffer (set it to background color)
		| GL_DEPTH_BUFFER_BIT //clear the depth buffer for z-buffer hidden surface removal 
		);	

	

	glPushMatrix(); // save current modelview matrix (mostly saves camera transform)
		
	//////////////////******* insert your openGL drawing code here ****** ///////////
	
	bg->Use();
	bg->Uniforms("Model").SetValue(Mat4(vl_one));
	DrawSkyBox();
	
	current->second->Use();

	current->second->Uniforms("View").SetValue(cameraTransform);	
	current->second->Uniforms("cubemap").SetValue(0);
	current->second->Uniforms("camPos").SetValue(camPos);
	current->second->Uniforms("lightPos").SetValue(lPos);
	current->second->Uniforms("normalMap").SetValue(2);
	current->second->Uniforms("diffuseMap").SetValue(1);

	if (printMatrices)
	{
		tickCount = 0;
		float matrix1[16];
		glGetUniformfv(current->second->GetId(), current->second->Uniforms("View").GetId(), matrix1);
		printf("View Matrix:\n");
		PrintMatrix(matrix1);
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix1);
		printf("ModelView Matrix:\n");
		PrintMatrix(matrix1);
	}

	if (printUniforms)
	{
		UniformList u = current->second->GetActiveUniforms();
		for (uniformIterator ui = u.begin(); ui != u.end(); ++ui)
		{
			printf("%d: %s\n", ui->second.GetId(), ui->second.GetName().c_str());
		}
		unsigned int id = current->second->Uniforms("cubemap").GetId();
		int i;
		glGetUniformiv(current->second->GetId(), id, &i);
		printf("cubemap: %d", i);

		printUniforms = false;
	}

	Mat4 invView = inv(cameraTransform);
	current->second->Uniforms("invView").SetValue(invView);	

	glEnable(GL_COLOR_MATERIAL);	//instead of specifying material properties

	float matrix1[16];

	glPushMatrix();
	glTranslatef(-2, 0, 0);
	glRotated(rotAngle, 0, 1, 0);
	glColor3f(0.6, 0.6, 0.6);
	current->second->Uniforms("Model").SetValue(teapotTransform);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix1);
	if (printMatrices)
	{
		PrintMatrix(matrix1);		
	}
	glmDraw(model1, GLM_SMOOTH | GLM_TEXTURE);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(2, 0, 0);
	glRotated(rotAngle, 0, -1, 0);
	glColor3f(.6, .6, .6);
	current->second->Uniforms("Model").SetValue(dodecTransform);

	gluSphere(nQ, 1.0, 50, 50);
	glPopMatrix();

	////////////////////////////////////////////////////////////////

	glPopMatrix(); // retrieve modelview matrix

	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1280, 720, 0);
	glUseProgram(0);
	glutPrint(200, 20, glutFonts[0], current->second->GetName(), 0, 0, 0, 1);

	glFlush(); //force OpenGL to render now;

	glutSwapBuffers(); // when using double buffering, need to swap front and back buffer
	++tickCount;
	printMatrices = false;
}

int lastTick = 0;

void idle()
{
	camPos[0] = sin(yaw) * 7.5;
	camPos[1] = sin(pitch) * 7.5;
	camPos[2] = cos(yaw) * 7.5;

	cameraTransform = HTrans4(-camPos) * HRot4(Vec3(0, 1, 0), -yaw) * HRot4(Vec3(1, 0, 0), pitch);
	Mat4 localTransform = HRot4(Vec3(0, 1, 0), rotAngle / (180.0f / M_PI));
	teapotTransform =  localTransform * teapotBaseTransform;
	dodecTransform = HTrans4(Vec3(5, 0, 0)) * HRot4(Vec3(0, 1, 0), -rotAngle / 57.2);
	rotAngle+=0.2;
	shaders["envmap"]->Use();
	shaders["envmap"]->Uniforms("time").SetValue((float)glutGet(GLUT_ELAPSED_TIME));
	shaders["envmap"]->Uniforms("camPos").SetValue(camPos);
	while (glutGet(GLUT_ELAPSED_TIME) - lastTick < 16) {}
	lastTick = glutGet(GLUT_ELAPSED_TIME);
	
	glutPostRedisplay();
}


/*************************************************************
 ** keyboard callback function **

 *************************************************************/

void IncrementShader()
{
	++current;
	if (current == shaders.end())
		current = shaders.begin();
}

void DecrementShader()
{
	if (current == shaders.begin())
		current = shaders.end();
	--current;
}

void keyboard(unsigned char k, int x, int y)
{

	switch (k)
	{
	case 'a':
		rotAngle+=.01; //changes a global variable ... this is later picked up by display
		break;
	case 'd':
		rotAngle-=.01;  //changes a global variable ... this is later picked up by display
		break;
	case 'w':
		IncrementShader();
		break;
	case 's':
		DecrementShader();
		break;
	case 'r':
		if (!current->second->Reload())
			printf("%s", current->second->GetErrorLog());		
		break;
	case 27:
		//exit(0);
		break;
	case 'b':
		printMatrices = true;
		break;
	case 'u':
		printUniforms = true;
		break;
	case 't':
		for (int i = 0; i < textures.size(); ++i)
		{
			textures[i]->Load();
		}
		break;
	}

	 //tell glut that the display callback should be called after this
}


/*************************************************************
 ** main ***
 where everything begins

*************************************************************/

int startX;
int startY;

bool dragging = false;

void DrawSkyBox(void)
{
    GLfloat fExtent = 15.0f;
    
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->GetId());

    glBegin(GL_QUADS);

        glTexCoord3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-fExtent, -fExtent, fExtent);
        
        glTexCoord3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-fExtent, -fExtent, -fExtent);
        
        glTexCoord3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-fExtent, fExtent, -fExtent);
        
        glTexCoord3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-fExtent, fExtent, fExtent);


        ///////////////////////////////////////////////
        //  Postive X
        glTexCoord3f(1.0f, -1.0f, -1.0f);
        glVertex3f(fExtent, -fExtent, -fExtent);
        
        glTexCoord3f(1.0f, -1.0f, 1.0f);
        glVertex3f(fExtent, -fExtent, fExtent);
        
        glTexCoord3f(1.0f, 1.0f, 1.0f);
        glVertex3f(fExtent, fExtent, fExtent);
        
        glTexCoord3f(1.0f, 1.0f, -1.0f);
        glVertex3f(fExtent, fExtent, -fExtent);
 

        ////////////////////////////////////////////////
        // Negative Z 
        glTexCoord3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-fExtent, -fExtent, -fExtent);
        
        glTexCoord3f(1.0f, -1.0f, -1.0f);
        glVertex3f(fExtent, -fExtent, -fExtent);
        
        glTexCoord3f(1.0f, 1.0f, -1.0f);
        glVertex3f(fExtent, fExtent, -fExtent);
        
        glTexCoord3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-fExtent, fExtent, -fExtent);


        ////////////////////////////////////////////////
        // Positive Z 
        glTexCoord3f(1.0f, -1.0f, 1.0f);
        glVertex3f(fExtent, -fExtent, fExtent);
        
        glTexCoord3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-fExtent, -fExtent, fExtent);
        
        glTexCoord3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-fExtent, fExtent, fExtent);
        
        glTexCoord3f(1.0f, 1.0f, 1.0f);
        glVertex3f(fExtent, fExtent, fExtent);


        //////////////////////////////////////////////////
        // Positive Y
        glTexCoord3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-fExtent, fExtent, fExtent);
        
        glTexCoord3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-fExtent, fExtent, -fExtent);
        
        glTexCoord3f(1.0f, 1.0f, -1.0f);
        glVertex3f(fExtent, fExtent, -fExtent);
        
        glTexCoord3f(1.0f, 1.0f, 1.0f);
        glVertex3f(fExtent, fExtent, fExtent);
  
    
        ///////////////////////////////////////////////////
        // Negative Y
        glTexCoord3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-fExtent, -fExtent, -fExtent);
        
        glTexCoord3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-fExtent, -fExtent, fExtent);
        
        glTexCoord3f(1.0f, -1.0f, 1.0f);
        glVertex3f(fExtent, -fExtent, fExtent);
        
        glTexCoord3f(1.0f, -1.0f, -1.0f);
        glVertex3f(fExtent, -fExtent, -fExtent);
    glEnd();
}

void MouseClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			startX = x;
			startY = y;
			dragging = true;
		}
		if (state == GLUT_UP)
		{
			dragging = false;
		}
	}
}

void MouseMove(int x, int y)
{
	if (dragging)
	{		
		yawVel -= (x - startX) / 3000.0f;
		startX = x;
		pitchVel += (y - startY) / 3000.0f;
		startY = y;
	}
	lPos[0] = (float)x / 200.0f;
	lPos[1] = (float)(720 - y) / 200.0f;
}

void main()
{
	glutInitDisplayMode(	// initialize GLUT
			GLUT_DOUBLE		// request memory for 2 frame buffers for animation
			| GLUT_DEPTH	// request memory for z-buffer
			| GLUT_RGBA		// set RGB color mode
			| GLUT_MULTISAMPLE
			); 	

	glutInitWindowSize(1280, 720);
	glutCreateWindow("Real-time Rendering Lab 1");	// create a window
	glutDisplayFunc(display);				// set the display callback
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(MouseMove);	
	glutMotionFunc(MouseMove);
	glutMouseFunc(MouseClick);	
	init();
	
	glutMainLoop();	// now let GLUT take care of everything
}
