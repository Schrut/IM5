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


//Info Structure while be useful when we need to communicate info
typedef struct Info
{
  int which_poly;
  int which_point;
  int is_a_hole;
} Info;


//Point Structure
typedef struct Point
{
  int coord[3];
} Point;

//Polygone Structure
typedef struct Poly
{
  Point* points;
  struct Poly** hole;
  int nb_point;
  int insert_here;
  int nb_hole;
}Poly;

Poly* list_poly;

Info motion;

int motion_hole;
int nb_poly;
int nb_poly_max;
int nb_point_max;
int nb_hole_max;
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

void addAHole (void)
{
	list_poly[poly_selected].hole = malloc((nb_hole_max + 1) * sizeof(Poly));
	int i;
	for (i = 0 ; i < nb_hole_max , )
	

}

void addAPoly (void)
{
	if (nb_poly < nb_poly_max){
  int i;
  list_poly[nb_poly].nb_point = 0;
  list_poly[nb_poly].insert_here = 0;
  list_poly[nb_poly].nb_hole = 0;
  list_poly[nb_poly].points = malloc( 3*(nb_point_max) + 1 * sizeof(Point) );
			for (i = 0; i < nb_point_max; i++)
	{
		list_poly[nb_poly].points[i].coord[2] = 0;
  }

	nb_poly++;
	}
}

/*Transforme deux points donnés en un vecteurs*/
Point madeAVector(Point A, Point B)
{
  Point new_point;
  new_point.coord[0] = B.coord[0] - A.coord[0];
  new_point.coord[1] = B.coord[1] - A.coord[1];
  return new_point;
}

/*Is a secant in my polygon?*/
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

			//Again here we close our polygon
      current_poly.points[nb_points].coord[0] = current_poly.points[nb_points].coord[0];
      current_poly.points[nb_points].coord[1] = current_poly.points[nb_points].coord[1];
      current_poly.points[nb_points].coord[2] = 0;
			//If we hve less than 3 points our poly isn't close yet so no need to check if we're inside it
      if (nb_points > 3)
      {
        for (i = 0; i < (nb_points/2)+1 ; i++)
					for (j = nb_points; j > (nb_points / 2) - 1 ; j--)
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
    if (transition[i + 1] == transition[i] )
      return 0;
    else if ( (transition[i - 1] == 1 && transition[i + 1] == 2) || (transition[i - 1] == 2 && transition[i + 1] == 1) )
      return 1;
    else if ( (transition[i - 1] == 4 && transition[i + 1] == 2) || (transition[i - 1] == 2 && transition[i + 1] == 4) )
      return 1;
    else
      return 0;
  }
}

/*Is this point inside a poly?*/
int isInside(int x, int y)
{
  /* To see if a point is inside we check its position by the poly's points.
  *    
  *               |
  *      TL       |       TR
  *               |
  * --------------------------------
  *               |
  *      BL       |       BR
  *               |
  *
  * Once we have divided our space if four, where the center is our point we have 5 cases : 
  * if the point is TR : transition = 1
  * if the point is BR : transition = 2
  * if the point is BL : transition = 3
  * if the point is TL : transition = 4
  * if the point is right between TR & BR : transition = 5
  *
  * We count how many points is cutting the axe between TR and BR if it's odd the point we are looking is inside
  * if it's even or null it's outside
  *
  * Some case are tricky. This is what 'delits-cas' does :
  * The case where one poly's point is TR and the other BL or the opposite
  * In these cases the axe can be cross but we need more process.
  * What we do here is to looking for cutting point using line's equations define by points and axe.
  * An other tricky case is if two point are on the axe. In this case we look the previous and next points to know where they are */

  int i;
  int is_it_inside;
  int NI;
  int nb_points;
  int k;
  Poly temp_poly;
	Point current_point;

  for (k = 0; k < nb_poly; k++)
  {
    
    temp_poly = list_poly[k];

    nb_points = temp_poly.nb_point;
    NI = 0;
    //Duplicate the first point to the last one + 1 to close our polygone
    temp_poly.points[nb_points].coord[0] = temp_poly.points[0].coord[0];
    temp_poly.points[nb_points].coord[1] = temp_poly.points[0].coord[1];
    temp_poly.points[nb_points].coord[2] = 0;

				for (i = 0; i < nb_points + 1; i++)
		{
			current_point = temp_poly.points[i];
			 if (current_point.coord[0] >= x && current_point.coord[1] > y){
				transition[i] = 1;
			 }
			 else if (current_point.coord[0] > x && current_point.coord[1] < y){
					 transition[i] = 2;
			 }
			 else if (current_point.coord[0] < x && current_point.coord[1] < y){
					 transition[i] = 3;
			 }
			 else if (current_point.coord[0] < x && current_point.coord[1] > y){
					 transition[i] = 4;
			 }
			 else if (current_point.coord[0] > x && current_point.coord[1] == y){
					 transition[i] = 5;
			 }
		}
		

    for (i = 0 ; i < nb_points ; i++)
    {
      if ( (transition[i] == 1 && transition[i+1] == 2) || (transition[i] == 2 && transition[i+1] == 1) )
        NI++;
      else if ( (transition[i] == 2 && transition[i+1] == 4) || (transition[i] == 4 && transition[i+1] == 2) )
        NI += delit_cas(i, x, y, 1, k);
      else if ( (transition[i] == 1 && transition[i+1] == 3) || (transition[i] == 3 && transition[i+1] == 1) )
				NI += delit_cas(i, x, y, 1, k);
			else if ( transition[i] == 5 )
				NI += delit_cas(i, x, y, 0, k);
		}
    if ( NI%2 != 0 && NI != 0){
      return 1;
			if (k == poly_selected)
				return 2;
		}
    else
      is_it_inside = 0;

    //Si notre curseur se trouve dans un polygone on vérifie alors qu'il ne se trouve pas dans un trou
    /*if (is_it_inside && current_poly.nb_hole > 0)
    {
      if ( isInside(x, y, current_poly.hole[current_poly.nb_hole--]) )
        is_it_inside = !is_it_inside
    }*/
  }

	return is_it_inside;
}

