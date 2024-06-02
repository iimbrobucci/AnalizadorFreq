#include "global.h"
#include "procesado_audio.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// BUfferes para la obtención de muestras y frecuencias
double complex outBuffer[N];
double buffer[N];


// Declaración de estructuras
sonido audio; // parametros de audio
Ventana ventana; // parametros de ventana
VisualizacionFreq vFreq; // parametros de visualización de frecuencias
recorte r;// parametros para recortar audio en tiempo real
parametroAudio param;// parametros para crear audio
recortarTrama trama;// parametros para recortar trama

char *programa;// nombre del programa

int buffer_index; // indice del buffer circular
int canal;// canal del cual se tomaran las muestras
int tamanoBuffer; // tamaño del buffer
int guardarFrecuencias; // booleano para guardar las frecuencias en tiempo real

char *ficheroF;// nombre del fichero para almacenar las frecuencias


int anchuraMax;// altura máxima que puede tener la ventana
int alturaMax;// anchura máxima que puede tener la ventana


//Función que almacena los datos del audio ( nombre, ruta y tipo)
int ObtenerDatosAudio(char * ficheroAudio){
  audio.ruta=ficheroAudio;
  
  audio.tipo= strrchr(ficheroAudio,'.');
    if (audio.tipo == NULL) {
      fprintf(stderr, " %s No se pudo determinar la extensión del archivo de audio.\n", programa);
       free(audio.ruta);
        return 1;
    }
  
  audio.nombreAudio=strrchr(audio.ruta,'/');
  if(audio.nombreAudio==NULL){
    int longitud= strlen(audio.ruta);
    int longitudTipo= strlen(audio.tipo);
    audio.nombreAudio= (char *) malloc( sizeof(char) * longitud-longitudTipo +1);
    strncpy(audio.nombreAudio, audio.ruta,longitud-longitudTipo);
  }else{
    char* aux=&audio.nombreAudio[1];
    int longitud= strlen(aux);
    int longitudTipo= strlen(audio.tipo);
    audio.nombreAudio= (char *) malloc( sizeof(char) * longitud-longitudTipo +1);
    strncpy(audio.nombreAudio,aux ,longitud-longitudTipo);
  }

    return 0;
 
}

// Libera los datos del audio
void LiberarDatosAudio(){
  free(audio.ruta);
  free(audio.nombreAudio);
}

// Comprueba si el archivo pasado como argumento se puede abrir y leer
bool ValidarArchivo(char *fichero){
    FILE *f = fopen(fichero, "rb");
    if(f == NULL){
      fprintf(stderr,"%s El archivo de audio %s No se se puede leer\n",programa,fichero);
      return false;
    }
    fclose(f);
    return true;
}

// Procesado de archivos WAV MP3 FLAC
int ProcesarAudioWavMp3(){
  audio.spectogram = 1;// se habilita la visualización de frecuencias
    
    // Se crea el dispositivo de reprodución
    if(CrearDispositivoAudio(audio.ruta) == 1){
      LiberarDatosAudio();
      return 1;
    }

    // Se inicializa el plan para calcular la FFT
    if(InitFFTW()==1){
      CerrarEngine();
      CerrarDispositivo();
      LiberarDatosAudio();
      return 1;
    }
    
    AbrirFreqRaw();// Se abre el archivo para almacenar las frecuencias
    // Se empieza a reproducir el audio
    if(ComenzarAudio(audio.ruta) == 1){
      LiberarDatosAudio();
      return 2;
    }
    return 0;
}

// Procsado de archivos RAW
int ProcesarAudioRaw(){
  audio.spectogram=0;// Se habilita la visualización de muestras
  guardarFrecuencias=1;
  
  // Se procesa el archivo y crea el audio
   if(LeerRaw(audio.ruta, param.bloque)==1){
     LiberarDatosAudio();
     return 3;
  }
   // Se crea el dispositivo para la reprodución
    if(CrearDispositivoAudio(audio.ruta) == 1){
      LiberarDatosAudio();
      return 1;
    }

    // Se empieza a reproducir el audio
    if(ComenzarAudio(audio.ruta) == 1){
      LiberarDatosAudio();
      return 2;
      
    }
    return 0;
}

// Inicialización de los parametros de programa por defecto
void InicializacionParametros(char *nombre){

  programa=nombre;
  
  buffer_index=0;
  tamanoBuffer=N;

  audio.volumen=0.5;
  audio.tono=1.0;
  audio.tiempo=0.0;
  audio.printInfo=1;
  audio.pausado=1;
  audio.spectogram=1;
  
  ventana.alto=800;
  ventana.ancho=1200;

  vFreq.inicio=1.0;
  vFreq.fin=tamanoBuffer/2;
  vFreq.suavidad=8;

  r.recortando=1;
  r.muestras=NULL;
  r.size=0;
  r.version=0;

  trama.recortar=1;
  trama.inicio=0;
  trama.fin=0;

  param.sampleRate=44100;
  param.channels=1;
  param.format=64;
  param.bloque=0;

  guardarFrecuencias=1;

  canal=0;

}


