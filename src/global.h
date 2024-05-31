#ifndef GLOBAL_H
#define GLOBAL_H

#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define N 8192

typedef struct{
  char *ruta;
  char *nombreAudio;
  char *tipo;
  int channels;
  int format;
  int sampleRate;
  unsigned int totalFrames;
  float volumen;
  float tono;
  float duracion;
  float tiempo;
  int printInfo;
  int pausado;
  int spectogram;
  FILE *fileF;
}sonido;

typedef struct{
  int alto;
  int ancho;

}Ventana;


typedef struct{
  int inicio;
  int fin;
  double suavidad;
  float dft;
}VisualizacionFreq;

typedef struct{
  int recortando;
  double *muestras;
  int size;
  int version;
}recorte;

typedef struct{
  int recortar;
  unsigned int inicio;
  unsigned int fin;
}recortarTrama;

typedef struct{
  int sampleRate;
  int channels;
  int format;
  int bloque;
}parametroAudio;

typedef struct{
  int x;
  int y;

}Coordenadas;

extern double complex outBuffer[N];
extern sonido audio;
extern Ventana ventana;
extern VisualizacionFreq vFreq;
extern char* programa;
extern recorte r;
extern parametroAudio param;
extern double buffer[N];
extern int buffer_index;
extern int tamanoBuffer;
extern recortarTrama trama;
extern int guardarFrecuencias;
extern char* ficheroF;
extern int anchuraMax;
extern int alturaMax;
extern int canal;

int ObtenerDatosAudio(char * ficheroAudio);
void LiberarDatosAudio();
bool ValidarArchivo(char *fichero);
int ProcesarAudioWavMp3();
int ProcesarAudioRaw();
void InicializacionParametros(char *nombre);
void ControlParametros();
void ObtenerUsoPrograma();


#endif