//Which point is the closest to the mouse?
Info nearestPoint (int x, int y)
{
  int i, k;
  int x_left, x_right, y_left, y_right , x_point, y_point;
  int neighboor = 10;
  int nb_points;
  Poly working_poly;
  Info is_a_point;
  is_a_point.which_poly = -1;
  is_a_point.which_point = -1;
  is_a_point.is_a_hole = -1;

  for (k = 0 ; k < nb_poly ; k++)
  {
    working_poly = list_poly[k];
    nb_points = working_poly.nb_point;
    x_left = x - neighboor;
    x_right = x + neighboor;

    y_left = y - neighboor;
    y_right = y + neighboor;

		//Check the surrounding looking for points
    for (i=0 ; i < nb_points ; i++)
    {
      x_point = working_poly.points[i].coord[0];
      y_point = working_poly.points[i].coord[1];
      if ((x_point < x_right && x_point > x_left) && (y_point < y_right && y_point > y_left))
      {
        is_a_point.which_poly = k;
        is_a_point.which_point = i;
        return is_a_point;
      }
    }
  }
	//If is a point we return the poly and point number or -1 and -1 if it's no point here
  return is_a_point;
}

//Is it a point by there?
int isAPointNear (int x, int y)
{
  int i, k;
  int x_left, x_right, y_left, y_right , x_point, y_point;
  int neighboor = 10;
  int nb_points;
  Poly working_poly;

  for (k = 0 ; k < nb_poly ; k++)
  {
    working_poly = list_poly[k];
    nb_points = working_poly.nb_point;
    x_left = x - neighboor;
    x_right = x + neighboor;

    y_left = y - neighboor;
    y_right = y + neighboor;
		//Check the surrounding looking for points
		for (i=0 ; i < nb_points ; i++)
    {
      x_point = working_poly.points[i].coord[0];
      y_point = working_poly.points[i].coord[1];

      if ( (x_point < x_right &&  x_point > x_left) && (y_point < y_right &&  y_point > y_left) ){
        return 1;
      }
    }
  }
	//If a point is near the mouse we return 1 else 0
  return 0;
}

/*Add a point in our poly*/
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
  list_poly[poly_selected].points[list_poly[poly_selected].insert_here].coord[2] = 0;
  list_poly[poly_selected].nb_point++;
}

/*Fonction qui supprime un point donné*/
/*void deleteAPoint (int indice)
{
  int i;
  for (i=indice ; i < nb_point ; i++)
    mon_poly[i] = mon_poly[i+1];
  nb_point--;
}*/

