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

typedef struct Point
{
  int coord[3];
} Point;

typedef struct Poly
{
  Point* points;
  struct Poly** hole;
  int nb_point;
  int insert_here;
  int nb_hole;
}Poly;

Poly* list_poly;

int motion;
int motion_hole;
int nb_poly;
int nb_poly_max;
int nb_point_max;
int poly_selected;
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
	glClearColor(0.7, 0.7, 0.7, 1.0);
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

void addAPoly (void)
{
  int i;
  list_poly[nb_poly].nb_point = 0;
  list_poly[nb_poly].insert_here = 0;
  list_poly[nb_poly].nb_hole = 0;
  list_poly[nb_poly].points = malloc( nb_point_max + 1 * sizeof(Point) );

  for (i = 0 ; i < nb_point_max ; i++ ){
    list_poly[nb_poly].points[i].coord[2] = 0;
  }

      if (nb_poly < nb_poly_max)
          nb_poly++;
}

/*Transforme deux points donnés en un vecteurs*/
Point madeAVector(Point A, Point B)
{
  Point new_point;
  new_point.coord[0] = B.coord[0] - A.coord[0];
  new_point.coord[1] = B.coord[1] - A.coord[1];
  return new_point;
}

/*Determine si il y a deux arrêtes sécantes*/
int isASecant (void)
{
  int x;
    int i,j;
    int signe1 , signe2;
    Point ac, bc, ad, bd, ca, da, cb, db;
    int nb_points;
    Poly current_poly;
    
    for (x = 0; x < nb_poly; x++)
    {
      current_poly = list_poly[x];
      nb_points = current_poly.nb_point;

      current_poly.points[nb_points].coord[0] = current_poly.points[nb_points].coord[0];
      current_poly.points[nb_points].coord[1] = current_poly.points[nb_points].coord[1];

      if (nb_points > 3)
      {
        for (i = 0; i < nb_points; i++)
          for (j = nb_points - 1; j > 1; j--)
          {
            ca = madeAVector(current_poly.points[j], current_poly.points[i]);
            da = madeAVector(current_poly.points[j - 1], current_poly.points[i]);
            cb = madeAVector(current_poly.points[j], current_poly.points[i + 1]);
            db = madeAVector(current_poly.points[j - 1], current_poly.points[i + 1]);

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
            ac = madeAVector(current_poly.points[i], current_poly.points[j]);
            ad = madeAVector(current_poly.points[i], current_poly.points[j - 1]);
            bc = madeAVector(current_poly.points[i + 1], current_poly.points[j]);
            bd = madeAVector(current_poly.points[i + 1], current_poly.points[j - 1]);

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
  }
  return 0;
}

/*Fonction permettant de traiter les cas non évidents*/
int delit_cas (int i, int x, int y, int option, int which_poly)
{

  if (option)
  {
    Poly working_poly = list_poly[which_poly];
    double a, b, _x;

    if (working_poly.points[i + 1].coord[1] == working_poly.points[i].coord[1])
      return 1;

    a = (double)(working_poly.points[i + 1].coord[1] - working_poly.points[i].coord[1]) / (double)(working_poly.points[i + 1].coord[0] - working_poly.points[i].coord[0]);
    b = (double)working_poly.points[i + 1].coord[1] - (working_poly.points[i + 1].coord[0] * a);

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

/*Fonction qui nous renseigne si un point est dans un Polygone*/
int isInside(int x, int y, Poly current_poly)
{
  int i;
  int is_it_inside;
  int NI;
  int nb_points;
  nb_points = current_poly.nb_point;
  NI = 0;
  //Duplication de la première case sur la dernière pour "fermé" notre polygone
  current_poly.points[nb_points].coord[0] = current_poly.points[0].coord[0];
  current_poly.points[nb_points].coord[1] = current_poly.points[0].coord[1];

  for (i = 0 ; i < nb_points+1 ; i++)
  {
    if (current_poly.points[i].coord[0] >= x && current_poly.points[i].coord[1] > y)
      transition[i] = 1;
    else if (current_poly.points[i].coord[0] > x && current_poly.points[i].coord[1] < y)
      transition[i] = 2;
    else if (current_poly.points[i].coord[0] < x && current_poly.points[i].coord[1] < y)
      transition[i] = 3;
    else if (current_poly.points[i].coord[0] < x && current_poly.points[i].coord[1] > y)
      transition[i] = 4;
    else if (current_poly.points[i].coord[0] > x && current_poly.points[i].coord[1] == y)
      transition[i] = 5;
  }

  for (i = 0 ; i < nb_points ; i++)
  {
    if ( (transition[i] == 1 && transition[i+1] == 2) || (transition[i] == 2 && transition[i+1] == 1) )
      NI++;
    else if ( (transition[i] == 2 && transition[i+1] == 4) || (transition[i] == 4 && transition[i+1] == 2) )
      NI += delit_cas(i, x, y, 1, 1);
    else if ( (transition[i] == 1 && transition[i+1] == 3) || (transition[i] == 3 && transition[i+1] == 1) )
      NI += delit_cas(i, x, y, 1, 1);
    else if ( transition[i] == 5 )
      NI += delit_cas(i, x, y, 0, 1);
  }

  if ( NI%2 != 0 && NI != 0)
    is_it_inside = 1;
  else
    is_it_inside = 0;

  //Si notre curseur se trouve dans un polygone on vérifie alors qu'il ne se trouve pas dans un trou
  if (is_it_inside && current_poly.nb_hole > 0)
  {
    //if ( isInside(x, y, current_poly->hole[current_poly.nb_hole--]) )
      //is_it_inside = !is_it_inside
  }
  
    return is_it_inside;
}

//Fonction qui détermine le points le plus proche de la souris
Point nearestPoint (int x, int y)
{
  int i, k;
  int x_left, x_right, y_left, y_right , x_point, y_point;
  int neighboor = 10;
  int nb_points;
  Poly working_poly;
  Point near_point;
  near_point.coord[0] = -1;
  near_point.coord[1] = -1;

  for (k = 0 ; k < nb_poly ; k++)
  {
    working_poly = list_poly[k];
    nb_points = working_poly.nb_point;
    x_left = x - neighboor;
    x_right = x + neighboor;

    y_left = y - neighboor;
    y_right = y + neighboor;

    for (i=0 ; i < nb_points ; i++)
    {
      x_point = working_poly.points[i].coord[0];
      y_point = working_poly.points[i].coord[1];

      if ( (x_point < x_right &&  x_point > x_left) && (y_point < y_right &&  y_point > y_left) ){
        near_point.coord[0] = k;
        near_point.coord[1] = i;
        return near_point;
      }
    }
  }
  return near_point;
}

/*Fonction qui ajoute un point*/
void addAPoint (int x, int y)
{
  int i;

  if (list_poly[poly_selected].nb_point < 3)
    list_poly[poly_selected].insert_here = list_poly[poly_selected].nb_point;

  if (list_poly[poly_selected].nb_point != list_poly[poly_selected].insert_here)
  {
    for (i = list_poly[poly_selected].nb_point; i > list_poly[poly_selected].insert_here; i--)
    {
      list_poly[poly_selected].points[i + 1] = list_poly[poly_selected].points[i];
    }
    list_poly[poly_selected].insert_here++;
  }

  list_poly[poly_selected].points[list_poly[poly_selected].insert_here].coord[0] = x;
  list_poly[poly_selected].points[list_poly[poly_selected].insert_here].coord[1] = y;
  list_poly[poly_selected].nb_point++;
}

/*Fonction qui supprime un point donné*/
/*oid deleteAPoint (int indice)
{
  int i;
  for (i=indice ; i < nb_point ; i++)
    mon_poly[i] = mon_poly[i+1];
  nb_point--;
}*/

/* Callback OpenGL d'affichage */
void displayGL()
{
  int i;
  int k;
  char buffer[32];
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.4, 0.1, 0.8);

  glPrintText(10, winY - 10, int_ext);

  sprintf(buffer, "Nombre de Polygnes : %d",list_poly[poly_selected].nb_point);
  glPrintText(10, winY - 55, buffer);

  if (isASecant())
    glPrintText(10, winY - 25, "Secant!");

  if (edit_mode)
    glPrintText(10, winY - 40, "Mode d'edition");

  /*if (coord_mode)
  {
    glColor3f(0.4, 0.1, 0.8);
    for (i = 0; i < nb_point; ++i)
    {
      sprintf(buffer, "[%02d] (%3d, %3d)", i, mon_poly[i].coord[0], mon_poly[i].coord[1]);
      glPrintText(mon_poly[i].coord[0], mon_poly[i].coord[1] + 6, buffer);
    }
    glColor3f(0.8, 0.1, 0.4);
    for (i = 0; i < nb_hole; ++i)
    {
      sprintf(buffer, "[%02d] (%3d, %3d)", i, hole_poly[i].coord[0], hole_poly[i].coord[1]);
      glPrintText(hole_poly[i].coord[0], hole_poly[i].coord[1] + 6, buffer);
    }
  }*/

  /*for (i = 0; i < nb_point; i++)
  {
    if (i == insert_here && !add_hole_mode){
      glColor3f(0.8,0.8,0.1); glPointSize(8.0);
      glBegin(GL_POINTS);
        glVertex3f(mon_poly[i].coord[0],mon_poly[i].coord[1],mon_poly[i].coord[2]);
      glEnd();
    }
    else if (i == motion && !add_hole_mode)
    {
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

  for (i = 0; i < nb_hole; i++)
  {
    if (i == insert_hole_here){
      glColor3f(0.1,0.8,0.1); glPointSize(8.0);
      glBegin(GL_POINTS);
      glVertex3f(hole_poly[i].coord[0], hole_poly[i].coord[1], hole_poly[i].coord[2]);
      glEnd();
    }
    else if (i == motion && add_hole_mode){
      glColor3f(0.1,0.5,0.8); glPointSize(10.0);
      glBegin(GL_POINTS);
      glVertex3f(hole_poly[i].coord[0], hole_poly[i].coord[1], hole_poly[i].coord[2]);
      glEnd();
    }
    else {
      glColor3f(0.8, 0.1, 0.1); glPointSize(5.0);
      glBegin(GL_POINTS);
      glVertex3f(hole_poly[i].coord[0], hole_poly[i].coord[1], hole_poly[i].coord[2]);
      glEnd();}
  }*/
  Poly current_poly = list_poly[poly_selected];

    glPointSize(10.0);
    glColor3f(0.8, 0.2, 0.2);
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < current_poly.nb_point; i++){
      glVertex3f(current_poly.points[i].coord[0], current_poly.points[i].coord[1], current_poly.points[i].coord[2]);
      printf("%d\n",current_poly.points[i].coord[2]);}
    glEnd();

  glutSwapBuffers();
}

/*void passivemouseGL (int x, int y)
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
}*/

/* Callback OpenGL de gestion de souris */
void mouseGL(int button, int state, int x, int y)
{
  y = winY-y;
  button_pressed = state;
  if (state ==  GLUT_DOWN && button == GLUT_LEFT_BUTTON)
  {
    if(1){
      if (edit_mode && !add_hole_mode){
          //motion = nb_point;
          addAPoint(x,y);
          //insert_here = nb_point;
        }
        /*if (edit_mode && add_hole_mode)
        {
          //motion = nb_hole;
          addAHole(x, y);
          insert_hole_here = nb_hole;
        }*/
      }
      else{
        //motion = nearestPoint(x,y);
      }
  }
  /*if (state ==  GLUT_DOWN && button == GLUT_RIGHT_BUTTON){
    motion = -1;
    if (nearestPoint(x, y) != -1)
    {
      deleteAPoint(nearestPoint(x,y));
      insert_here--;
    }
  }
  if (state ==  GLUT_DOWN && button == GLUT_MIDDLE_BUTTON){
    motion = -1;
    if (nearestPoint(x, y) != -1)
    {
      insert_here = nearestPoint(x,y);
    }
  }*/

  glutPostRedisplay();
}


/* Callback OpenGL de gestion de drag */
/*void motionGL(int x, int y)
{
	y = winY - y;
  int i;

	if(button_pressed == GLUT_LEFT_BUTTON && motion != -1 && !add_hole_mode)
	{
    mon_poly[motion].coord[0] = x;
    mon_poly[motion].coord[1] = y;
	}
	if(button_pressed == GLUT_LEFT_BUTTON && motion != -1 && add_hole_mode)
	{
    hole_poly[motion].coord[0] = x;
    hole_poly[motion].coord[1] = y;
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
}*/

/*Fonction d'initialisation de nos variables*/
void init()
{
  int i;

  nb_poly         = 0;
  nb_poly_max     = 5;
  nb_point_max    = 100;
  motion          = -1;
  motion_hole     = -1;
  poly_selected   = 0;
  button_pressed  = 0;
  edit_mode       = 1;
  display_mode    = 2;
  add_hole_mode   = 0;
  coord_mode      = 0;
  mouse_is_inside = 0;
  int_ext         = "Exterieur";

  list_poly = malloc( (nb_poly_max + 1) * sizeof(Poly) );
  addAPoly();
  transition = malloc( (nb_point_max + 1) * sizeof(int) );
  
}

/* Callback OpenGL de gestion de clavier */
void keyboardGL(unsigned char k, int _x, int _y)
{
  switch (k){

    case 27 :
		      free(list_poly);
		      exit(0);
          break;

    case 'm' :
		      edit_mode = !edit_mode;
          break;

    case 'e' :
          free(list_poly);
          init();
          break;

    case 'c' :
          coord_mode = !coord_mode;
          break;

    case 'h' :
          add_hole_mode = !add_hole_mode;
          break;

    case '+' :
        if (display_mode == 4)
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
  //glutMotionFunc(motionGL);
  //glutPassiveMotionFunc(passivemouseGL);

  initGL();
  glutMainLoop();

  return 0;
}
