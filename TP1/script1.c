
/* Includes standards */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*Includes OpenGL*/
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

/* Taille de la fenetre */
int winX = 800;
int winY = 600;

/*Structure pour les Coordonnées*/
typedef struct Polygone
{
  GLdouble coord[3];
  struct Polygone* next;
}*Poly;

Poly poly_test;

int nb_point;


/* Fonction d'initialisation OpenGL */
void initGL()
{
	glClearColor(0.8, 0.8, 0.7, 1.0);
}

/* Callback OpenGL de redimensionnement */
void reshapeGL(int _w, int _h)
{
	winX = _w;
	winY = _h;

	glViewport(0, 0, winX, winY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0, winX, 0.0, winY);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutPostRedisplay();
}

Poly addAPoint (int x, int y)
{
  nb_point++;
  Poly P1 = malloc (sizeof(struct Polygone));	//Attribution de l'espace memoire
  P1->coord[0] = x;				//Attribution de la valeur sur la liste nouvellement cree
  P1->coord[1] = y;				//Attribution de la valeur sur la liste nouvellement cree
  P1->coord[2] = 0;				//Attribution de la valeur sur la liste nouvellement cree
  P1->next = poly_test;			//La suite de la nouvelle liste sera l'ancienne liste
  return P1;				//Renvoi de la nouvelle liste
}

/* Callback OpenGL d'affichage */
void displayGL()
{
  int i,j;
  Poly P1 = poly_test;
  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.1,.5,0.5);
  glPointSize(8.0);

  glBegin(GL_POINTS);
  while (P1 != NULL){
    glVertex3f(P1->coord[0],P1->coord[1],P1->coord[2]);
    P1 = P1->next;
  }
  glEnd();

  if (nb_point > 2)
  {
    P1 = poly_test;
    glColor3f(0.5,.2,0.8);

    glBegin(GL_LINE_LOOP);
    while (P1 != NULL){
      glVertex3f(P1->coord[0],P1->coord[1],P1->coord[2]);
      P1 = P1->next;
    }
    glEnd();
  }
  free(P1);
  printf("nb_point = %d\n",nb_point);



	glutSwapBuffers();
}


/* Callback OpenGL de gestion de souris */
void mouseGL(int button, int state, int x, int y)
{
  if (state ==  GLUT_UP && button == GLUT_LEFT_BUTTON)
  {
    poly_test = addAPoint(x,winY-y);
    glutPostRedisplay();
  }
}

/*Fonction d'initialisation de nos variables*/
void init()
{
  nb_point = 0;
}

/* Callback OpenGL de gestion de clavier */
void keyboardGL(unsigned char _k, int _x, int _y)
{
	if(_k == 27 || _k == 'q' || _k == 'Q'){
		free(poly_test);
		exit(0);}
}

int main (int _argc, char ** _argv){

  int posX, posY;
  glutInit(&_argc, _argv);
  init();

  posX = (glutGet(GLUT_SCREEN_WIDTH ) - winX) / 2;
	posY = (glutGet(GLUT_SCREEN_HEIGHT) - winY) / 2;

  glutInitWindowSize(winX, winY);
	glutInitWindowPosition(posX, posY);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutCreateWindow("TP1 - Un point appartient-il a un Polygone ?");

  glutDisplayFunc(displayGL);
	glutReshapeFunc(reshapeGL);

	glutMouseFunc(mouseGL);
  glutKeyboardFunc(keyboardGL);

  initGL();
  glutMainLoop();

  return 0;
}
