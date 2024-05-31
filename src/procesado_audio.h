#ifndef PROCESADO_AUDIO_H
#define PROCESADO_AUDIO_H

#include <stdlib.h>
#include <complex.h>


int CrearDispositivoAudio(char *fichero);

int ComenzarAudio(char *fichero);

void CerrarDispositivo();

void CerrarEngine();

  void CerrarSound();

void AbrirFreqRaw();

void CerrarFreqRaw();

int InitFFTW();

void FreeFFTW();

void data_callback();

void GuardarMuestrasTR(float (*samples)[2],uint32_t frameCount);

int LeerRaw(char *archivoRaw, int b);

void Pausar();

void Renaudar();

void SetPitch(float pitch);

void SetVolumen(float tono);

void ObtenerTiempo();

int SonidoTerminado();

void ImprimirInformacion();

void RecortarTrama();

char * getFormatAudio();

void EjecutarFFTWForward(unsigned int frames,int channels, void *inData, int flag);

void CrearRaw(unsigned int size, double complex * frecuencias, int flag);


#endif
