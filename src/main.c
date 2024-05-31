#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "procesado_audio.h"
#include "interfaz.h"


int main(int argc, char *argv[]){
  
  int opt;
  char *fichero;
  char *endptr;
  int numarg=0;

  // Se inicializan los parametros del programa
  InicializacionParametros(argv[0]);

  // Se leen los argumentos por la linea de comandos
  while((opt = getopt(argc, argv, "f:v:t:a:w:h:s:n:q:i:l:e:d:c:b:m:pru")) != -1)
    {
      switch(opt)
	{
	case 'f':
	  fichero = optarg;
	  numarg++;
	  break;
	case 'v':
	  endptr=NULL;
            errno=0;
            float vol = strtof(optarg,&endptr);
            if(endptr[0]!='\0' || (errno == ERANGE && vol== HUGE_VALF) || (errno == ERANGE && vol==0) || vol > 1.0 || vol < 0.0){
                fprintf(stderr, "%s El volumen debe estar entre [0.0,1.0]. Por defecto 0.5\n", argv[0]);
                exit(1);
            }
            audio.volumen = (float)round(vol*10)/10;
	    break;
	case 't':
	   endptr=NULL;
            errno=0;
            float t = strtof(optarg,&endptr);
            if(endptr[0]!='\0' || (errno == ERANGE && t== HUGE_VALF) || (errno == ERANGE && t==0) || t < 1.0 || t> 6.0){
                fprintf(stderr, "%s El tono debe estar entre [1.0,6.0]. Por defecto 1.0\n", argv[0]);
                exit(1);
            }
            audio.tono=(float)round(t*10)/10;
            break;
	case 'p':
	  audio.printInfo=0;
	  break;
	case 'h':
	    endptr=NULL;
            errno=0;
            long int alto=strtol(optarg,&endptr,10);
            if(endptr[0]!='\0' ||(errno == ERANGE && (alto == LONG_MAX || alto==LONG_MIN)) || alto< 600 || alto > 1400){
                fprintf(stderr, "%s El alto de la ventana de audio (-h) debe estar comprendido entre [600, 1400]. Por defecto 800\n", argv[0]);
                exit(1);
            }
            ventana.alto=(int)alto;
	  break;
	case 'w':
	    endptr=NULL;
            errno=0;
            long int ancho=strtol(optarg,&endptr,10);
            if(endptr[0]!='\0'||(errno == ERANGE && (ancho == LONG_MAX || ancho==LONG_MIN)) || ancho < 800 || ancho > 2400 ){
                fprintf(stderr, "%s El ancho de la ventana de audio (-w) debe estar comprendido entre [800, 2400]. Por defecto 1200\n", argv[0]);
                exit(1);
            }
            ventana.ancho=(int)ancho;
	  break;
	case 'i':
	    endptr=NULL;
            errno=0;
            long int indice= strtol(optarg,&endptr,10);
            if(endptr[0]!='\0' ||(errno == ERANGE && (indice == LONG_MAX || indice==LONG_MIN)) || indice< 0 || indice > 200 ){
                fprintf(stderr,"%s El índice de inicio de visualización de frecuencias (-i) debe estar entre [0,200]. Por defecto uno\n",argv[0]);
                exit(1);
            }
	     vFreq.inicio=(int)indice;
	  break;
	case 'e':
	  endptr=NULL;
	  errno=0;
	  long int fin= strtol(optarg, &endptr,10);
	  if(endptr[0]!='\0' ||(errno == ERANGE && (fin == LONG_MAX || fin==LONG_MIN)) || fin< 64 || fin > 4096 ){
	    fprintf(stderr,"%s El índice de fin de visualización de frecuencias (-e) debe estar entre [64,4096]. Por defecto 4096\n", argv[0]);
	      exit(1);
	    }
	    vFreq.fin=(int)fin;
	  break;
	case 'n':
	  endptr=NULL;
	  errno=0;
	  long int n= strtol(optarg, &endptr, 10);
	  if(endptr[0]!='\0' || (errno == ERANGE && (n == LONG_MAX || n==LONG_MIN)) || n < 1024 || n > 8192 || (n && (n-1)) == 0){
	    fprintf(stderr,"%s El tamaño del buffer (-n) debe ser potencia de dos y estar entre [1024,8192]. Por defecto 8192\n", argv[0]);
	    exit(1);
	  }
	  tamanoBuffer=(int)n;
	  break;
	case 'r':
	  trama.recortar=0;
	  break;
	case 'a':
	  endptr=NULL;
	  errno=0;
	  long int i=strtol(optarg, &endptr, 10);
	   if(endptr[0]!='\0' || (errno == ERANGE && (i == LONG_MAX || i==LONG_MIN)) || i < 0 ){
	    fprintf(stderr,"%s El rango de inicio de recorte de trama (-a) debe ser mayor o igual a cero (>=0). Por defecto cero\n", argv[0]);
	    exit(1);
	  }
	   trama.inicio=(unsigned int )i;
	  break;
	case 'l':
	  endptr=NULL;
	  errno=0;
	  long int f=strtol(optarg, &endptr, 10);
	   if(endptr[0]!='\0' || (errno == ERANGE && (f == LONG_MAX || f==LONG_MIN)) || f < 0 ){
	    fprintf(stderr,"%s El rango  de recorte de trama (-f) debe ser mayor o igual a cero (>=0).\n", argv[0]);
	    exit(1);
	  }
	   trama.fin=(unsigned int )f;
	  break;
	case 's':
	  endptr=NULL;
	  errno=0;
	  long int sampleRate= strtol(optarg, &endptr, 10);
	   if(endptr[0]!='\0' ||(errno == ERANGE && (sampleRate == LONG_MAX || sampleRate ==LONG_MIN)) || sampleRate< 8000 || sampleRate > 96000 ){
	       fprintf(stderr,"%s La frecuencia de muestreo para la creación de audios debe estar entre [8000, 96000]. Por defecto 44100\n", argv[0]);
	      exit(1);
	    }
	   param.sampleRate= (int) sampleRate;
	   break;
	case 'c':
	   endptr=NULL;
	  errno=0;
	  long int c= strtol(optarg, &endptr, 10);
	  if(endptr[0]!='\0' ||(errno == ERANGE && (c == LONG_MAX || c ==LONG_MIN)) || (c!= 1 && c != 2) ){
	       fprintf(stderr,"%s El número de canales para la creación del audio debe ser 1 o 2. Por defecto 1\n", argv[0]);
	      exit(1);
	    }
	   param.channels= (int) c;
	  break;
	case 'b':
	  endptr=NULL;
	  errno=0;
	  long int b= strtol(optarg, &endptr, 10);
	  if(endptr[0]!='\0' ||(errno == ERANGE && (b == LONG_MAX || b ==LONG_MIN)) || b<1024 || b>8192 || ( b && (b-1)) == 0 ){
	       fprintf(stderr,"%s El tamaño de bloque para la creación de audio debe ser potencia de dos y estar entre [1024, 8192]. Por defecto es el tamaño del archivo\n", argv[0]);
	      exit(1);
	    }
	  param.bloque = (int)b;
	  break;
	case 'm':
	    endptr=NULL;
	  errno=0;
	  long int m= strtol(optarg, &endptr, 10);
	  if(endptr[0]!='\0' ||(errno == ERANGE && (m == LONG_MAX || m ==LONG_MIN)) || (m!= 8 && m != 16 && m!=24 && m!=32) ){
	       fprintf(stderr,"%s El tamaño de muestra para la creación del audio debe ser 8, 16, 24, 32 bits. Por defecto punto flotante\n", argv[0]);
	      exit(1);
	    }
	  param.format = (int)m;
	  break;
	case 'q':
	    endptr=NULL;
	  errno=0;
	  long int q= strtol(optarg, &endptr, 10);
	  if(endptr[0]!='\0' ||(errno == ERANGE && (q == LONG_MAX || q ==LONG_MIN)) || (q!= 1 && q!= 2) ){
	       fprintf(stderr,"%s El canal a procesar solo puede ser 1 o 2. Por defecto 1\n", argv[0]);
	      exit(1);
	    }
	  canal = (int)q -1;
	  break;
	case 'd':
	  ficheroF=optarg;
	  guardarFrecuencias=0;
	  break;
	case 'u':
	  ObtenerUsoPrograma();
	  exit(0);
	  break;
	default:
	  ObtenerUsoPrograma();
	  exit(1);
	  break;
	}

    }

  // Se comprueba que se pasara al menos un argumento al programa
   if(opt==-1 && numarg == 0 ){
     ObtenerUsoPrograma();
     exit(1);
    }

   // Se comprueba que se pasara un fichero 
   if(fichero==NULL){
     fprintf(stderr, "Uso: %s -f Audio\n",argv[0]);
     exit(1);
  }
   
   // Se valida si el fichero se puede abrir y es legible
  if(!ValidarArchivo(fichero)) exit(1);

  ControlParametros();// Se comprueba que los parametros esten bien

  // Se obtienen y guardan los datos del fichero
  char *ficheroAudio= strdup(fichero);
  if( ObtenerDatosAudio(ficheroAudio) != 0) exit(1);

  int res=0;
  if(strcmp(audio.tipo,".mp3")==0 || strcmp(audio.tipo,".wav")==0 || strcmp(audio.tipo,".flac")== 0){

      res = ProcesarAudioWavMp3();
      if(res==1){exit(1);
      }else if(res==2){
	 FreeFFTW();
	 CerrarFreqRaw();
	 exit(1);
      }
      DibujarVentana();
      CerrarSound();
      CerrarEngine();
      CerrarDispositivo();
   
    }else if(strcmp(audio.tipo, ".raw")==0){
      res= ProcesarAudioRaw();
      if(res!=0){
	  exit(1);
      }
      DibujarVentana();
      CerrarSound();
      CerrarEngine();
      CerrarDispositivo();
     
    }else{
      fprintf(stderr,"%s Solo se permiten archivos de tipo: WAV, MP3, FLAC, RAW\n",argv[0]);
      exit(1);
    }
    if(audio.spectogram==1){ FreeFFTW(); CerrarFreqRaw();}
    LiberarDatosAudio();

  return 0;
}


