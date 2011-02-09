#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "camera.h"
#include "shape.h"

#include "display_func.h"

#include "spectrum.h"
#ifndef M_PI
#define M_PI       3.14159265358979323846f
#endif

extern void ReInit(const int);
extern void ReInitScene();
extern void UpdateRendering();

extern camera_t camera;
extern sphere_t *spheres;
extern unsigned int sphereCount;

int amiSmallptCPU;

int width = 80;
int height = 60;
unsigned int *pixels;
char captionBuffer[256];

static int printHelp = 0;
static int currentSphere;

void keyFunc(unsigned char key, int x, int y);
void reshapeFunc(int newWidth, int newHeight);
void specialFunc(int key, int x, int y);
void displayFunc();
void idleFunc();
void init_display(int argc, char *argv[], char *tittle)
{
	//
	update_camera();
	setup_opencl();

	glutInitWindowSize(width, height);
	glutInitWindowPosition(0,0);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInit(&argc, argv);

	glutCreateWindow(tittle);

	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyFunc);
	glutSpecialFunc(specialFunc);
	glutDisplayFunc(displayFunc);
	glutIdleFunc(idleFunc);

	glViewport(0, 0, width, height);
	glLoadIdentity();
	glOrtho(0.f, width - 1.f, 0.f, height - 1.f, -1.f, 1.f);

}

void display()
{
	glutMainLoop();
}



double WallClockTime() {
#if defined(__linux__) || defined(__APPLE__)
	struct timeval t;
	gettimeofday(&t, NULL);

	return t.tv_sec + t.tv_usec / 1000000.0;
#elif defined (WIN32)
	return GetTickCount() / 1000.0;
#else
	Unsupported Platform !!!
#endif
}

static void PrintString(void *font, const char *string) {
	int len, i;

	len = (int)strlen(string);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, string[i]);
}

static void PrintHelp() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.f, 0.f, 0.5f, 0.5f);
	glRecti(40, 40, 600, 440);

	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(300, 420);
	PrintString(GLUT_BITMAP_HELVETICA_18, "Help");

	glRasterPos2i(60, 390);
	PrintString(GLUT_BITMAP_HELVETICA_18, "h - toggle Help");
	glRasterPos2i(60, 360);
	PrintString(GLUT_BITMAP_HELVETICA_18, "arrow Keys - rotate camera left/right/up/down");
	glRasterPos2i(60, 330);
	PrintString(GLUT_BITMAP_HELVETICA_18, "a and d - move camera left and right");
	glRasterPos2i(60, 300);
	PrintString(GLUT_BITMAP_HELVETICA_18, "w and s - move camera forward and backward");
	glRasterPos2i(60, 270);
	PrintString(GLUT_BITMAP_HELVETICA_18, "r and f - move camera up and down");
	glRasterPos2i(60, 240);
	PrintString(GLUT_BITMAP_HELVETICA_18, "PageUp and PageDown - move camera target up and down");
	glRasterPos2i(60, 210);
	PrintString(GLUT_BITMAP_HELVETICA_18, "+ and - - to select next/previous object");
	glRasterPos2i(60, 180);
	PrintString(GLUT_BITMAP_HELVETICA_18, "2, 3, 4, 5, 6, 8, 9 - to move selected object");

	glDisable(GL_BLEND);
}


void update_camera() {
	vsub(camera.dir, camera.center, camera.eye);
	vnorm(camera.dir);

	const vector3f_t up = {0.f, 1.f, 0.f};
	const float fov = (M_PI / 180.f) * 45.f;
	vxcross(camera.x, camera.dir, up);
	vnorm(camera.x);
	vsmul(camera.x, width * fov / height, camera.x);

	vxcross(camera.y, camera.x, camera.dir);
	vnorm(camera.y);
	vsmul(camera.y, fov, camera.y);
}

void idleFunc(void) {
	UpdateRendering();

	glutPostRedisplay();
}

void displayFunc(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2i(0, 0);
	glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Title
	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(4, height - 16);
	
	PrintString(GLUT_BITMAP_HELVETICA_18, "clpbr");

	// Caption line 0
	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(4, 10);
	PrintString(GLUT_BITMAP_HELVETICA_18, captionBuffer);

	if (printHelp) {
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-0.5, 639.5, -0.5, 479.5, -1.0, 1.0);

		PrintHelp();

		glPopMatrix();
	}

	glutSwapBuffers();
}

void reshapeFunc(int newWidth, int newHeight) {
	width = newWidth;
	height = newHeight;

	glViewport(0, 0, width, height);
	glLoadIdentity();
	glOrtho(0.f, width - 1.f, 0.f, height - 1.f, -1.f, 1.f);

	ReInit(1);

	glutPostRedisplay();
}

