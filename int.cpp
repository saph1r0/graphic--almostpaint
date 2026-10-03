
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
 
struct Punto{
	float x;      // float: las rotaciones/escalas acumuladas no pierden
	float y;      // precision; se redondea solo al momento de rasterizar
};
 
struct Color{
	float R, G, B;
};
 
// Cada poligono guarda sus vertices, si esta cerrado,
// si debe rellenarse y SU PROPIO color de relleno.
struct Poligono{
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
 
bool hayActivo(){
	return poligonoActivo >= 0 &&
	       poligonoActivo < (int)poligonos.size() &&
	       poligonos[poligonoActivo].cerrado;
}
 
void actualizarTitulo(){
	string t = "Practica Integradora | Activo: ";
	t += hayActivo() ? to_string(poligonoActivo + 1) : "ninguno";
	t += " | Color: ";
	if (colorSeleccionado.R == 1) t += "rojo";
	else if (colorSeleccionado.G == 1) t += "verde";
	else t += "azul";
	glutSetWindowTitle(t.c_str());
}

//. soph1|
//  Bresenham, contorno, interaccion con el mouse y seleccion por clic

void Bresenham(int x1, int y1, int x2, int y2){
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
 
	int p = dx - dy;
	while (true){
		glVertex2i(x1, y1);            // se pinta el pixel actual
		if (x1 == x2 && y1 == y2)
			break;
		int p2 = 2 * p;
		if (p2 > -dy) { p -= dy; x1 += sx; }
		if (p2 <  dx) { p += dx; y1 += sy; }
	}
}
 
// Cada lado PiPi+1 es un segmento rasterizado con Bresenham.
// Si el poligono esta cerrado se agrega la arista Pn -> P1.
void dibujarContorno(const Poligono &pol){
	int n = pol.P.size();
	if (n < 2) return;
	glBegin(GL_POINTS);
	for (int i = 0; i < n - 1; i++)
		Bresenham(redondear(pol.P[i].x),     redondear(pol.P[i].y),
		          redondear(pol.P[i + 1].x), redondear(pol.P[i + 1].y));
	if (pol.cerrado && n >= 3)
		Bresenham(redondear(pol.P[n - 1].x), redondear(pol.P[n - 1].y),
		          redondear(pol.P[0].x),     redondear(pol.P[0].y));
	glEnd();
}
// Prueba punto-en-poligono (ray casting / paridad):
// se lanza un rayo horizontal hacia la derecha desde (px,py) y se
// cuentan los cruces con las aristas. Impar = dentro, par = fuera.
bool puntoDentro(const Poligono &pol, float px, float py){
	bool dentro = false;
	int n = pol.P.size();
	for (int i = 0, j = n - 1; i < n; j = i++){
		const Punto &A = pol.P[i];
		const Punto &B = pol.P[j];
 
		if ((A.y > py) != (B.y > py)){
			float xCruce = A.x + (py - A.y) * (B.x - A.x) / (B.y - A.y);
			if (px < xCruce)
				dentro = !dentro;
		}
	}
	return dentro;
}

void mouse(int button, int state, int x, int y){
	if (state != GLUT_DOWN)
		return;
	y = ALTO - y;   // GLUT tiene el origen arriba, OpenGL abajo!!!!!
 
	if (button == GLUT_LEFT_BUTTON){
		Poligono &pol = poligonos[poligonoActual];
		// Si NO se esta construyendo un poligono, un clic dentro de
		// un poligono cerrado lo selecciona (se revisa del ultimo
		// al primero para elegir el que esta dibujado encima).
		if (pol.P.empty()){
			for (int i = (int)poligonos.size() - 1; i >= 0; i--){
				if (poligonos[i].cerrado && puntoDentro(poligonos[i], x, y)){
					poligonoActivo = i;
					cout << "Poligono " << i + 1
					     << " seleccionado con el mouse." << endl;
					actualizarTitulo();
					glutPostRedisplay();
					return;
				}
			}
		}
 
		Punto Pi = {(float)x, (float)y};
		pol.P.push_back(Pi);
 
		cout << "Poligono " << poligonoActual + 1 << " - P" << pol.P.size()
		     << " = (" << x << ", " << y << ")" << endl;
	}
 
	if (button == GLUT_RIGHT_BUTTON){
		Poligono &pol = poligonos[poligonoActual];
 
		if (pol.P.size() < 3){
			cout << "Se necesitan al menos 3 vertices." << endl;
			return;
		}
 
		pol.cerrado    = true;
		poligonoActivo = poligonoActual;   // el recien cerrado queda activo
 
		cout << "Poligono " << poligonoActual + 1 << " cerrado con "
		     << pol.P.size() << " vertices." << endl;
 
		poligonos.push_back(Poligono());   // espacio para el siguiente
		poligonoActual = poligonos.size() - 1;
		actualizarTitulo();
	}
	glutPostRedisplay();
}
 
void movimiento(int x, int y){
	Pmouse.x = x;
	Pmouse.y = ALTO - y;
	glutPostRedisplay();
}

// TOOOOODO I22222
// Rotar 'angulo' grados respecto al centro: M = T(C) * R * T(-C)
void rotar(Poligono &pol, float angulo)
{
	// TODO Integrante 2
	(void)pol; (void)angulo;
	cout << "[Pendiente] rotar() - Integrante 2" << endl;
}

// Escalar respecto al centro: M = T(C) * S * T(-C)
void escalar(Poligono &pol, float escala)
{
	// TODO Integrante 2
	(void)pol; (void)escala;
	cout << "[Pendiente] escalar() - Integrante 2" << endl;
}

// Trasladar con matriz homogenea T(tx, ty) 
void trasladar(Poligono &pol, float tx, float ty)
{
	// TODO Integrante 2
	(void)pol; (void)tx; (void)ty;
	cout << "[Pendiente] trasladar() - Integrante 2" << endl;
}


///iiiii3
void rellenarPoligono(const Poligono &pol)
{
	// TODO Integrante 3
	(void)pol;
}








//integracion

void display(){
	glClear(GL_COLOR_BUFFER_BIT);
	// 1) Rellenos (cada uno con su propio color)  -> I3
	for (int i = 0; i < (int)poligonos.size(); i++)
		if (poligonos[i].cerrado && poligonos[i].relleno)
			rellenarPoligono(poligonos[i]);
	// 2) Contornos con Bresenham (activo en naranja y mas grueso)
	for (int i = 0; i < (int)poligonos.size(); i++){
		if (!poligonos[i].cerrado) continue;
		if (i == poligonoActivo){
			glPointSize(3.0f);
			glColor3f(1.0f, 0.55f, 0.0f);
		}
		else{
			glPointSize(1.0f);
			glColor3f(0.0f, 0.0f, 0.0f);
		}
		dibujarContorno(poligonos[i]);
	}
	// 3) Poligono en construccion + linea de previsualizacion
	Poligono &pol = poligonos[poligonoActual];
	if (!pol.cerrado && !pol.P.empty())
	{
		glPointSize(1.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		dibujarContorno(pol);

		int n = pol.P.size();
		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_POINTS);
		Bresenham(redondear(pol.P[n - 1].x), redondear(pol.P[n - 1].y),
		          redondear(Pmouse.x), redondear(Pmouse.y));
		glEnd();
	}

	glutSwapBuffers();
}

void teclado(unsigned char tecla, int, int){
	// ---- Seleccion por numero ----
	if (tecla >= '1' && tecla <= '9'){
		int k = tecla - '1';
		if (k < (int)poligonos.size() && poligonos[k].cerrado){
			poligonoActivo = k;
			cout << "Poligono " << k + 1 << " activo" << endl;
		}
	}
	// ---- TAB: siguiente poligono cerrado ----
	if (tecla == 9 && !poligonos.empty()){
		int n = poligonos.size();
		for (int paso = 1; paso <= n; paso++){
			int k = (max(poligonoActivo, 0) + paso) % n;
			if (poligonos[k].cerrado){
				poligonoActivo = k;
				cout << "Poligono " << k + 1 << " activo" << endl;
				break;
			}
		}
	}
	// ---- Transformaciones (I2) y relleno (I3) ----
	if (hayActivo()){
		Poligono &act = poligonos[poligonoActivo];

		if (tecla == 'd' || tecla == 'D') rotar(act, -5.0f);
		if (tecla == 'i' || tecla == 'I') rotar(act,  5.0f);
		if (tecla == 'S')                 escalar(act, 1.10f);
		if (tecla == 's')                 escalar(act, 0.90f);

		if (tecla == 'p' || tecla == 'P'){
			act.color   = colorSeleccionado;
			act.relleno = true;
			cout << "Poligono " << poligonoActivo + 1 << " marcado para relleno." << endl;
		}
	}
	// ---- Seleccion de color ----
	if (tecla == 'r') colorSeleccionado = {1.0f, 0.0f, 0.0f};
	if (tecla == 'g') colorSeleccionado = {0.0f, 1.0f, 0.0f};
	if (tecla == 'b') colorSeleccionado = {0.0f, 0.0f, 1.0f};
	// ---- Limpiar ----
	if (tecla == 'c' || tecla == 'C'){
		poligonos.clear();
		poligonos.push_back(Poligono());
		poligonoActual = 0;
		poligonoActivo = -1;
		cout << "Pantalla limpiada." << endl;
	}
	if (tecla == 27)
		exit(0);

	actualizarTitulo();
	glutPostRedisplay();
}

// Flechas -> traslacion del poligono activo (I2)
void teclasEspeciales(int tecla, int, int){
	if (!hayActivo()) return;
	Poligono &act = poligonos[poligonoActivo];
	switch (tecla){
	case GLUT_KEY_LEFT:  trasladar(act, -PASO_TRASLACION, 0); break;
	case GLUT_KEY_RIGHT: trasladar(act,  PASO_TRASLACION, 0); break;
	case GLUT_KEY_UP:    trasladar(act, 0,  PASO_TRASLACION); break;
	case GLUT_KEY_DOWN:  trasladar(act, 0, -PASO_TRASLACION); break;
	}

	glutPostRedisplay();
}

void inicializar(){
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, ANCHO, 0, ALTO);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	poligonos.push_back(Poligono());
}

int main(int argc, char **argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(ANCHO, ALTO);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("2integracion");

	inicializar();
	actualizarTitulo();
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(movimiento);
	glutKeyboardFunc(teclado);
	glutSpecialFunc(teclasEspeciales);
	glutMainLoop();
	return 0;
}