// Controlar errores en paramtros pasados por el usuario
void ControlParametros(){
  
  if(vFreq.fin >tamanoBuffer/2) vFreq.fin=tamanoBuffer/2;
  if(vFreq.inicio >= vFreq.fin){
    fprintf(stderr,"%s El indice de inicio de visualizacion de frecuenciano puede ser mayor o igual al indice de fin\n", programa);
    fprintf(stderr,"-i = %d\n",1);
    vFreq.inicio=1;
  }
  
     
  if(trama.recortar==0 && (trama.inicio==trama.fin || trama.inicio>trama.fin)){
    fprintf(stderr,"%s No se puede hacer un recorte del audio ya que debe especificarse el indice de muestras de Inicio (-a) y fin (-l), siendo inicio<fin.\n", programa);
    trama.recortar=1;
  }

}


// Función para obtener el uso del programa
void ObtenerUsoPrograma(){
  fprintf(stdout, "Uso del programa:\n");
    fprintf(stdout, "  -f <fichero>     : Especifica el fichero de audio. (Obligatorio)\n");
    fprintf(stdout, "                     Solo se permiten audios *tipo = WAV, MP3, RAW.\n");
    fprintf(stdout, "                          *tipo = WAV, MP3, RAW.\n");
    fprintf(stdout, "                          *formato = 8 bits, 16 bits, 24 bits, 32 bits y punto flotante.\n");
    fprintf(stdout, "                          *canales = 1, 2.\n");
    fprintf(stdout, "                     Los archivos tipo MP3 Y WAV, serán codificados, reproducidos y mostrarán las frecuencias presentes en ellos a través de una interfaz\n");
    fprintf(stdout, "                     Los archivos tipo RAW deben obtener solo valores de frecuencias de audio en formato 'double complex', a partir de ellos será posible crear\n");
    fprintf(stdout, "                     un archivo de audio que luego se reproducidirá y mostrará  sus muestras a través de una interfaz.\n");
    fprintf(stdout, "  -v <volumen>     : Ajusta el volumen del audio (rango [0.0, 1.0]). (Por defecto 0.5).\n");
    fprintf(stdout, "  -t <tono>        : Ajusta el tono del audio (rango [1.0, 6.0]). (Por defecto 1.0).\n");
    fprintf(stdout, "  -p               : Imprime la información del audio (Formato, número de canales, tamaño de muestra, número total de frames).\n");
    fprintf(stdout, "  -h <alto>        : Especifica la altura de la ventana de interfaz de usuario (rango [600, 1400]). (Por defecto 800).\n");
    fprintf(stdout, "  -w <ancho>       : Especifica el ancho de la ventana de interfaz de usuario (rango [800, 2400]). (Por defecto 1200).\n");
    fprintf(stdout, "  -n <tamaño>      : Especifica el tamaño del búfer de procesamiento de muestras (rango [1024, 8192]).\n");
    fprintf(stdout, "                     Solo se permiten potencias de dos (Por defecto 8192). Establece el tamaño del búfer que se usara para calcular\n");
    fprintf(stdout, "                     la Transformada de Fourier de las muestras y, por ende, el número de muestras que serán representadas a la vez\n");
    fprintf(stdout, "                     en la interfaz, es decir, las muestras serán procesadas en tamaños de ventana que pueden variar según (-n).\n");
    fprintf(stdout, "  -i <inicio>      : Especifica el índice de inicio de visualización de frecuencias (rango [0, 200]). (Por defecto 1.0).\n");
    fprintf(stdout, "  -e <fin>         : Especifica el índice de fin de visualización de frecuencias (rango [64, 4096]). (Por defecto 4096).\n");
    fprintf(stdout, "                     Si el índice de fin es mayor que la mitad del tamaño de la ventana (-n) se establecerá a ser igual al tamaño de la\n");
    fprintf(stdout, "                     ventana/2. Si el índice de inicio es mayor que el índice de fin el índice de inicio pasara a ser 1.\n");
    fprintf(stdout, "  -q <canal>       : Especifica el canal del audio que se quiere procesar ( 1 o 2 ). 1 corresponde al izquierdo y 2 al derecho. Por defecto 1\n");
    fprintf(stdout, "  -r               : Recorta una trama del audio. (Si se habilita esta opción se debe especificar el rango a cortar (-a) (-l).\n");
    fprintf(stdout, "  -a <inicio>      : Especifica el número de frame a partir de la cual se quiere empezar a recortar la trama (>= 0).\n");
    fprintf(stdout, "  -l <fin>         : Especifica el número de la última frame que se quiere recortar de la trama (>= 0).\n");
    fprintf(stdout, "  -s <frecuencia>  : Especifica la frecuencia de muestreo para la creación de audios (rango [8000, 96000]). (Por defecto 44100).\n");
    fprintf(stdout, "  -c <canales>     : Especifica el número de canales para la creación del audio (1 o 2). (Por defecto 1)\n");
    fprintf(stdout, "  -b <bloque>      : Especifica el tamaño de bloque al que se le debe aplicar la FFT en la creación de audio rango[1024, 8192]. (Por defecto es el tamaño del archivo).\n");
    fprintf(stdout, "  -m <formato>     : Especifica el formato para la creación del audio (8, 16 o 32 bits). (Por defecto 32)\n");
    fprintf(stdout, "                     -s, -c, -b, -m Solo se utilizan en el caso de pasar un archivo tipo RAW.\n");
    fprintf(stdout, "  -d <nombre>      : Activa el guardado de las frecuencias en el nombre del archivo que se pase.\n");
    fprintf(stdout, "  -u               : Muestra la información de uso del programa.\n");
}
