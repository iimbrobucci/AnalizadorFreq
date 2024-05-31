#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "procesado_audio.h"
#include "global.h"
#include <fftw3.h>

// declaración de variables para reporducir el audio

ma_result result;// variable de control

ma_engine engine;// crear la engine
ma_engine_config engineConfig;// configurar la engine

ma_decoder decoder; // decodificar el audio
ma_device device; // crear dispositivo
ma_device_config deviceConfig; // configurar dispositivo

ma_sound sound;// crear sonido

ma_encoder encoder;// crear audio
ma_encoder_config config;// configuración para crear audio

fftw_plan plan;// plan para ejecutar FFT

ma_uint64 timeF;

// Ejeucta la FFT para recortar archivos de audio
void EjecutarFFTWForward(unsigned int frames, int channels,void* inData, int flag){
  
  unsigned int size= frames*channels;// Se calcula el tamaño de la FFT
  
  double complex * out= (double complex *) malloc(sizeof(double complex) * size); // variable para almacenar las frecuencias
  double *in = (double *) malloc( sizeof(double) *size); // variable donde se almacenaran las muestras de entrada
  if(out==NULL || in==NULL){
    fprintf(stderr, "%s No se ha podido crear memoria dinamica\n", programa);
    free(out);
    free(in);
    return;
  }

  //Se crea el plan para ejecutar la FFT
  fftw_plan forward= fftw_plan_dft_r2c_1d(size,in, out, FFTW_ESTIMATE);

  // Se copian las muestras de entrada en in
  for(ma_uint64 i=0; i<size; i++){
    if(flag==2){
       in[i]= (double) ((double *)inData)[i];
    }else
    if(audio.format==ma_format_u8){
      in[i]=(double)((uint8_t *)inData)[i];
    }else if( audio.format== ma_format_s16){
      in[i]= (double) ((int16_t *)inData)[i];
    }else if(audio.format== ma_format_f32){
      in[i]= (double) ((float *)inData)[i];
    }else if(audio.format == ma_format_s24){
      in[i] = (double) ((int *)inData)[i];
    }else if(audio.format == ma_format_s32){
      in[i] = (double) ((int32_t *)inData)[i];
    }
  }

  fftw_execute(forward);// se ejecuta la FFT

  fftw_destroy_plan(forward);// Se destruye el plan
  free(in);// se libera memoria dinamica

  CrearRaw(size, out, flag); // Se crea el archivo y se alamcenan las frecuencias
  free(out);// se libera la memoria 

}

//Crea un archivo RAW y alamcena las frecuencias
void CrearRaw(unsigned int size, double complex *frecuencias, int flag){

  //se establce memoria para crear el nombre del archivo
  int length= strlen(audio.nombreAudio);
  char * name = (char *) malloc(sizeof(char) * length + 5 + flag);
  if(name==NULL){
    fprintf(stderr,"%s No se ha podido recortar trama\nLa memoria dinamica no se ha podido asignar\n",programa);
    return;
  }

  size_t num;// variable para almacenar el numero de freceuncias escritas en el archivos

  //Se establece el nombre del archivo
  if(flag==0) sprintf(name,"%s.raw",audio.nombreAudio);
  else sprintf(name,"%s%d.raw", audio.nombreAudio,r.version++);

  FILE *fraw=fopen(name,"wb");// Se abre el archivo
    if(fraw==NULL){
      fprintf(stderr, "%s No se ha podido recortar trama\nEl archivo %s no se puede abrir o escribir\n",programa, name);
      free(name);
      return;
    }else{
      num= fwrite(frecuencias, sizeof(double complex),size, fraw);// se escriben las frecuencias en el archivo
      fclose(fraw);// Se cierra el archivo
    }

    // Se imprimen el nombre del arhivo creado y el formato de las muestras a partir de las cuales se calcularon las frecuencias
    fprintf(stdout,"Se ha creado el archivo %s\n",name);
    fprintf(stdout,"Fecuencia de Muestreo = %d\n", audio.sampleRate);
    if(flag==2){
       fprintf(stdout,"Tamaño de muestra = 32 bits f\n");
       fprintf(stdout,"Canales = 1\n");
    }else{
      fprintf(stdout,"Tamaño de muestra = %s\n", getFormatAudio());
      fprintf(stdout,"Canales = %d\n", audio.channels);
    }
    fprintf(stdout,"Numero de muestras = %ld\n", num);

    free(name);

}

