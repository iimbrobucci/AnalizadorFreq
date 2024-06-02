#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <complex.h>

void DibujarFrecuencias();

double amp(double complex z);

void DibujarVentana();

void DibujarMuestras();

void ActualizarCoordenadas();

void ChequearBotones();

void ChequearTeclado();

void DibujarBotones();

void CargarAudio();

void PrepararParaNuevoAudio();

double calcularEnergiaFrecuencia(const double complex* coeficientes, int num_coeficientes);

#endif
