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
	int active;
} Point;

//Polygone Structure
typedef struct Poly
{
	Point *points;
	int nb_point;
	int insert_here;
	int which_poly;
} Poly;

Poly* list_poly;
Poly* list_hole;

Info* Yposition;
int* segment_drawing;
Info motion;
Info motionX;
Info motionY;

int motion_hole;
int max_sample;
int nb_sample;
int nb_poly;
int nb_hole;
int nb_hole_max;
int nb_poly_max;
int nb_point_max;
int nb_point_hole_max;
int poly_selected;
int hole_selected;
int button_pressed;
int button_state;
int add_hole_mode;
int edit_mode;
int coord_mode;
int display_mode;
int xpos;
int ypos;
int mouse_is_inside;
int nbYposition;
int *transition;
const char* int_ext;

/*Transforme deux points donnés en un vecteurs*/
Point madeAVector(Point A, Point B)
{
	Point new_point;
	new_point.coord[0] = B.coord[0] - A.coord[0];
	new_point.coord[1] = B.coord[1] - A.coord[1];
	return new_point;
}



/*Is a secant in my polygon?*/
int isASecant(Poly current_poly)
{
	int i, j;
	int signe1, signe2;
	Point ac, bc, ad, bd, ca, da, cb, db;
	int nb_points;
	int nb_secant;

	nb_points = current_poly.nb_point;
	nb_secant = 0;
	//Again here we close our polygon
	//Poly
	current_poly.points[nb_points].coord[0] = current_poly.points[0].coord[0];
	current_poly.points[nb_points].coord[1] = current_poly.points[0].coord[1];
	current_poly.points[nb_points].coord[2] = 0;

	//If we hve less than 3 points our poly isn't close yet, so no need to check if it is secant
	if (nb_points > 3)
	{
		for (i = 0; i < nb_points / 2 + 1; i++)
			for (j = nb_points; j > nb_points / 2 - 1; j--)
			{
				ca = madeAVector(current_poly.points[j], current_poly.points[i]);
				da = madeAVector(current_poly.points[j - 1], current_poly.points[i]);
				cb = madeAVector(current_poly.points[j], current_poly.points[i + 1]);
				db = madeAVector(current_poly.points[j - 1], current_poly.points[i + 1]);

				if ((ca.coord[0] * cb.coord[1] - cb.coord[0] * ca.coord[1]) >= 0)
					signe1 = 1;
				else
					signe1 = 0;

				if ((da.coord[0] * db.coord[1] - db.coord[0] * da.coord[1]) >= 0)
					signe2 = 1;
				else
					signe2 = 0;

				if (signe1 != signe2)
				{
					ac = madeAVector(current_poly.points[i], current_poly.points[j]);
					ad = madeAVector(current_poly.points[i], current_poly.points[j - 1]);
					bc = madeAVector(current_poly.points[i + 1], current_poly.points[j]);
					bd = madeAVector(current_poly.points[i + 1], current_poly.points[j - 1]);

					if ((ac.coord[0] * ad.coord[1] - ad.coord[0] * ac.coord[1]) >= 0)
						signe1 = 1;
					else
						signe1 = 0;

					if ((bc.coord[0] * bd.coord[1] - bd.coord[0] * bc.coord[1]) >= 0)
						signe2 = 1;
					else
						signe2 = 0;

					if (signe1 != signe2)
					{
						nb_secant++;
					}
				}
			}
	}
	return nb_secant;
}

/*Fonction permettant de traiter les cas non évidents*/
int delit_cas(int i, int x, int y, int option, Poly working_poly)
{

	if (option)
	{
		//Poly working_poly = list_poly[which_poly];
		double a, b, _x;

		if (working_poly.points[i + 1].coord[1] == working_poly.points[i].coord[1])
			return 1;

		a = (double)(working_poly.points[i + 1].coord[1] - working_poly.points[i].coord[1]) / (double)(working_poly.points[i + 1].coord[0] - working_poly.points[i].coord[0]);
		b = (double)working_poly.points[i + 1].coord[1] - (working_poly.points[i + 1].coord[0] * a);

		_x = (y - b) / a;

		if (_x > x)
			return 1;
		else
			return 0;
	}

	else
	{
		if (transition[i + 1] == transition[i])
			return 0;
		else if ((transition[i - 1] == 1 && transition[i + 1] == 2) || (transition[i - 1] == 2 && transition[i + 1] == 1))
			return 1;
		else if ((transition[i - 1] == 4 && transition[i + 1] == 2) || (transition[i - 1] == 2 && transition[i + 1] == 4))
			return 1;
		else
			return 0;
	}
}