//Recortar TRama de los audios 
void RecortarTrama(){

  // Comprueba que la utlima frame a recortar no sea mayor que el número total de franes del audio
  if(trama.fin>audio.totalFrames){
    fprintf(stderr,"%s No se ha podido recortar trama\nEl número de Frames del audio %d es menor que el rango establecido %d - %d\n", programa, audio.totalFrames, trama.inicio, trama.fin);
    return;
  }

  result = ma_decoder_seek_to_pcm_frame(&decoder, trama.inicio); // se ubica en la trama de inicio a recortar
  if (result != MA_SUCCESS) {
    fprintf(stderr,"%s No se ha podido recortar trama\nLa posición inicial de la trama (%d) no ha sido establecida\n", programa, trama.inicio);
    return;
  }


  ma_uint64 framesToRead= trama.fin - trama.inicio; // se calula el número de frames a leer
  void* pFrames; // variable donde se leeran las frames
  // se crea espacio para almacenar las frames segun el formato del audio
  if(audio.format==ma_format_u8){ pFrames= (uint8_t *) malloc( sizeof(uint8_t) * framesToRead* audio.channels);}
  else if(audio.format==ma_format_s16){ pFrames = (int16_t *) malloc( sizeof(int16_t) *framesToRead * audio.channels);}
  else if(audio.format==ma_format_f32){ pFrames = (float *) malloc( sizeof(float) *framesToRead*audio.channels);}
  else if(audio.format==ma_format_s24){ pFrames=  (int *) malloc(sizeof(int) *framesToRead*audio.channels);}
  else if(audio.format==ma_format_s32){ pFrames = (int32_t *) malloc( sizeof(int32_t) *framesToRead * audio.channels);}
  if(pFrames==NULL){
    fprintf(stderr,"%s No se ha podido recortar trama\nLa memoria dinamica no se ha asignado\n",programa);
     ma_decoder_seek_to_pcm_frame(&decoder, 0);
     return;
  }
  
  ma_uint64 framesRead;
  result = ma_decoder_read_pcm_frames(&decoder, pFrames, framesToRead, &framesRead);// se leen las frames del audio y se almacenan en pFrames
  if (framesRead < framesToRead) {
    fprintf(stderr,"%s Recortar trama:\nSe han leido %lld frames en vez de %lld\n", programa, framesRead, framesToRead);
  }

  ma_decoder_seek_to_pcm_frame(&decoder, 0);// se establece de nuevo el puntero al inicio del audio
    
  EjecutarFFTWForward(framesRead,audio.channels, pFrames, 0);// Se ejecuta la FFT 

  free(pFrames);// Se libera la memoria 
  
}

