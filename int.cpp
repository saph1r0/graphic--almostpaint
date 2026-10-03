
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
 
// Arista usada por el relleno scan-line (Edge Table / Edge Active Table).
struct Arista{
	int   ymin;
	int   ymax;
	float x;
	float invM;   // 1/pendiente (dx/dy)
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

// --- FUNCIONES AUXILIARES DE MATRICES Y CENTROIDE ---
Punto obtenerCentro(Poligono &pol) {
    float sumX = 0, sumY = 0;
    int n = pol.P.size();
    if (n == 0) return {0, 0};
    for (auto &p : pol.P) {
        sumX += p.x;
        sumY += p.y;
    }
    return {sumX / n, sumY / n};
}

vector<vector<float>> matrizTraslacion(float tx, float ty) {
    return {
        {1, 0, tx},
        {0, 1, ty},
        {0, 0, 1}
    };
}

vector<vector<float>> matrizRotacion(float angulo) {
    float rad = angulo * M_PI / 180.0;
    float cosA = cos(rad);
    float sinA = sin(rad);
    return {
        {cosA, -sinA, 0},
        {sinA,  cosA, 0},
        {   0,     0, 1}
    };
}

vector<vector<float>> matrizEscala(float sx, float sy) {
    return {
        {sx,  0, 0},
        { 0, sy, 0},
        { 0,  0, 1}
    };
}

vector<vector<float>> multiplicarMatrices(const vector<vector<float>> &A, const vector<vector<float>> &B) {
    vector<vector<float>> C(3, vector<float>(3, 0));
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

void rotar(Poligono &pol, float angulo) {
    Punto c = obtenerCentro(pol);
    vector<vector<float>> T_pos = matrizTraslacion(c.x, c.y);
    vector<vector<float>> R = matrizRotacion(angulo);
    vector<vector<float>> T_neg = matrizTraslacion(-c.x, -c.y);
    vector<vector<float>> M = multiplicarMatrices(T_pos, multiplicarMatrices(R, T_neg));

    for (auto &p : pol.P) {
        float x_new = M[0][0] * p.x + M[0][1] * p.y + M[0][2];
        float y_new = M[1][0] * p.x + M[1][1] * p.y + M[1][2];
        p.x = x_new; p.y = y_new;
    }
	cout << "Poligono rotado " << angulo << " grados respecto a su centro." << endl;
}

void escalar(Poligono &pol, float escala) {
    Punto c = obtenerCentro(pol);
    vector<vector<float>> T_pos = matrizTraslacion(c.x, c.y);
    vector<vector<float>> S = matrizEscala(escala, escala);
    vector<vector<float>> T_neg = matrizTraslacion(-c.x, -c.y);
    vector<vector<float>> M = multiplicarMatrices(T_pos, multiplicarMatrices(S, T_neg));

    for (auto &p : pol.P) {
        float x_new = M[0][0] * p.x + M[0][1] * p.y + M[0][2];
        float y_new = M[1][0] * p.x + M[1][1] * p.y + M[1][2];
        p.x = x_new; p.y = y_new;
    }
	cout << "Poligono escalado por un factor de " << escala << "." << endl;
}

// Trasladar con matriz homogenea T(tx, ty) 
void trasladar(Poligono &pol, float tx, float ty) {
    vector<vector<float>> T = matrizTraslacion(tx, ty);
    for (auto &p : pol.P) {
        float x_new = T[0][0] * p.x + T[0][1] * p.y + T[0][2];
        float y_new = T[1][0] * p.x + T[1][1] * p.y + T[1][2];
        p.x = x_new; p.y = y_new;
    }
	cout << "Poligono trasladado (dx: " << tx << ", dy: " << ty << ")." << endl;
}


// --- RELLENO SCAN-LINE (ET -> EAT), portado de Relleno.cpp ---
// Construye la Edge Table: para cada scan-line guarda las aristas que
// comienzan en ella (ymin). Las aristas horizontales no se agregan.
vector<vector<Arista>> construirET(const Poligono &pol){
	int n = pol.P.size();
	vector<vector<Arista>> ET(ALTO);
	for (int i = 0; i < n; i++){
		Punto P1 = pol.P[i];
		Punto P2 = pol.P[(i + 1) % n];   // cierra el poligono

		int y1 = redondear(P1.y);
		int y2 = redondear(P2.y);
		if (y1 == y2) continue;          // arista horizontal: se ignora

		Arista A;
		if (y1 < y2){
			A.ymin = y1; A.ymax = y2; A.x = P1.x;
			A.invM = (P2.x - P1.x) / (float)(y2 - y1);
		}
		else{
			A.ymin = y2; A.ymax = y1; A.x = P2.x;
			A.invM = (P1.x - P2.x) / (float)(y1 - y2);
		}

		// Las transformaciones pueden sacar vertices de la pantalla;
		// ET esta indexada por scan-line, asi que recortamos.
		if (A.ymax < 0 || A.ymin > ALTO - 1) continue;
		if (A.ymin < 0){ A.x += A.invM * (0 - A.ymin); A.ymin = 0; }
		if (A.ymax > ALTO) A.ymax = ALTO;
		ET[A.ymin].push_back(A);
	}
	return ET;
}

void rellenarPoligono(const Poligono &pol)
{
	if (pol.P.size() < 3) return;

	// 1) Edge Table agrupada por scan-line
	vector<vector<Arista>> ET = construirET(pol);
	// 2) Edge Active Table: aristas que cruzan la scan-line actual
	vector<Arista> EAT;

	int ymin = ALTO, ymax = 0;
	for (auto &p : pol.P){
		ymin = min(ymin, redondear(p.y));
		ymax = max(ymax, redondear(p.y));
	}
	ymin = max(ymin, 0);
	ymax = min(ymax, ALTO - 1);

	glPointSize(1.0f);
	glColor3f(pol.color.R, pol.color.G, pol.color.B);
	glBegin(GL_POINTS);
	for (int y = ymin; y < ymax; y++){
		// Agregar a EAT las aristas que comienzan en y
		for (int i = 0; i < (int)ET[y].size(); i++)
			EAT.push_back(ET[y][i]);

		// Eliminar aristas cuyo ymax es y
		EAT.erase(remove_if(EAT.begin(), EAT.end(),
		                    [y](const Arista &A){ return A.ymax == y; }),
		          EAT.end());

		// Ordenar EAT por interseccion x
		sort(EAT.begin(), EAT.end(),
		     [](const Arista &A, const Arista &B){ return A.x < B.x; });

		// Pintar entre pares de intersecciones (paridad)
		for (int i = 0; i + 1 < (int)EAT.size(); i += 2){
			int x1 = (int)ceil(EAT[i].x);
			int x2 = (int)floor(EAT[i + 1].x);
			if (x1 < 0)        x1 = 0;
			if (x2 > ANCHO - 1) x2 = ANCHO - 1;
			for (int x = x1; x <= x2; x++)
				glVertex2i(x, y);
		}

		// Actualizar x para la siguiente scan-line
		for (int i = 0; i < (int)EAT.size(); i++)
			EAT[i].x += EAT[i].invM;
	}
	glEnd();
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