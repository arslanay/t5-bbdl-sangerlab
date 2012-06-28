/*
	Demo program for the OpenGL Graph component
	Author: Gabriyel Wong (gabriyel@gmail.com)
*/

#include <windows.h>   // Standard Header For Most Programs
#include <math.h>
#include "glut.h"   // The GL Utility Toolkit (Glut) Header
#include "OGLGraph.h"

OGLGraph* myGraph;

void init ( GLvoid )     // Create Some Everyday Functions
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);				// Black Background
	//glClearDepth(1.0f);								// Depth Buffer Setup
	myGraph = OGLGraph::Instance();
	myGraph->setup( 500, 100, 10, 10, 2, 2, 1, 200 );
}

void display ( void )   // Create The Display Function
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) 800 / (float) 600, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// This is a dummy function. Replace with custom input/data
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	float value;
	value = 5*sin( 5*time ) + 10.f;

	myGraph->update( value );
	myGraph->draw();

	glutSwapBuffers ( );
}

void reshape ( int w, int h )   // Create The Reshape Function (the viewport)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) w / (float) h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
	switch ( key ) 
	{
	case 27:        // When Escape Is Pressed...
		exit ( 0 );   // Exit The Program
		break;        // Ready For Next Case
	default:        // Now Wrap It Up
		break;
	}
}

void idle(void)
{
  glutPostRedisplay();
}

void main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
  glutInit( &argc, argv ); // Erm Just Write It =)
  init();

  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE ); // Display Mode
  glutInitWindowSize( 500, 250 ); // If glutFullScreen wasn't called this is the window size
  glutCreateWindow( "OpenGL Graph Component" ); // Window Title (argv[0] for current directory as title)
  glutDisplayFunc( display );  // Matching Earlier Functions To Their Counterparts
  glutReshapeFunc( reshape );
  glutKeyboardFunc( keyboard );
  glutIdleFunc(idle);
  glutMainLoop( );          // Initialize The Main Loop
}

