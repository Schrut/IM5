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
  int coord[3];
}Poly;

Poly* mon_poly;
Poly* trou_poly;

int nb_point;
int nb_hole;
int nb_point_max;
int nb_hole_max;
int insert_here;
int insert_hole_here;
int motion;
int button_pressed;
int add_hole_mode;
int edit_mode;
int coord_mode;
int display_mode;
int xpos;
int ypos;
int mouse_is_inside;
int* transition;
const char* int_ext;


void addAPoint (int x, int y);

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

/* Fonction pour afficher du texte en OpenGL */
void glPrintText(int x, int  y, const char * text)
{
	int i;

	glRasterPos2i(x, y);

	for(i = 0; i < (int)strlen(text); ++i)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
}

/*Transforme deux points donnés en un vecteurs*/
Poly madeAVector (Poly A, Poly B)
{
  Poly new_poly;
  new_poly.coord[0] = B.coord[0] - A.coord[0];
  new_poly.coord[1] = B.coord[1] - A.coord[1];
  return new_poly;
}

/*Determine si il y a deux arrêtes sécantes*/
int isASecant (void)
{
  int i,j;
  int signe1 , signe2;
  Poly ac, bc, ad, bd, ca, da, cb, db;

  mon_poly[nb_point].coord[0] = mon_poly[0].coord[0];
  mon_poly[nb_point].coord[1] = mon_poly[0].coord[1];

  if (nb_point > 3){
    for (i=0 ; i < nb_point ; i++)
      for (j = nb_point -1 ; j > 1; j--)
      {
        ca = madeAVector(mon_poly[j], mon_poly[i]);
        da = madeAVector(mon_poly[j - 1], mon_poly[i]);
        cb = madeAVector(mon_poly[j], mon_poly[i + 1]);
        db = madeAVector(mon_poly[j-1], mon_poly[i + 1]);

        if ((ca.coord[0] * cb.coord[1] - cb.coord[0] * ca.coord[1]) > 0)
          signe1 = 1;
        else
          signe1 = 0;

        if ((da.coord[0] * db.coord[1] - db.coord[0] * da.coord[1]) > 0)
          signe2 = 1;
        else
          signe2 = 0;

        if (signe1 != signe2)
        {
          ac = madeAVector(mon_poly[i], mon_poly[j]);
          ad = madeAVector(mon_poly[i], mon_poly[j - 1]);
          bc = madeAVector(mon_poly[i + 1], mon_poly[j]);
          bd = madeAVector(mon_poly[i + 1], mon_poly[j - 1]);

          if ((ac.coord[0] * ad.coord[1] - ad.coord[0] * ac.coord[1]) > 0)
            signe1 = 1;
          else
            signe1 = 0;

          if ((bc.coord[0] * bd.coord[1] - bd.coord[0] * bc.coord[1]) > 0)
            signe2 = 1;
          else
            signe2 = 0;

          if (signe1 != signe2)
            return 1;
        }
    }
  }
  return 0;
}

/*Fonction permettant de traiter les cas non évidents*/
int delit_cas (int i, int x, int y, int choix)
{
  if (choix)
  {
    double a, b, _x;

    if (mon_poly[i+1].coord[1] == mon_poly[i].coord[1]) return 1;

    a = (double)( mon_poly[i+1].coord[1] - mon_poly[i].coord[1] ) / (double)( mon_poly[i+1].coord[0] - mon_poly[i].coord[0] );
    b = (double) mon_poly[i+1].coord[1] - (mon_poly[i+1].coord[0]*a);

    _x = ( y - b ) / a;

    if (_x > x) return 1;
    else return 0;
  }

  else {
    if (transition[i + 1] == transition[i])
      return 0;
    else if ( (transition[i - 1] == 1 && transition[i + 1] == 2) || (transition[i - 1] == 2 && transition[i + 1] == 1) ) return 1;
    else return 0; 
  }
}

/*Fonction qui nous renseigne si un point est dans le Polygone*/
int isInside(int x, int y)
{
  int i;
  int NI=0;
  
  //Faire condition si pas de trou ok mais si trou verif dans le trou

  
  mon_poly[nb_point].coord[0] = mon_poly[0].coord[0];
  mon_poly[nb_point].coord[1] = mon_poly[0].coord[1];

  for (i = 0 ; i < nb_point+1 ; i++){
    if (mon_poly[i].coord[0] >= x && mon_poly[i].coord[1] > y)       transition[i] = 1;
    else if (mon_poly[i].coord[0] > x && mon_poly[i].coord[1] < y)  transition[i] = 2;
    else if (mon_poly[i].coord[0] < x && mon_poly[i].coord[1] < y)  transition[i] = 3;
    else if (mon_poly[i].coord[0] < x && mon_poly[i].coord[1] > y)  transition[i] = 4;
    else if (mon_poly[i].coord[0] > x && mon_poly[i].coord[1] == y) transition[i] = 5;
    else transition[i] = 6;
  }

  for (i = 0 ; i < nb_point ; i++){
    if ( (transition[i] == 1 && transition[i+1] == 2) || (transition[i] == 2 && transition[i+1] == 1) )
      NI++;
    else if ( (transition[i] == 2 && transition[i+1] == 4) || (transition[i] == 4 && transition[i+1] == 2) )
      NI += delit_cas(i, x, y, 1);
    else if ( (transition[i] == 1 && transition[i+1] == 3) || (transition[i] == 3 && transition[i+1] == 1) )
      NI += delit_cas(i, x, y, 1);
    else if ( transition[i] == 5 )
      NI += delit_cas(i, x, y, 0);
  }

  if ( NI%2 != 0 && NI != 0) return 1;
  else return 0;
}