/*Are we in the windows?*/
int isOutsideWindow(int x, int y)
{
	int winW = glutGet(GLUT_WINDOW_WIDTH);
	int winH = glutGet(GLUT_WINDOW_HEIGHT);

	if (x > winW || x < 0)
		return 1;
	if (y > winH || y < 0)
		return 1;
	else
		return 0;
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
	int NI;
	int nb_points;
	int k;
	int it_is_inside;
	Poly temp_poly;
	Poly temp_hole;
	Point current_point;

	for (k = 0; k < nb_poly; k++)
	{
		temp_poly = list_poly[k];
		temp_hole = list_hole[k];

		nb_points = temp_poly.nb_point;
		NI = 0;
		it_is_inside = -1;
		//Duplicate the first point to the last one + 1 to close our polygone
		temp_poly.points[nb_points].coord[0] = temp_poly.points[0].coord[0];
		temp_poly.points[nb_points].coord[1] = temp_poly.points[0].coord[1];
		temp_poly.points[nb_points].coord[2] = 0;

		//Check where points are by the cursor
		for (i = 0; i < nb_points + 1; i++)
		{
			current_point = temp_poly.points[i];
			if (current_point.coord[0] >= x && current_point.coord[1] > y)
			{
				transition[i] = 1;
			}
			else if (current_point.coord[0] > x && current_point.coord[1] < y)
			{
				transition[i] = 2;
			}
			else if (current_point.coord[0] < x && current_point.coord[1] < y)
			{
				transition[i] = 3;
			}
			else if (current_point.coord[0] < x && current_point.coord[1] > y)
			{
				transition[i] = 4;
			}
			else if (current_point.coord[0] > x && current_point.coord[1] == y)
			{
				transition[i] = 5;
			}
		}

		//Analyse data previously earn
		for (i = 0; i < nb_points; i++)
		{
			if ((transition[i] == 1 && transition[i + 1] == 2) || (transition[i] == 2 && transition[i + 1] == 1))
				NI++;
			else if ((transition[i] == 2 && transition[i + 1] == 4) || (transition[i] == 4 && transition[i + 1] == 2))
				NI += delit_cas(i, x, y, 1, temp_poly);
			else if ((transition[i] == 1 && transition[i + 1] == 3) || (transition[i] == 3 && transition[i + 1] == 1))
				NI += delit_cas(i, x, y, 1, temp_poly);
			else if (transition[i] == 5)
				NI += delit_cas(i, x, y, 0, temp_poly);
		}
		if (NI % 2 != 0 && NI != 0)
		{
			it_is_inside = k;
		}

		//If we are in a Poly we need to check if we are in a hole too
		if (it_is_inside != -1 && temp_hole.nb_point > 2)
		{

			nb_points = temp_hole.nb_point;
			NI = 0;
			//Duplicate the first point to the last one + 1 to close our polygone
			temp_hole.points[nb_points].coord[0] = temp_hole.points[0].coord[0];
			temp_hole.points[nb_points].coord[1] = temp_hole.points[0].coord[1];
			temp_hole.points[nb_points].coord[2] = 0;

			//Check where hole's points are by the cursor
			for (i = 0; i < nb_points + 1; i++)
			{
				current_point = temp_hole.points[i];
				if (current_point.coord[0] >= x && current_point.coord[1] > y)
				{
					transition[i] = 1;
				}
				else if (current_point.coord[0] > x && current_point.coord[1] < y)
				{
					transition[i] = 2;
				}
				else if (current_point.coord[0] < x && current_point.coord[1] < y)
				{
					transition[i] = 3;
				}
				else if (current_point.coord[0] < x && current_point.coord[1] > y)
				{
					transition[i] = 4;
				}
				else if (current_point.coord[0] > x && current_point.coord[1] == y)
				{
					transition[i] = 5;
				}
			}

			//Analyse data previously earn
			for (i = 0; i < nb_points; i++)
			{
				if ((transition[i] == 1 && transition[i + 1] == 2) || (transition[i] == 2 && transition[i + 1] == 1))
					NI++;
				else if ((transition[i] == 2 && transition[i + 1] == 4) || (transition[i] == 4 && transition[i + 1] == 2))
					NI += delit_cas(i, x, y, 1, temp_hole);
				else if ((transition[i] == 1 && transition[i + 1] == 3) || (transition[i] == 3 && transition[i + 1] == 1))
					NI += delit_cas(i, x, y, 1, temp_hole);
				else if (transition[i] == 5)
					NI += delit_cas(i, x, y, 0, temp_hole);
			}
			// if we are in the poly's hole we are outside our poly so we return -1
			if (NI % 2 != 0 && NI != 0)
			{
				return -1;
			}
		}

		//if we are inside a Poly but in this poly there is no hole so we can return the poly's id
		if (it_is_inside != -1)
			return it_is_inside;
	}

	return -1;
}