// Inicializa el dispositivo de audio y la engine
int CrearDispositivoAudio(char *fichero){

  result= ma_decoder_init_file(fichero,NULL,&decoder); // decodifica el audio
  if(result!=MA_SUCCESS){
    fprintf(stderr,"No se ha podido cargar el archivo %s\n",fichero);
    return 1;
  }

  ma_uint64 length;
  result = ma_data_source_get_length_in_pcm_frames(&decoder, &length); // se obtienen el número de frames totals del audio
  if (result != MA_SUCCESS) {
    fprintf(stderr, "%s No se ha podido obtener el numero total de Frames del audio\n", programa);
    ma_decoder_uninit(&decoder);
    return 1;
  }

  audio.format= decoder.outputFormat; // se guarda el formato del audio
  audio.channels=decoder.outputChannels; // se guarda el número de canales del audio
  audio.sampleRate=decoder.outputSampleRate; // se gurada la frecuencia de muestreo del audio
  audio.totalFrames=length; // se guarda el número de frames del audio

  // Se comprueba que el formato del audio este definido y soportado por miniaudio y que el número de canales sea 1 o 2
  if((audio.format!=ma_format_u8 && audio.format!=ma_format_s16 && audio.format!=ma_format_s24 && audio.format!= ma_format_s32 && audio.format!=ma_format_f32) || (audio.channels!= 1 && audio.channels!=2)){
    fprintf(stderr,"%s Solo se aceptan audios que tengan frecuencia de muestreo de 8bits, 16bits, 24bist, 32bits y punto flotante, y numero de canales tipo mono(1) o estereo(2)\n", programa);
     ma_decoder_uninit(&decoder);
    return 1;
  }
  // si se pasa como parametro de entrada un tramo de frames a recortar se llama a la función recortar
  if(trama.recortar==0){
    RecortarTrama();
    trama.recortar=1;
  }

  ma_decoder_uninit(&decoder);// se cierra el decodificador

  // Se configura el dispositivo para la reproduci´on de audio
  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format   = ma_format_f32; // formato en punto flotante
  deviceConfig.playback.channels = 2;// dos canales
  deviceConfig.sampleRate        = audio.sampleRate;// la misma frecuencia de muestreo que el audio original
  deviceConfig.dataCallback      = data_callback; // función para recibir las muestras
  deviceConfig.pUserData         = &engine;

  // Se inicializa el dispositivo de audio
 if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
      fprintf(stderr,"No se ha podido abrir un dispositivo \n");
      return 1;
    }

 // Se configura ls engine pasandole el dispositivo configurado y especificando que no empiece automaticamente
    engineConfig=ma_engine_config_init();
    engineConfig.pDevice = &device;
    engineConfig.noAutoStart  = MA_TRUE;

    // Se crea la engine
    result = ma_engine_init(&engineConfig, &engine);
     if (result != MA_SUCCESS) {
	fprintf(stderr,"No se ha podido inicializar una engine\n");
         ma_device_uninit(&device);
         return 1;
     }

     return 0;
  
}

//Carga el sonido y empieza a reproducirlo
int ComenzarAudio(char *fichero){

  // Se empieza la engine
    result = ma_engine_start(&engine);
     if (result != MA_SUCCESS) {
	 fprintf(stderr,"No se ha podido empezar el engine\n");
	  ma_engine_uninit(&engine);
          ma_device_uninit(&device);
	  return 1;
        }

     // Se inicializa el sonido a partir del audio y la engine
  result = ma_sound_init_from_file(&engine, fichero , MA_SOUND_FLAG_DECODE, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
      fprintf(stderr,"No se ha podido cargar el audio %s\n",fichero);
            ma_engine_uninit(&engine);
            ma_device_uninit(&device);
            return 1;
        }

    // Se empieza a reproducir el sonido
     result = ma_sound_start(&sound);
        if (result != MA_SUCCESS) {
	  fprintf(stderr,"No se ha podido empezar el audio %s\n",fichero);
	    ma_sound_uninit(&sound);
	    ma_engine_uninit(&engine);
            ma_device_uninit(&device);
	    return 1;
        }

	SetVolumen(audio.volumen); // se establece el volumen del audio
	SetPitch(audio.tono); // Se establece el tono del audio

	if(audio.printInfo==0) ImprimirInformacion(); // si se activa se imprime los parametros del audio

	return 0;
       	
}

// Función para cerrar la engine
void CerrarEngine(){
   ma_engine_uninit(&engine);
}

// Función para cerrar el sonido
void CerrarSound(){
   ma_sound_uninit(&sound);
}

// Función para cerrar el dispositivo
void CerrarDispositivo(){
  ma_device_uninit(&device); 
}

// Fucnión para abrir el archivo donde se iran escribiendo todas las frecuencias
void AbrirFreqRaw(){
  if(guardarFrecuencias==0){
    int l= strlen(ficheroF);
    char *nombre= (char*) malloc( sizeof(char) * l + 6);
    sprintf(nombre,"%s.raw",ficheroF);
    audio.fileF=fopen(nombre,"wb");
    if(audio.fileF==NULL){
      fprintf(stderr,"%s No se ha podido crear el archivo %s\n", programa, nombre);
      free(nombre);
      guardarFrecuencias=1;
    }
    free(nombre);
  }
}

// Función para cerrar el archivo donde se escriben todas las frecuencias mostradas por la ventana
void CerrarFreqRaw(){
   if(guardarFrecuencias==0 && audio.fileF!=NULL){
     fprintf(stdout,"Se han escrito las frecuencias en el archivo %s\n", ficheroF);
   fprintf(stdout,"Tamaño de bloque = %d\n", tamanoBuffer);
   fprintf(stdout,"Frecuencia de muestreo = %d\n", audio.sampleRate);
   fprintf(stdout,"Tamaño de muestra = 32 bits\n");
   fprintf(stdout,"Canales = 1\n");
   fclose(audio.fileF);
  }
  
}