#define MOVE_STEP 10.0f
#define ROTATE_STEP (2.f * M_PI / 180.f)
void keyFunc(unsigned char key, int x, int y) {
	switch (key) {
		case 'p': {
			FILE *f = fopen("image.ppm", "w"); // Write image to PPM file.
			if (!f) {
				fprintf(stderr, "Failed to open image file: image.ppm\n");
			} else {
				fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);

				int x, y;
				for (y = height - 1; y >= 0; --y) {
					unsigned char *p = (unsigned char *)(&pixels[y * width]);
					for (x = 0; x < width; ++x, p += 4)
						fprintf(f, "%d %d %d ", p[0], p[1], p[2]);
				}

				fclose(f);
			}
			break;
				  }
		case 27: /* Escape key */
			fprintf(stderr, "Done.\n");
			exit(0);
			break;
		case ' ': /* Refresh display */
			ReInit(1);
			break;
		case 'a': {
			vector3f_t dir = camera.x;
			vnorm(dir);
			vsmul(dir, -MOVE_STEP, dir);
			vadd(camera.eye, camera.eye, dir);
			vadd(camera.center, camera.center, dir);
			ReInit(0);
			break;
				  }
		case 'd': {
			vector3f_t dir = camera.x;
			vnorm(dir);
			vsmul(dir, MOVE_STEP, dir);
			vadd(camera.eye, camera.eye, dir);
			vadd(camera.center, camera.center, dir);
			ReInit(0);
			break;
				  }
		case 'w': {
			vector3f_t dir = camera.dir;
			vsmul(dir, MOVE_STEP, dir);
			vadd(camera.eye, camera.eye, dir);
			vadd(camera.center, camera.center, dir);
			ReInit(0);
			break;
				  }
		case 's': {
			vector3f_t dir = camera.dir;
			vsmul(dir, -MOVE_STEP, dir);
			vadd(camera.eye, camera.eye, dir);
			vadd(camera.center, camera.center, dir);
			ReInit(0);
			break;
				  }
		case 'r':
			camera.eye.y += MOVE_STEP;
			camera.center.y += MOVE_STEP;
			ReInit(0);
			break;
		case 'f':
			camera.eye.y -= MOVE_STEP;
			camera.center.y -= MOVE_STEP;
			ReInit(0);
			break;
		case '+':
			currentSphere = (currentSphere + 1) % sphereCount;
			fprintf(stderr, "Selected sphere %d (%f %f %f)\n", currentSphere,
				spheres[currentSphere].p.x, spheres[currentSphere].p.y, spheres[currentSphere].p.z);
			ReInitScene();
			break;
		case '-':
			currentSphere = (currentSphere + (sphereCount - 1)) % sphereCount;
			fprintf(stderr, "Selected sphere %d (%f %f %f)\n", currentSphere,
				spheres[currentSphere].p.x, spheres[currentSphere].p.y, spheres[currentSphere].p.z);
			ReInitScene();
			break;
		case '4':
			spheres[currentSphere].p.x -= 0.5f * MOVE_STEP;
			ReInitScene();
			break;
		case '6':
			spheres[currentSphere].p.x += 0.5f * MOVE_STEP;
			ReInitScene();
			break;
		case '8':
			spheres[currentSphere].p.z -= 0.5f * MOVE_STEP;
			ReInitScene();
			break;
		case '2':
			spheres[currentSphere].p.z += 0.5f * MOVE_STEP;
			ReInitScene();
			break;
		case '9':
			spheres[currentSphere].p.y += 0.5f * MOVE_STEP;
			ReInitScene();
			break;
		case '3':
			spheres[currentSphere].p.y -= 0.5f * MOVE_STEP;
			ReInitScene();
			break;
		case 'h':
			printHelp = (!printHelp);
			break;
		default:
			break;
	}
}

void specialFunc(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP: {
			vector3f_t t = camera.center;
			vsub(t, t, camera.eye);
			t.y = t.y * cos(-ROTATE_STEP) + t.z * sin(-ROTATE_STEP);
			t.z = -t.y * sin(-ROTATE_STEP) + t.z * cos(-ROTATE_STEP);
			vadd(t, t, camera.center);
			camera.center = t;
			ReInit(0);
			break;
						  }
		case GLUT_KEY_DOWN: {
			vector3f_t t = camera.center;
			vsub(t, t, camera.eye);
			t.y = t.y * cos(ROTATE_STEP) + t.z * sin(ROTATE_STEP);
			t.z = -t.y * sin(ROTATE_STEP) + t.z * cos(ROTATE_STEP);
			vadd(t, t, camera.eye);
			camera.center = t;
			ReInit(0);
			break;
							}
		case GLUT_KEY_LEFT: {
			vector3f_t t = camera.center;
			vsub(t, t, camera.eye);
			t.x = t.x * cos(-ROTATE_STEP) - t.z * sin(-ROTATE_STEP);
			t.z = t.x * sin(-ROTATE_STEP) + t.z * cos(-ROTATE_STEP);
			vadd(t, t, camera.eye);
			camera.center = t;
			ReInit(0);
			break;
							}
		case GLUT_KEY_RIGHT: {
			vector3f_t t = camera.center;
			vsub(t, t, camera.eye);
			t.x = t.x * cos(ROTATE_STEP) - t.z * sin(ROTATE_STEP);
			t.z = t.x * sin(ROTATE_STEP) + t.z * cos(ROTATE_STEP);
			vadd(t, t, camera.eye);
			camera.center = t;
			ReInit(0);
			break;
							 }
		case GLUT_KEY_PAGE_UP:
			camera.center.y += MOVE_STEP;
			ReInit(0);
			break;
		case GLUT_KEY_PAGE_DOWN:
			camera.center.y -= MOVE_STEP;
			ReInit(0);
			break;
		default:
			break;
	}
}