//Which point is the closest to the mouse?
Info nearestPoint(int x, int y)
{
	int i, k;
	int x_left, x_right, y_left, y_right, x_point, y_point;
	int neighboor = 10;
	int nb_points;
	Poly working_poly;
	Info is_a_point;
	is_a_point.which_poly = -1;
	is_a_point.which_point = -1;
	is_a_point.is_a_hole = -1;

	x_left = x - neighboor;
	x_right = x + neighboor;

	y_left = y - neighboor;
	y_right = y + neighboor;

	for (k = 0; k < nb_poly; k++)
	{
		working_poly = list_poly[k];
		nb_points = working_poly.nb_point;

		//Check the surrounding looking for points
		for (i = 0; i < nb_points; i++)
		{
			x_point = working_poly.points[i].coord[0];
			y_point = working_poly.points[i].coord[1];
			if ((x_point < x_right && x_point > x_left) && (y_point < y_right && y_point > y_left))
			{
				is_a_point.which_poly = k;
				is_a_point.which_point = i;
				is_a_point.is_a_hole = 0;
				return is_a_point;
			}
		}
	}
	for (k = 0; k < nb_hole; k++)
	{
		working_poly = list_hole[k];
		nb_points = working_poly.nb_point;

		//Check the surrounding looking for points
		for (i = 0; i < nb_points; i++)
		{
			x_point = working_poly.points[i].coord[0];
			y_point = working_poly.points[i].coord[1];
			if ((x_point < x_right && x_point > x_left) && (y_point < y_right && y_point > y_left))
			{
				is_a_point.which_poly = k;
				is_a_point.which_point = i;
				is_a_point.is_a_hole = 1;
				return is_a_point;
			}
		}
	}
	//If is a point near we return the poly's and point's number or -1 and -1 if it's no point here
	return is_a_point;
}

//Is it a point by there?
int isAPointNear(int x, int y)
{
	int i, k;
	int x_left, x_right, y_left, y_right, x_point, y_point;
	int neighboor = 10;
	int nb_points;
	Poly working_poly;

	x_left = x - neighboor;
	x_right = x + neighboor;

	y_left = y - neighboor;
	y_right = y + neighboor;

	for (k = 0; k < nb_poly; k++)
	{
		working_poly = list_poly[k];
		nb_points = working_poly.nb_point;
		//Check the surrounding looking for points
		for (i = 0; i < nb_points; i++)
		{
			x_point = working_poly.points[i].coord[0];
			y_point = working_poly.points[i].coord[1];

			if ((x_point < x_right && x_point > x_left) && (y_point < y_right && y_point > y_left))
			{
				return 1;
			}
		}
	}
	for (k = 0; k < nb_hole; k++)
	{
		working_poly = list_hole[k];
		nb_points = working_poly.nb_point;

		//Check the surrounding looking for points
		for (i = 0; i < nb_points; i++)
		{
			x_point = working_poly.points[i].coord[0];
			y_point = working_poly.points[i].coord[1];
			if ((x_point < x_right && x_point > x_left) && (y_point < y_right && y_point > y_left))
			{
				return 1;
			}
		}
	}
	//If a point is near the mouse we return 1 else 0
	return 0;
}

/*Add a point in our poly*/
void addAPoint(int x, int y, int is_hole)
{
	int i;
	if (is_hole)
	{
		if (list_hole[hole_selected].nb_point < 3)
			list_hole[hole_selected].insert_here = list_hole[hole_selected].nb_point;

		if (list_hole[hole_selected].nb_point != list_hole[hole_selected].insert_here)

		{
			for (i = list_hole[hole_selected].nb_point; i > list_hole[hole_selected].insert_here; i--)
			{
				list_hole[hole_selected].points[i + 1] = list_hole[hole_selected].points[i];
			}
			list_hole[hole_selected].insert_here++;
		}

		list_hole[hole_selected].points[list_hole[hole_selected].insert_here].coord[0] = x;
		list_hole[hole_selected].points[list_hole[hole_selected].insert_here].coord[1] = y;
		list_hole[hole_selected].points[list_hole[hole_selected].insert_here].coord[2] = 0;
		list_hole[hole_selected].points[list_hole[hole_selected].insert_here].active = 0;
		list_hole[hole_selected].nb_point++;
	}
	else
	{
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
		list_poly[poly_selected].points[list_poly[poly_selected].insert_here].active = 0;
		list_poly[poly_selected].nb_point++;

		Yposition[nbYposition].which_point = y;
		Yposition[nbYposition].which_poly = list_poly[poly_selected].insert_here;
		nbYposition++;
	}
}