/*Fonction qui détécte si un point est déjà présent ou non*/
int isAlreadyPoint(int x, int y){
  int i;
  int x_left, x_right, y_top, y_bottom, x_point, y_point;
  int neighboor = 10;

      x_left = x - neighboor;
      x_right = x + neighboor;

      y_bottom = y - neighboor;
      y_top = y + neighboor;

  for (i=0 ; i < nb_point ; i++){

      x_point = mon_poly[i].coord[0];
      y_point = mon_poly[i].coord[1];

    if ( (x_point < x_right &&  x_point > x_left) && (y_point < y_top &&  y_point > y_bottom) ){
      return 1;
    }
  }
  return 0;
}
//Fonction qui détermine le points le plus proche de la souris
int nearestPoint (int x, int y)
{
  int i;
  int x_left, x_right, y_left, y_right , x_point, y_point;
  int neighboor = 10;

      x_left = x - neighboor;
      x_right = x + neighboor;

      y_left = y - neighboor;
      y_right = y + neighboor;

  for (i=0 ; i < nb_point ; i++){

      x_point = mon_poly[i].coord[0];
      y_point = mon_poly[i].coord[1];

    if ( (x_point < x_right &&  x_point > x_left) && (y_point < y_right &&  y_point > y_left) ){
      return i;
    }
  }
  return -1;
}


/*Fonction qui ajoute un point*/
void addAPoint (int x, int y)
{
  int i;
  if ( nb_point < 3 )
    insert_here = nb_point;

  if (nb_point != insert_here){
    for (i = nb_point ; i > insert_here ; i--){
        mon_poly[i+1] = mon_poly[i];
    }
    insert_here ++;
  }

  mon_poly[insert_here].coord[0] = x;
  mon_poly[insert_here].coord[1] = y;
  mon_poly[insert_here].coord[2] = 0;
  nb_point++;

}

/*Fonction qui ajoute un point*/
void addAHole (int x, int y)
{
  int i;
  insert_hole_here = nb_hole;
  for (i = nb_hole; i > insert_hole_here; i--)
  {
    trou_poly[i + 1] = trou_poly[i];
  }

  trou_poly[insert_hole_here].coord[0] = x;
  trou_poly[insert_hole_here].coord[1] = y;
  trou_poly[insert_hole_here].coord[2] = 0;
  nb_hole++;

}

/*Fonction qui supprime un point donné*/
void deleteAPoint (int indice)
{
  int i;
  for (i=indice ; i < nb_point ; i++)
    mon_poly[i] = mon_poly[i+1];
  nb_point--;
}

/* Callback OpenGL d'affichage */
void displayGL()
{
  int i;
  char buffer[32];
  glClear(GL_COLOR_BUFFER_BIT);

  glPrintText(10, winY - 10, int_ext);
  
  glColor3f(0.4, 0.1, 0.8);
  if (isASecant())
    glPrintText(10, winY - 25, "Secant!");

  if (edit_mode)
    glPrintText(10, winY - 40, "Mode d'edition");

  if (coord_mode)
  {
    glColor3f(0.8, 0.8, 0.8);
    for (i = 0; i < nb_point; ++i)
    {
      sprintf(buffer, "[%02d] (%3d, %3d)", i, mon_poly[i].coord[0], mon_poly[i].coord[1]);
      glPrintText(mon_poly[i].coord[0], mon_poly[i].coord[1] + 6, buffer);
    }
    glEnd();
  }

  glBegin(GL_LINE_LOOP);
  for (i = 0; i < nb_hole; i++)
  {
    glVertex3f(trou_poly[i].coord[0], trou_poly[i].coord[1], trou_poly[i].coord[2]);
  }
  glEnd();

  for (i = 0; i < nb_point; i++)
  {
    if (i == insert_here){
      glColor3f(0.8,0.8,0.1); glPointSize(8.0);
      glBegin(GL_POINTS);
        glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
      glEnd();
    }
    else if (i == motion){
      glColor3f(0.6,0.5,0.8); glPointSize(10.0);
      glBegin(GL_POINTS);
        glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
      glEnd();
    }
    else {
      glColor3f(0.8,.1,0.1); glPointSize(5.0);
      glBegin(GL_POINTS);
        glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
      glEnd();}
  }

  if (nb_point > 2 )
  {
    glColor3f(0.5,.2,0.8);
    switch (display_mode) {
      case 2 :
        glBegin(GL_LINES);
        for(i=0 ; i < nb_point ; i++){
          glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
        }
        glEnd();
      break;
      case 3 :
        glBegin(GL_LINE_LOOP);
        for(i=0 ; i < nb_point ; i++){
          glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
        }
        glEnd();
      break;
      case 4 :
        glBegin(GL_LINE_STRIP);
        for(i=0 ; i < nb_point ; i++){
          glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
        }
        glEnd();
      break;
      case 5 :
        glBegin(GL_POLYGON);
        for(i=0 ; i < nb_point ; i++){
          glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
        }
        glEnd();
      break;
    }
  }
	glutSwapBuffers();
}

