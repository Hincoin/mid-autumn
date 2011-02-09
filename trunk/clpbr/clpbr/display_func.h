#ifndef _DISPLAY_FUNC_H_
#define _DISPLAY_FUNC_H_

#include <GL/glut.h>

extern int width;
extern int height;
extern unsigned int *pixels;
extern unsigned int renderingFlags;
extern char captionBuffer[256];

extern void init_display(int argc, char *argv[], char *windowTittle);
extern void update_camera();
extern void display();

//render
extern void setup_opencl();
extern double WallClockTime();
#endif