/*Fonction qui supprime un point donné*/
void deleteAPoint(Info point_to_delete)
{
	int i;
	if (!point_to_delete.is_a_hole)
	{
		for (i = point_to_delete.which_point; i < list_poly[point_to_delete.which_poly].nb_point; i++)
			list_poly[point_to_delete.which_poly].points[i] = list_poly[point_to_delete.which_poly].points[i + 1];

		if (list_poly[point_to_delete.which_poly].insert_here == list_poly[point_to_delete.which_poly].nb_point)
			list_poly[point_to_delete.which_poly].insert_here--;

		list_poly[point_to_delete.which_poly].nb_point--;
	}
	else
	{
		for (i = point_to_delete.which_point; i < list_hole[point_to_delete.which_poly].nb_point; i++)
			list_hole[point_to_delete.which_poly].points[i] = list_hole[point_to_delete.which_poly].points[i + 1];

		if (list_hole[point_to_delete.which_poly].insert_here == list_hole[point_to_delete.which_poly].nb_point)
			list_hole[point_to_delete.which_poly].insert_here--;

		list_hole[point_to_delete.which_poly].nb_point--;
	}
}

void initPoly(void)
{
	int x;
	list_poly = malloc((nb_poly_max + 1) * sizeof(Poly));
	list_hole = malloc((nb_poly_max + 1) * sizeof(Poly));
	for (x = 0; x < nb_poly_max; x++)
	{
		if (nb_poly < nb_poly_max)
		{
			list_poly[x].nb_point = 0;
			list_poly[x].insert_here = 0;
			list_poly[x].points = malloc(3 * (nb_point_max) + 1 * sizeof(Point));
			nb_poly++;
		}
	}
	for (x = 0; x < nb_hole_max; x++)
	{
		if (nb_hole < nb_hole_max)
		{
			list_hole[nb_hole].nb_point = 0;
			list_hole[nb_hole].insert_here = 0;
			list_hole[nb_hole].which_poly = -1;
			list_hole[nb_hole].points = malloc(3 * (nb_point_max) + 1 * sizeof(Point));
			nb_hole++;
		}
	}
}

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

void closeThePoly (int k)
{
	list_poly[k].points[list_poly[k].nb_point].coord[0] = list_poly[k].points[0].coord[0];
	list_poly[k].points[list_poly[k].nb_point].coord[1] = list_poly[k].points[0].coord[1];
	list_poly[k].points[list_poly[k].nb_point].coord[2] = 0;
}

void theTooSimpleDrawing (int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;

	float x = x1;
	float y = y1;

	glBegin(GL_POINTS);
	glVertex2i(x,y1);

	if (pow(dx, 2) > pow(dy, 2))
	{
		if (dx > 0)
		{
			float a = (float) dy / dx;
			while (x < x2)
			{
				x++;
				y += a;
				glVertex2i(x, (int)(y + 0.5));
			}
		}
		else
		{
			float a = (float)dy / dx;
			while (x > x2)
			{
				x--;
				y -= a;
				glVertex2i(x, (int)(y + 0.5));
			}
		}
	}
	else 
	{
		if (dy > 0)
		{
			float a = (float)dx / dy;
			while (y < y2)
			{
				y++;
				x += a;
				glVertex2i((int)(x + 0.5),y);
			}
		}
		else
		{
			float a = (float)dx / dy;
			while (y > y2)
			{
				y--;
				x -= a;
				glVertex2i((int)(x + 0.5), y);
			}
		}
	}
	glEnd();
}