/* Callback OpenGL */
void displayGL()
{
  int i;
  int k;
  char buffer[32];
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.4, 0.1, 0.8);

  glPrintText(10, winY - 10, int_ext);

  sprintf(buffer, "Number of Points : %d",list_poly[poly_selected].nb_point);
  glPrintText(10, winY - 55, buffer);

  sprintf(buffer, "Selected Polygon : %d",poly_selected+1);
  glPrintText(10, winY - 70, buffer);

  if (isASecant())
    glPrintText(10, winY - 25, "Secant!");

  if (edit_mode)
    glPrintText(10, winY - 40, "Mode d'edition");


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
  for (k = 0 ; k < nb_poly ; k++)
	{
		Poly current_poly = list_poly[k];
		if (coord_mode)
		{
			glColor3f(0.4, 0.1, 0.8);
			for (i = 0; i < current_poly.nb_point; ++i)
			{
				sprintf(buffer, "[%02d] (%3d, %3d)", i, current_poly.points[i].coord[0], current_poly.points[i].coord[1]);
				glPrintText(current_poly.points[i].coord[0], current_poly.points[i].coord[1] + 6, buffer);
			}
		}
    for (i = 0; i < current_poly.nb_point; i++)
    {

			glPointSize(8.0);
			glBegin(GL_POINTS);
      if (i == current_poly.insert_here && k == poly_selected)
      {
        glColor3f(0.2, 0.5, 0.5);
        glVertex3f(current_poly.points[i].coord[0], current_poly.points[i].coord[1], current_poly.points[i].coord[2]);
      }
      else 
      {
        glColor3f(0.9, 0.2, 0.1);
        glVertex3f(current_poly.points[i].coord[0], current_poly.points[i].coord[1], current_poly.points[i].coord[2]);
      }
			glEnd();
		}
			glPointSize(1.0);
			if (k == poly_selected)
				glColor3f(0.2, 0.2, 0.8);
			else
				glColor3f(0.2,0.2,0.2);
			glBegin(GL_LINE_LOOP);
			for (i = 0; i < current_poly.nb_point; i++)
			{
				glVertex3f(current_poly.points[i].coord[0], current_poly.points[i].coord[1], current_poly.points[i].coord[2]);
			}
			glEnd();
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
    if (!isAPointNear(x,y))
    {
			motion.which_poly = -1;
      if (edit_mode && !add_hole_mode){
          addAPoint(x,y);
        }
        /*if (edit_mode && add_hole_mode)
        {
          //motion = nb_hole;
          addAHole(x, y);
          insert_hole_here = nb_hole;
        }*/
      }
      else{
        motion = nearestPoint(x,y);
      }
  }
  /*if (state ==  GLUT_DOWN && button == GLUT_RIGHT_BUTTON){
    motion = -1;
    if (nearestPoint(x, y) != -1)
    {
      deleteAPoint(nearestPoint(x,y));
      insert_here--;
    }
  }*/
  if (state ==  GLUT_DOWN && button == GLUT_MIDDLE_BUTTON){
    //motion = -1;
    Info is_a_point_there = nearestPoint(x, y);
    if (is_a_point_there.which_poly != -1)
    {
     // list_poly[is_a_point_there.which_poly].insert_here = is_a_point_there.which_point;
    }
  }

  glutPostRedisplay();
}


/* Callback OpenGL de gestion de drag */
void motionGL(int x, int y)
{
	y = winY - y;
  

	if(button_pressed == GLUT_LEFT_BUTTON && motion.which_poly != -1 )
	{
		list_poly[motion.which_poly].points[motion.which_point].coord[0] = x;
		list_poly[motion.which_poly].points[motion.which_point].coord[1] = y;
	}
	/*if(button_pressed == GLUT_LEFT_BUTTON && motion != -1 && add_hole_mode)
	{
    hole_poly[motion].coord[0] = x;
    hole_poly[motion].coord[1] = y;
	}*/
  /*if (button_pressed == GLUT_LEFT_BUTTON && isInside(x,y) && !edit_mode)
  {
    for (i=0 ; i < nb_point ; i++)
    {
      mon_poly[i].coord[0] = x + (x - mon_poly[i].coord[0]);
      mon_poly[i].coord[1] = y + (y - mon_poly[i].coord[1]);
    }*/
	glutPostRedisplay();
}

/*Fonction d'initialisation de nos variables*/
void init()
{
  int i;

  nb_poly         = 0;
  nb_poly_max     = 5;
  nb_point_max    = 100;
  nb_hole_max    	= 5;
  motion.which_point = -1;
  motion.which_poly = -1;
  motion.is_a_hole = -1;
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
	for (i = 0 ; i < nb_poly_max ; i++)
	{
		addAPoly();
	}
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

    case 9 :
					if (poly_selected < nb_poly-1)
		      {
						poly_selected++;
					}
					else
					{ 
						poly_selected = 0;
					}
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
  glutMotionFunc(motionGL);
  glutPassiveMotionFunc(passivemouseGL);

  initGL();
  glutMainLoop();

  return 0;
}