void passivemouseGL (int x, int y)
{ 
  y = winY-y;
  if(isInside(x, y))
  {
    int_ext = "Interieur";
    mouse_is_inside = 1;
  }
  else{
    int_ext = "Exterieur";
    mouse_is_inside = 0;
  }
  glutPostRedisplay();
}

/* Callback OpenGL de gestion de souris */
void mouseGL(int button, int state, int x, int y)
{
  y = winY-y;
  button_pressed = state;
  if (state ==  GLUT_DOWN && button == GLUT_LEFT_BUTTON)
  {
    if(!isAlreadyPoint(x,y)){
      motion = nb_point;
        if (edit_mode && !add_hole_mode){
          addAPoint(x,y);
          printf("nb_point = %d\n",nb_point);
          insert_here = nb_point;
        }
        if (edit_mode && add_hole_mode)
        {
          addAHole(x, y);
        }
      }
      else{
        motion = nearestPoint(x,y);
    }
  }
  if (state ==  GLUT_DOWN && button == GLUT_RIGHT_BUTTON){
    motion = -1;
    if (isAlreadyPoint(x,y)){
      deleteAPoint(nearestPoint(x,y));
      printf("nb_point = %d\n",nb_point);
    }
  }
  if (state ==  GLUT_DOWN && button == GLUT_MIDDLE_BUTTON){
    motion = -1;
    if (isAlreadyPoint(x,y)){
      insert_here = nearestPoint(x,y);
    }
  }

  glutPostRedisplay();
}


/* Callback OpenGL de gestion de drag */
void motionGL(int x, int y)
{
	y = winY - y;
  int i;

	if(button_pressed == GLUT_LEFT_BUTTON && motion != -1)
	{
    mon_poly[motion].coord[0] = x;
    mon_poly[motion].coord[1] = y;
	}
  if (button_pressed == GLUT_LEFT_BUTTON && isInside(x,y) && !edit_mode)
  {
    for (i=0 ; i < nb_point ; i++)
    {
      mon_poly[i].coord[0] = x + (x - mon_poly[i].coord[0]);
      mon_poly[i].coord[1] = y + (y - mon_poly[i].coord[1]);
    }
  }
	glutPostRedisplay();
}

/*Fonction d'initialisation de nos variables*/
void init()
{
  int i;

  nb_point        = 0;
  nb_point_max    = 100;
  nb_hole         = 0;
  nb_hole_max     = 100;
  motion          = -1;
  button_pressed  = 0;
  edit_mode       = 1;
  insert_here     = 0;
  insert_hole_here= 0;
  display_mode    = 3;
  add_hole_mode   = 0;
  coord_mode      = 0;
  mouse_is_inside = 0;
  int_ext         = "Exterieur";


  transition = malloc((nb_point_max + 1) * sizeof(Poly));
  mon_poly = malloc (nb_point_max * sizeof(Poly));
  trou_poly = malloc (nb_hole_max * sizeof(Poly));

  //Nous ne travaillerons pas avec dimension z
  for (i=0 ; i < nb_point_max ; i++){
    mon_poly[i].coord[2] = 0;
    trou_poly[i].coord[2] = 0;
  }
}

/* Callback OpenGL de gestion de clavier */
void keyboardGL(unsigned char k, int _x, int _y)
{
  switch (k){

    case 27 :
		      free(mon_poly);
		      exit(0);
          break;

    case 'm' :
		      edit_mode = !edit_mode;
          break;

    case 'e' :
          free(mon_poly);
          init();
          break;

    case 'c' :
          coord_mode = !coord_mode;
          break;

    case 'h' :
          add_hole_mode = !add_hole_mode;
          break;

    case '+' :
        if (display_mode == 5)
          display_mode = 1;
        else display_mode ++;
          break;


  }
    glutPostRedisplay();
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
  glutMotionFunc(motionGL);
  glutPassiveMotionFunc(passivemouseGL);

  initGL();
  glutMainLoop();

  return 0;
}