void draw_bresenham (int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	

	int e = dx;

	dx *= 2;
	dy *= 2;
	if (x1 < x2)
	{
		if (pow(dx, 2) > pow(dy, 2))
		{
			if (y1 < y2)
			{
				glBegin(GL_POINTS);
				while (x1 < x2)
				{
					x1++;
					glVertex2i(x1,y1);
					e -= dy;
					if (e < 0)
					{
						y1++;
						e += dx;
					}
				}
				glEnd();
			}
			else
			{
				glBegin(GL_POINTS);
				while (x1 < x2)
				{
					x1++;
					glVertex2i(x1, y1);
					e += dy;
					if (e < 0)
					{
						y1--;
						e += dx;
					}
				}
				glEnd();
			}
		}
		else
		{
			if (y1 < y2)
			{
				e = dy;
				glBegin(GL_POINTS);
				while (y1 < y2)
				{
					y1++;
					glVertex2i(x1, y1);
					e -= dx;
					if (e < 0)
					{
						x1++;
						e += dy;
					}
				}
				glEnd();
			}
			else
			{
				e = dy;
				glBegin(GL_POINTS);
				while (y1 > y2)
				{
					y1--;
					glVertex2i(x1, y1);
					e += dx;
					if (e > 0)
					{
						x1++;
						e += dy;
					}
				}
				glEnd();
			}
		}
	}
	glutPostRedisplay();
}

void compute_bresenham (int x1, int y1, int x2, int y2,int i)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	
	nb_sample = 0;
	int e = dx;

	dx *= 2;
	dy *= 2;
	if (x1 < x2)
	{
		if (pow(dx, 2) > pow(dy, 2))
		{
			if (y1 < y2)
			{
				while (x1 < x2)
				{
					x1++;
					segment_drawing[i] = x1;
					i += 2;
					nb_sample++;
					e -= dy;
					if (e < 0)
					{
						y1++;
						e += dx;
					}
				}
			}
			else
			{
				while (x1 < x2)
				{
					x1++;
					segment_drawing[i] = x1;
					i += 2;
					nb_sample++;
					e += dy;
					if (e < 0)
					{
						y1--;
						e += dx;
					}
				}
			}
		}
		else
		{
			if (y1 < y2)
			{
				e = dy;
				while (y1 < y2)
				{
					y1++;
					nb_sample++;
					e -= dx;
					if (e < 0)
					{
						x1++;
						segment_drawing[i] = x1;
						i += 2;
						e += dy;
					}
				}
			}
			else
			{
				e = dy;
				while (y1 > y2)
				{
					y1--;
					e += dx;
					if (e > 0)
					{
						x1++;
						segment_drawing[i] = x1;
						i += 2;
						e += dy;
					}
				}
			}
		}
	}
	else
	{
		if (pow(dx, 2) > pow(dy, 2))
		{
			if (y1 < y2)
			{
				while (x1 > x2)
				{
					x1--;
					segment_drawing[i] = x1;
					i += 2;
					nb_sample++;
					e -= dy;
					if (e < 0)
					{
						y1++;
						e += dx;
					}
				}
			}
			else
			{
				while (x1 > x2)
				{
					x1--;
					segment_drawing[i] = x1;
					i += 2;
					nb_sample++;
					e += dy;
					if (e < 0)
					{
						y1--;
						e += dx;
					}
				}
			}
		}
		else
		{
			if (y1 < y2)
			{
				e = dy;
				while (y1 < y2)
				{
					y1++;
					nb_sample++;
					e -= dx;
					if (e < 0)
					{
						x1--;
						segment_drawing[i] = x1;
						i += 2;
						e += dy;
					}
				}
			}
			else
			{
				e = dy;
				while (y1 > y2)
				{
					y1--;
					e += dx;
					if (e > 0)
					{
						x1--;
						segment_drawing[i] = x1;
						i += 2;
						e += dy;
					}
				}
			}
		}
	}
}

void interpolation(int x1, int y1, int x2, int y2)
{
	int delta, inc1, inc2, x, y, tmp, tmp1, tmp2, pente;


	if (x1 > x2)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;

		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	/* Repere a 0*/
	tmp1 = x1;
	tmp2 = y1;

	x1 = 0;
	y1 = 0;

	x2 = x2 - tmp1;
	y2 = y2 - tmp2;

	x = x1;
	y = y1;

	/* calcul de la pente */

	if (x2 != 0)
		pente = (y2 - y1) / (x2 - x1);
	else
		pente = y2;

	delta = 2 * abs(y2) - 2 * (abs(pente) + 1) * x2;
	inc1 = 2 * abs(y2) - 2 * abs(pente) * x2;
	inc2 = delta;

	while (x < x2)
	{

		if (y2 < 0)
		{
			glBegin(GL_POINTS);
			glVertex3i((x + tmp1), abs(-y + tmp2), 0);
			glEnd();
		}
		else
		{
			glVertex3i(x + tmp1, y + tmp2, 0);
			glEnd();
		}

		if (delta < 0)
		{
			delta += inc1;
			y += abs(pente);
		}
		else
		{
			delta += inc2;
			y += abs(pente) + 1;
		}
		x++;
	}
	glPointSize(3);
}

