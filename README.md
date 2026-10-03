# Práctica Integradora – Computación Gráfica 

Aplicación en C++ con OpenGL/GLUT que integra rasterización con Bresenham,
transformaciones con coordenadas homogéneas y relleno Scan-Line con ET y EAT.

## Compilar

Linux: `g++ main.cpp -o main -lGL -lGLU -lglut`

Windows (Dev-C++ / Code::Blocks): enlazar con `freeglut`, `opengl32` y `glu32`.

## Controles

| Control | Acción |
|---|---|
| Clic izquierdo | Agregar vértice |
| Clic izquierdo dentro de un polígono | Seleccionarlo como activo (si no se está construyendo otro) |
| Clic derecho | Cerrar polígono |
| 1..9 / TAB | Seleccionar polígono |
| D / I | Rotar derecha / izquierda |
| S / s | Aumentar / disminuir escala |
| Flechas | Trasladar |
| r / g / b | Elegir color |
| P | Rellenar polígono activo |
| C | Limpiar |
| ESC | Salir |

## División del trabajo

| Integrante | Parte | Estado |
|---|---|---|
| Integrante 1 | Estructuras, Bresenham, contorno, mouse, selección por clic 
| Integrante 2 | Transformaciones homogéneas (rotar, escalar, trasladar) | Pendiente |
| Integrante 3 | Relleno Scan-Line con ET y EAT, color por polígono | ⏳ Pendiente |

Cada integrante completa **solo el cuerpo** de sus funciones (marcadas con `TODO`).
Las llamadas desde `teclado()` y `display()` ya están conectadas.