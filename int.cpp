
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <string>
 
using namespace std;
 
const int   ANCHO = 800;
const int   ALTO  = 600;
const float PI    = 3.14159265f;
const float PASO_TRASLACION = 10.0f;
 
struct Punto
{
	float x;      // float: las rotaciones/escalas acumuladas no pierden
	float y;      // precision; se redondea solo al momento de rasterizar
};
 
struct Color
{
	float R, G, B;
};
 
// Cada poligono guarda sus vertices, si esta cerrado,
// si debe rellenarse y SU PROPIO color de relleno.
struct Poligono
{
	vector<Punto> P;
	bool  cerrado = false;
	bool  relleno = false;                // lo usa el Integrante 3
	Color color   = {1.0f, 0.0f, 0.0f};  // lo usa el Integrante 3
};
 
vector<Poligono> poligonos;
int   poligonoActual = 0;     // poligono que se esta CONSTRUYENDO
int   poligonoActivo = -1;    // poligono SELECCIONADO (-1 = ninguno)
Punto Pmouse = {0, 0};
Color colorSeleccionado = {1.0f, 0.0f, 0.0f};
 
int redondear(float v) { return (int)lround(v); }
 
bool hayActivo()
{
	return poligonoActivo >= 0 &&
	       poligonoActivo < (int)poligonos.size() &&
	       poligonos[poligonoActivo].cerrado;
}
 
void actualizarTitulo()
{
	string t = "Practica Integradora | Activo: ";
	t += hayActivo() ? to_string(poligonoActivo + 1) : "ninguno";
	t += " | Color: ";
	if (colorSeleccionado.R == 1) t += "rojo";
	else if (colorSeleccionado.G == 1) t += "verde";
	else t += "azul";
	glutSetWindowTitle(t.c_str());
}
 

void inicializar()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, ANCHO, 0, ALTO);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 
	poligonos.push_back(Poligono());
}
 
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(ANCHO, ALTO);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Practica Integradora");
 
	inicializar();
	actualizarTitulo();
 
	
	return 0;
}