void shakesort(int iMax)
{
	int i, k, temp, z = 1;
	int flag = 0;
	for (k = iMax - 1; k >= 0; k--)
	{
		for (i = 1; i <= k; i++)
		{
			//passage vers le haut
			if (Yposition[i].which_point < Yposition[i + 1].which_point)
			{
				temp = Yposition[i].which_point;
				Yposition[i].which_point = Yposition[i + 1].which_point;
				Yposition[i + 1].which_point = temp;

				temp = Yposition[i].which_poly;
				Yposition[i].which_poly = Yposition[i + 1].which_poly;
				Yposition[i + 1].which_poly = temp;
				flag = 1;
			}
		}
		for (i = k; i >= z; i--)
		{
			//Passage vers le bas
			if (Yposition[i].which_point > Yposition[i - 1].which_point)
			{
				temp = Yposition[i].which_point;
				Yposition[i].which_point = Yposition[i - 1].which_point;
				Yposition[i - 1].which_point = temp;

				temp = Yposition[i].which_poly;
				Yposition[i].which_poly = Yposition[i - 1].which_poly;
				Yposition[i - 1].which_poly = temp;
				flag = 1;
			}
		}
		z++;
		if (flag == 0)
			break;
		flag = 0;
	}
}

void remplissage()
{
	/*if (list_poly[0].nb_point > 2)
	{
		closeThePoly (0);
		//Copie des Ordonnées dans Yposition
		for (int i ; i <= list_poly[0].nb_point ; i++)
		{
			Yposition[i].which_point = list_poly[0].points[i].coord[1];
			Yposition[i].which_poly = i;
		}
		shakesort(list_poly[0].nb_point+1);
		int y;
		int ind = 0;


		for (int i = 0; i < list_poly[0].nb_point; i++)
		{
			compute_bresenham(list_poly[0].points[Yposition[i].which_poly].coord[0],
												list_poly[0].points[Yposition[i].which_poly].coord[1],
												list_poly[0].points[Yposition[i].which_poly - 1].coord[0],
												list_poly[0].points[Yposition[i].which_poly - 1].coord[1], 0);

			compute_bresenham(list_poly[0].points[Yposition[i].which_poly].coord[0],
												list_poly[0].points[Yposition[i].which_poly].coord[1],
												list_poly[0].points[Yposition[i].which_poly + 1].coord[0],
												list_poly[0].points[Yposition[i].which_poly + 1].coord[1], 1);
			//printf("%d\n",segment_drawing[1]);

				y = Yposition[i].which_point;
				while (y > Yposition[i + 1].which_point)
				{
					y--;
					glBegin(GL_LINES);
					glVertex2f(segment_drawing[ind], y);
					printf("%d----%d----%d----%d\n", segment_drawing[ind], segment_drawing[ind + 1], y, Yposition[i + 1].which_point);
					glVertex2f(segment_drawing[ind + 1], y);
					glEnd();
					ind++;
				}
			ind = 0;
			//}
		}
	}*/

	if (list_poly[0].nb_point > 2)
	{
		closeThePoly(0);
		//Copie des Ordonnées dans Yposition
		for (int i = 0; i <= list_poly[0].nb_point; i++)
		{
			Yposition[i].which_point = list_poly[0].points[i].coord[1];
			Yposition[i].which_poly = i;
		}
		shakesort(list_poly[0].nb_point + 1);

		int etape = 0;
		int ind;

		for(int y = winY ; y > 0 ; y--)
		{
			if (y < Yposition[etape].which_point && etape < (list_poly[0].nb_point -1) )
			{
			printf("%d --- %d --- %d\n", y, Yposition[etape].which_point, Yposition[etape+1].which_point);
				
				compute_bresenham(list_poly[0].points[Yposition[etape].which_poly].coord[0],
													list_poly[0].points[Yposition[etape].which_poly].coord[1],
													list_poly[0].points[Yposition[etape].which_poly - 1].coord[0],
													list_poly[0].points[Yposition[etape].which_poly - 1].coord[1], 0);

				compute_bresenham(list_poly[0].points[Yposition[etape].which_poly].coord[0],
													list_poly[0].points[Yposition[etape].which_poly].coord[1],
													list_poly[0].points[Yposition[etape].which_poly + 1].coord[0],
													list_poly[0].points[Yposition[etape].which_poly + 1].coord[1], 1);
													
				while (y > Yposition[etape+1].which_point)
				{
					y--;
					glBegin(GL_LINES);
					glVertex2f(segment_drawing[ind], y);
					printf("%d----%d----%d----%d\n", segment_drawing[ind], segment_drawing[ind + 1], y, Yposition[etape + 1].which_point);
					glVertex2f(segment_drawing[ind + 1], y);
					glEnd();
					ind++;
				}
				ind = 0;
				etape++;
			}
		}
	}
}

