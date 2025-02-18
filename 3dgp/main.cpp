#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// Wolf Position and Velocity
vec3 wolfPos = vec3(0, 0, 0);	// note: Y coordinate will be amended in run time
vec3 wolfVel = vec3(0, 0, 0);

// 3D Models
C3dglTerrain terrain;
C3dglModel wolf, tree;

// GLSL Objects (Shader Program)
C3dglProgram program;
//textures 
C3dglBitmap grass;
C3dglBitmap wolftex;
GLuint idTexGrass;
GLuint idTexWolf;
// Skybox
C3dglSkyBox skybox;
// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	glEnable(GL_ALPHA_TEST);	// Keep this code!
	glAlphaFunc(GL_GREATER, 0.5);

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.compile()) return false;

	if (!FragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(VertexShader)) return false;
	if (!program.attach(FragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));
	
	
	// load your 3D models here!
	// DON'T REMOVE ANYTHING - this code loads some objects and all tree textures
	if (!terrain.load("models\\heightmap.png", 50)) return false;
	if (!wolf.load("models\\wolf.dae")) return false;
	
	
	
	if (!tree.load("models\\tree\\tree.3ds")) return false;
	tree.loadMaterials("models\\tree");
	tree.getMaterial(0)->loadTexture(GL_TEXTURE1, "models\\tree", "pine-trunk-norm.dds");
	tree.getMaterial(1)->loadTexture(GL_TEXTURE1, "models\\tree", "pine-leaf-norm.dds");
	tree.getMaterial(2)->loadTexture(GL_TEXTURE1, "models\\tree", "pine-branch-norm.dds");
	
	if (!skybox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;

	// load the grass texture
	grass.load("models\\grass.jpg", GL_RGBA);
	if (!grass.getBits()) return false;
	// load the wolf texture
	wolftex.load("models\\wolf.jpg", GL_RGBA);
	if (!wolftex.getBits()) return false;
	// prepare the grass texture 
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grass.getWidth(), grass.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, grass.getBits());
	// prepare the wolf texture
	glGenTextures(1, &idTexWolf);
	glBindTexture(GL_TEXTURE_2D, idTexWolf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wolftex.getWidth(), wolftex.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, wolftex.getBits());
	// send the texture to the shader
	program.sendUniform("texture0", 0);
	// Initialise the View Matrix (initial position of the camera)
	matrixView = lookAt(
		vec3(-2.0, 1.0, 3.0),
		vec3(-2.0, 1.0, 0.0),
		vec3(0.0, 1.0, 0.0));
	matrixView = rotate(matrixView, radians(12.f), vec3(1, 0, 0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;
	m = matrixView;
	// Disable depth test for skybox rendering
	program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0));
	program.sendUniform("lightDirection", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("lightColor", vec3(0.0f, 0.0f, 0.0f));
	glDisable(GL_DEPTH_TEST);
	skybox.render(m);
	// Re-enable depth test for other objects
	glEnable(GL_DEPTH_TEST);
	program.sendUniform("lightDirection", vec3(-1.0f, 1.0f, 1.0f));
	program.sendUniform("lightColor", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialAmbient", vec3(0.1f, 0.1f, 0.1));
	glActiveTexture(GL_TEXTURE0);
	// bind the grass texture
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	// render the terrain
	
	terrain.render(m);


	// Calculation of the TARGET POSITION FOR THE WOLF
	mat4 inv = inverse(translate(matrixView, vec3(-1, 0, 1)));
	vec3 targetPos = vec3(inv[3].x, 0, inv[3].z);

	// Calculation of UPDATED POSITION OF THE WOLF:
	// Just provide the velocity value in wolfVel and the following line will find the new position:
	wolfPos = wolfPos + wolfVel;

	// This vector automatically amends the Y-coordinate of the wolf according to the terrain elevation
	vec3 amendY = vec3(vec3(0, terrain.getInterpolatedHeight(wolfPos.x, wolfPos.z), 0));

	// render the wolf
	m = matrixView;
	m = translate(m, wolfPos + amendY);
	//bind the wolf texture
	glBindTexture(GL_TEXTURE_2D, idTexWolf);
	wolf.render(m);
	

	// render the trees
	m = translate(matrixView, vec3(0, terrain.getInterpolatedHeight(0, -2), -2));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	tree.render(m);
	m = translate(matrixView, vec3(-5, terrain.getInterpolatedHeight(-5, -1), -1));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	m = rotate(m, radians(150.f), vec3(0.f, 1.f, 0.f));
	tree.render(m);
	m = translate(matrixView, vec3(-4, terrain.getInterpolatedHeight(-4, -4), -4));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	m = rotate(m, radians(70.f), vec3(0.f, 1.f, 0.f));
	tree.render(m);
}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in secs
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	program.sendUniform("matrixProjection", perspective(radians(_fov), ratio, 0.02f, 1000.f));
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)),
		-pitch, vec3(1.f, 0.f, 0.f))
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char** argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char*)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char*)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything � not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