// Se inicialia el plan para ejecutar la FFT
int InitFFTW(){
  // el plan tendra el mimso tamaño que el buffer, las muestras de entrada estran almacenadas en la variable buffer y las frecuencias se obtendran en ourBUffer
  plan= fftw_plan_dft_r2c_1d(tamanoBuffer, buffer, outBuffer, FFTW_MEASURE);
  if(plan==NULL){
    fprintf(stderr,"%s No se ha podido crear el algoritmo FFT\n", programa);
    return 1;
  }
  return 0;
}

// Se destruye el plan para calcular la FFT
void FreeFFTW(){
   fftw_destroy_plan(plan);
}

// Pausar el audio
void Pausar(){
  ma_sound_stop(&sound);
}

// Renaudar el audio
void Renaudar(){
  ma_sound_start(&sound);
}

// Cambia el tono del audio
void SetPitch(float pitch){
  ma_sound_set_pitch(&sound, pitch);

}

// Cambia el volumen del audio
void SetVolumen(float volume){
  ma_sound_set_volume(&sound, volume);

}

// Deterima si se ha terminado de reproducir el audio
int  SonidoTerminado(){
  if( ma_sound_at_end(&sound) == MA_TRUE ){
    return 0;
  }else{
    return 1;
  }  
}

// Obtiene el tiempo que lleva reproduciendose el audio
void ObtenerTiempo(){

  if(audio.pausado==1){
 
    ma_uint64 currenTime = timeF; // se obtienen el número de frames leidas hasta ahora

  float segundos= (float)currenTime/ audio.sampleRate;// se divide el numero de frames entre la frecuencia de muestreo para obtener los segundos

  float s= (float)((int)segundos % 60)/100; // se obtiene el resto, es decir los segundos del audio

  float entero= (float) segundos/60;// se calcula cuantos minutos han pasado

  audio.tiempo=s;// se suman los segundos
  if(entero >=1) audio.tiempo+= (float) floor(entero); // se suman los minutos
  }
}


// Fucnión donde se reciben las muestras de audio enviadas por miniaudio y se aplica la FFT para calcular sus frecuencias
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  (void)pInput;
  
  if(audio.pausado==1)   timeF+=frameCount; // Se guarda el número de frames que se van leyendo

   // Se leen las frames y se almacenan en pOutput
   ma_engine_read_pcm_frames((ma_engine*)pDevice->pUserData, pOutput, frameCount, NULL);

   // Se leen las muestras en formato punto flotante
   float (*samples)[2] = pOutput;
   
   for (ma_uint32 i = 0; i < frameCount; i++) {
     
     buffer[buffer_index] = (double)samples[i][canal];// Se copian las muestras en el buffer circular

     // Si el buffer esta lleno y se quieren obtener las frecuencias de audio
      if (buffer_index == tamanoBuffer - 1 && audio.spectogram == 1) {
	
	fftw_execute(plan); // Se ejecuta la FFT
	
	// Se van guardando las frecuencias en el archivo "Frecuencias.raw" si esta activado
        if (guardarFrecuencias == 0) {
                fwrite(outBuffer, sizeof(double complex), tamanoBuffer, audio.fileF);
        }// if
	
      }// if

     // se incrementa el indice del buffer
      buffer_index = (buffer_index + 1) % tamanoBuffer; 
	
    }

   //Si se quiere recortar una trama se guardan las muestras para calcular sus componentes de frecuencia
      if(audio.spectogram==1 && r.recortando==0){
	GuardarMuestrasTR(samples, frameCount);
       }
}

// Guarda las muestras de audio en tiempo real
void GuardarMuestrasTR(float (* samples)[2], uint32_t frameCount ){

  	if(r.muestras==NULL){
	  r.muestras= (double *) malloc(sizeof(double)*frameCount);
	}else{
	  r.muestras= (double *) realloc(r.muestras, sizeof(double)* (r.size+frameCount));
	}
      
	if(r.muestras==NULL){
	  fprintf(stderr,"%s No se ha podido asignar memoria dinámica\n",programa);
	  r.recortando=1;
	  r.size=0;
	}else{
	
	  for(ma_uint32 i=0; i<frameCount; i++){
	    r.muestras[r.size+i]=(double)samples[i][canal];
	  }
	  r.size+=frameCount;
      }

}