/* Fonction pour afficher du texte en OpenGL */
void glPrintText(int x, int y, const char *text)
{
	int i;

	glRasterPos2i(x, y);

	for (i = 0; i < (int)strlen(text); ++i)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
}

		/* Callback OpenGL */
		void displayGL()
		{
			int i;
			int k;
			glClear(GL_COLOR_BUFFER_BIT);

			for (k = 0; k < nb_poly; k++)
			{
				closeThePoly(k);
				for (i = 0; i < list_poly[k].nb_point; i++)
				{
					glPointSize(5);
					glColor3f(0.4, 0.5, 0.7);
					glBegin(GL_POINTS);
					glVertex3f(list_poly[k].points[i].coord[0], list_poly[k].points[i].coord[1],0);
					glVertex3f(list_poly[k].points[i+1].coord[0], list_poly[k].points[i+1].coord[1],0);
				glEnd();
			if (list_poly[k].nb_point > 1)
			{
				glPointSize(1);
  			glColor3f(0.9, 0.2, 0.2);
				switch (display_mode)
				{
					case 1 :
						draw_bresenham(list_poly[k].points[i].coord[0], list_poly[k].points[i].coord[1], list_poly[k].points[i + 1].coord[0], list_poly[k].points[i + 1].coord[1]);
						break;
					case 2 : 
						theTooSimpleDrawing(list_poly[k].points[i].coord[0], list_poly[k].points[i].coord[1], list_poly[k].points[i + 1].coord[0], list_poly[k].points[i + 1].coord[1]);
						break;
					case 3 : 
						interpolation(list_poly[k].points[i].coord[0], list_poly[k].points[i].coord[1], list_poly[k].points[i + 1].coord[0], list_poly[k].points[i + 1].coord[1]);
						break;
				}
			}
		}
	}
	remplissage();
	glutSwapBuffers();
}

void passivemouseGL (int x, int y)
{ 
  y = winY-y;
  if(isInside(x, y) != -1)
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
	button_pressed = button;
	button_state = state;
  if (state ==  GLUT_DOWN && button == GLUT_LEFT_BUTTON)
  {
		if (!isAPointNear(x, y))
		{
			motion.which_poly = -1;
      if (edit_mode && !add_hole_mode){
          addAPoint(x,y,0);
        }
        if (edit_mode && add_hole_mode)
        {
					if (isInside(x, y) != -1){
            hole_selected = isInside(x, y);
              addAPoint(x, y, 1);
          }
				}
			}
      else{
        motion = nearestPoint(x,y);
      }
	}
  if (state ==  GLUT_DOWN && button == GLUT_RIGHT_BUTTON){
    motion = nearestPoint(x, y);
		if (motion.which_point != -1 && edit_mode)
		{
      deleteAPoint(nearestPoint(x,y));
    }
  }
  if (state ==  GLUT_DOWN && button == GLUT_MIDDLE_BUTTON){
    Info is_a_point_there = nearestPoint(x, y);
		if (is_a_point_there.which_poly != -1 && edit_mode)
		{
      if (is_a_point_there.is_a_hole)
        list_hole[is_a_point_there.which_poly].insert_here = is_a_point_there.which_point;
      else
        list_poly[is_a_point_there.which_poly].insert_here = is_a_point_there.which_point;
    }
  }
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		motionX.which_point = -1;
		motionX.which_poly = -1;
		motionY.which_poly = -1;
		motionY.which_point = -1;
	}
  glutPostRedisplay();
}


/* Callback OpenGL de gestion de drag */
void motionGL(int x, int y)
{
	y = winY - y;
  int i;
	int outside = 0;
  

	if(button_pressed == GLUT_LEFT_BUTTON && motion.which_poly != -1 && edit_mode)
	{
		if (motion.is_a_hole && !isOutsideWindow(x, y))
		{
			list_hole[motion.which_poly].points[motion.which_point].coord[0] = x;
			list_hole[motion.which_poly].points[motion.which_point].coord[1] = y;
		}
		else if (!motion.is_a_hole && !isOutsideWindow(x, y))
		{
      list_poly[motion.which_poly].points[motion.which_point].coord[0] = x;
      list_poly[motion.which_poly].points[motion.which_point].coord[1] = y;
    }
	}
	
	if (button_pressed == GLUT_LEFT_BUTTON && isInside(x, y) == -1 && !edit_mode)
	{
		motionX.which_poly = -2;
	}

	if (button_pressed == GLUT_LEFT_BUTTON && isInside(x, y) != -1 &&!edit_mode && motionX.which_poly != -2)
	{
		if (motionX.which_poly == isInside(x, y) )
		{
			for (i = 0; i < list_poly[motionX.which_poly].nb_point; i++)
				{
					if (isOutsideWindow(list_poly[motionX.which_poly].points[i].coord[0] - (motionX.which_point - x), list_poly[motionY.which_poly].points[i].coord[1] - (motionY.which_point - y)))
					{
						outside = 1;
					}
				}
			for (i = 0; i < list_poly[motionX.which_poly].nb_point; i++)
				{
					if (!outside)
					{
						list_poly[motionX.which_poly].points[i].coord[0] = list_poly[motionX.which_poly].points[i].coord[0] - (motionX.which_point - x);
						list_poly[motionY.which_poly].points[i].coord[1] = list_poly[motionY.which_poly].points[i].coord[1] - (motionY.which_point - y);
					}
				}
			for (i = 0; i < list_hole[motionX.which_poly].nb_point; i++)
			{
				if (!outside)
				{
					list_hole[motionX.which_poly].points[i].coord[0] = list_hole[motionX.which_poly].points[i].coord[0] - (motionX.which_point - x);
					list_hole[motionY.which_poly].points[i].coord[1] = list_hole[motionY.which_poly].points[i].coord[1] - (motionY.which_point - y);
				}
			}
		}
			motionX.which_point = x;
			motionY.which_point = y;
			motionX.which_poly = isInside(x, y);
			motionY.which_poly = isInside(x, y);
	}

		glutPostRedisplay();
}

void usage()
{
	printf("---------------------------------------------------------------------------\n");
	printf("%c[%dm", 27, 1);
	printf("[Utilisation]\n");
	printf("%c[%dm", 27, 0);
	printf("\n\n");
	printf("    Clic gauche   :\t ajouter/deplacer un point / deplacer un polygone\n");
	printf("    Clic droit    :\t supprimer un point\n");
	printf("    Clic milieu   :\t definir un nouvel indice d'insertion\n");
	printf("    1-4				    :\t naviguer entre les differents affichage\n");
	printf("    'e'           :\t supprimer tous les points\n");
	printf("    'tab'         :\t passer au polygone suivant\n");
	printf("    'm'           :\t activer/desactive le mode d'edition\n");
	printf("    'c'           :\t afficher/masquer les coordonnees des points de controle\n");
	printf("    'h'           :\t travailler sur les trous\n");
	printf("    'q'/Esc       :\t quitter\n");
	printf("---------------------------------------------------------------------------\n");
}

/*Fonction d'initialisation de nos variables*/
void init()
{
  nb_poly					= 0;
	max_sample			= 50000;
  nb_hole					= 0;
  nb_poly_max			= 5;
  nb_hole_max			= 5;
  motion_hole     = -1;
  poly_selected   = 0;
  hole_selected   = 0;
	nbYposition 		= 0;
	motionX.which_point = -1;
	motionX.which_poly  =	-2;
	motionY.which_poly  =	-1;
	motionY.which_point =	-1;
  button_pressed  = 0;
	button_state		= 0;
  edit_mode       = 1;
	display_mode 		= GL_LINE_LOOP;
	add_hole_mode   = 0;
  coord_mode      = 0;
  mouse_is_inside = 0;
  nb_point_max    = 100;
  int_ext         = "Exterieur";
  nb_point_hole_max	= 5;
  motion.which_point = -1;
  motion.which_poly = -1;
  motion.is_a_hole = -1;

  initPoly();
  transition = malloc( (nb_point_max + 1) * sizeof(int) );
	Yposition = malloc((nb_point_max) * sizeof(Info));
	segment_drawing = malloc ((2*max_sample)*sizeof(int));
	usage();
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

		case '1':
			display_mode = 1;
			break;

		case '2':
			display_mode = 2;
			break;

		case '3':
			display_mode = 3;
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