// Devuelve un string con el formato del audio
char * getFormatAudio(){

  switch(audio.format){
  case ma_format_u8: return "8 bits"; break;
  case ma_format_s16: return "16 bits"; break;
  case ma_format_s24: return "24 bits"; break;
  case ma_format_s32: return "32 bits"; break;
  case ma_format_f32: return "32 bits f"; break;
  }

  return "Unkown Format";
  
}

// Imprimer las caracteristicas del audio
void ImprimirInformacion(){
   char *format= getFormatAudio();
   fprintf(stdout,"%s Audio = %s\n",programa,audio.ruta);
   fprintf(stdout," %s.sampleSize = %s\n",audio.nombreAudio, format);
   fprintf(stdout," %s.channels = %i\n",audio.nombreAudio, audio.channels);
   fprintf(stdout," %s.sampleRate = %i\n",audio.nombreAudio, audio.sampleRate);
   fprintf(stdout," %s.totalFrames = %i\n",audio.nombreAudio, audio.totalFrames);
}


// Lee y procesa los archivos Tipo RAW
int LeerRaw(char *archivoRaw, int b){

  double complex * frecuencias;
  double *outbackward;
  void * senal;
  fftw_plan backward;
  ma_uint64 bloque= (ma_uint64) b;

  // Abre el archivo  
  FILE *archivo=fopen(archivoRaw,"rb");
  if(archivo==NULL){
    fprintf(stderr,"%s No se ha podido abrir el archivo %s \n", programa, archivoRaw);
    return 1;
  }

  // obtiene la longitud del archivo en bytes
  fseek(archivo, 0, SEEK_END);
  long long size= ftell(archivo);
  rewind(archivo);

  // Se comprueba que el archivo este formado por frecuencias en formato "double complex"
  if( size % sizeof(double complex) != 0 ){
    fprintf(stderr, "%s El archivo no esta formado por valores de frecuencia en formato 'double complex'\n", programa);
    fclose(archivo);
    return 1;
  }

  // Calcula número de muestras diviendiendo el número d bytes del fichero entre el número de bytes que ocupa double complex
  ma_uint64 numMuestras= size / sizeof(double complex);    
  if(numMuestras==0){
    fprintf(stderr,"%s El archivo .raw no contiene ninguna muestra\n", programa);
    fclose(archivo);
    return 1;
  }

  // Si bloque es igual a cero se hace la transformada al fichero entero en conjunto
  if(bloque == 0) bloque = numMuestras;

  // Se compueba que el ficherp este dividido or el número de bloque establecido
  if(numMuestras % bloque != 0){
    fprintf(stderr,"%s El bloque no concuerda con el numero de muestras presentes en el fichero\n", programa);
    fclose(archivo);
    return 1;
  }
  
  ma_uint64 framesToWrite = bloque / param.channels; // Número de frames a escribir
  if(!(framesToWrite == bloque) && !(framesToWrite == bloque / 2)){
    fprintf(stderr, "%s El numero de frames no es compatible con el numero de muestras:\nFrames: %lld\nMuestras: %lld\n", programa, framesToWrite, numMuestras);
    fclose(archivo);
    return 1;
  }

  //Se crea la memoria para almacenar las muestras segun el formato del audio
  ma_format format;
  switch (param.format) {
        case 8:
            format = ma_format_u8;
            senal = malloc(sizeof(uint8_t) * bloque);
            break;
        case 16:
            format = ma_format_s16;
            senal = malloc(sizeof(int16_t) * bloque);
            break;
        case 24:
	  senal = malloc(sizeof(int) *bloque);
	  break;
        case 32:
            format = ma_format_s32;
            senal = malloc(sizeof(int32_t) * bloque);
            break;
        default:
	    format = ma_format_f32;
            senal = malloc(sizeof(float) * bloque);
            break;
    }

  // Se crea memoria para calcular la FFT
  frecuencias = (double complex *) malloc( bloque * sizeof(double complex));
  outbackward= (double *) malloc(bloque * sizeof(double));
  if(frecuencias==NULL || outbackward==NULL || senal== NULL){
    fprintf(stderr,"%s No se ha podido asignar memoria dinamica\n", programa);
    free(frecuencias);
    free(outbackward);
    free(senal);
    fclose(archivo);
    return 1;
  }

  // Se crea el plan para calcular la FFT
   backward= fftw_plan_dft_c2r_1d(bloque, frecuencias, outbackward, FFTW_ESTIMATE);
    if (backward == NULL) {
        fprintf(stderr, "%s No se ha podido crear el plan FFTW\n", programa);
        free(frecuencias);
        free(outbackward);
        free(senal);
        fclose(archivo);
        return 1;
    }

    // Se establece el nombre del audio
   int length = strlen(audio.nombreAudio);
   char *name =  (char *) malloc( length + 5 * sizeof(char));
    if (name == NULL) {
        fprintf(stderr, "%s No se ha podido asignar memoria para el nombre del archivo\n", programa);
        fftw_destroy_plan(backward);
        free(frecuencias);
        free(outbackward);
        free(senal);
        fclose(archivo);
        return 1;
    }
   sprintf(name,"%s.wav",audio.nombreAudio);


  config= ma_encoder_config_init(ma_encoding_format_wav,format, param.channels, param.sampleRate);  // Se configura el archivo de audio a crear
  result= ma_encoder_init_file(name,&config,&encoder); // Se inicializa el archivo de audio
  if(result!=MA_SUCCESS){
    fprintf(stderr,"%s No ha sido posible crear el archivo de audio\n", programa);
    fftw_destroy_plan(backward);
    free(frecuencias);
    free(outbackward);
    free(senal);
    free(name);
    fclose(archivo);
    return 1;
  }

   
   ma_uint64 framesWritten;
   int numBloques = numMuestras / bloque; // Se calcula el número de iteracciones
   for(int i=0; i<numBloques; i++){
     
     // Se leen los bloques de frecuencia del archivo RAW 
      if (fread(frecuencias, sizeof(double complex), bloque, archivo) != bloque) {
            fprintf(stderr, "%s Error al leer el archivo .raw\n", programa);
            ma_encoder_uninit(&encoder);
            free(name);
            fftw_destroy_plan(backward);
            free(frecuencias);
            free(outbackward);
            free(senal);
            fclose(archivo);
            return 1;
      }
     
      fftw_execute(backward);// Se ejecuta la FFT

      // Se guardan los valores de muestra obtenidos segun el formato
     for(ma_uint64 j =0; j < bloque; j++){
       if(format==ma_format_u8){
	 ((uint8_t *) senal)[j]=(uint8_t) (outbackward[j]/bloque);
       }else if(format==ma_format_s16){
	 ((int16_t*) senal)[j]=(int16_t) (outbackward[j]/bloque);
       }else if(format==ma_format_f32){
	 ((float *) senal)[j]=(float) (outbackward[j]/bloque);
       }else if(format==ma_format_s24){
	 ((int *) senal)[j] = (int) (outbackward[j]/bloque);
       }else if(format==ma_format_s32){
	 ((int32_t *) senal)[j]= (outbackward[j]/bloque);
       }
      }

     // Se copian los valores de muestra obtenidos en el archivo de audio
    result= ma_encoder_write_pcm_frames(&encoder, senal, framesToWrite, &framesWritten);
    if(result!=MA_SUCCESS){
    fprintf(stderr," %s No se ha podido escribir en el archivo de audio\n", programa);
    ma_encoder_uninit(&encoder);
    fftw_destroy_plan(backward);
    free(frecuencias);
    free(outbackward);
    free(senal);
    fclose(archivo);
    free(name);
    return 1;
  }  

   }

   // Se cierra el audio y se libera la memoria
  ma_encoder_uninit(&encoder);
  fftw_destroy_plan(backward);
  free(frecuencias);
  free(outbackward);
  free(senal);
  fclose(archivo);

  // Se obtiene los parametros del archivo de audio para su reprodución
  LiberarDatosAudio();
  ObtenerDatosAudio(name);

  return 0;
 
}


