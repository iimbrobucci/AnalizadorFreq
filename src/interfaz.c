#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "interfaz.h"
#include "global.h"
#include "procesado_audio.h"

// Variables para la visualización de frecuencias
double outLog[N];
double outSmooth[N];

// Variable para la visualización de muestras
float smoothS[N];

Coordenadas botonPR; // Coordenada para pausar/Reanudar audio
Coordenadas botonAV; // Coordenada para Aumentar volumen
Coordenadas botonDV; // Coordenada para Disminuir volumen
Coordenadas botonAT; // Coordenada para Aumentar Tono
Coordenadas botonDT; // Coordenada para Dismunuir Tono
Coordenadas botonRE;

Coordenadas vol; // Coordenada para dibujar volumen
Coordenadas ton; // Coordenada para dibujar tono
Coordenadas dur; // Coordenada para dibujar tiempo audio
Coordenadas val1; // Cordenada para dibujar primer indice de frecuencia
Coordenadas val2; // Coordenada para dibujar ultimo indice de frecuencia

int terminado=1; // booleano que determina si termino el audio

// Devuelve la amplitud en db
double amp(double complex z)
{
  return 20 * log10(cabs(z)+1);
} // amp


// Dibuja las frecuencias por la INterfaz
void DibujarFrecuencias(){
  
  double max_amp=1.0;
  int m=0;

  //Calcula la frecuencia maxima y los valores de amplitud en db y los alamcena en outLog
  for(int i=vFreq.inicio; i<vFreq.fin; i++){
    
    double freq=amp(outBuffer[i]); //se obtiene la frecuencia en db
    
    if(freq>max_amp) max_amp=freq;  // Se calcula la frecuencia maxima
    
    outLog[m++]=freq; // Se guardan los valores de frecuencia en db
  }
  
  // Se calcula el ancho que ocupara cada frecuencia en la interfaz
  int anchoFreq= ventana.ancho/ m;
  if(anchoFreq<1) anchoFreq=1;
   
  for(int i=vFreq.inicio; i<vFreq.fin; i++){
    double a=outLog[i]/max_amp; // Se normalizan las frecuencias
    outSmooth[i] += (a - outSmooth[i]) * vFreq.suavidad * vFreq.dft ; // Se suavizan las frecuencias
    double t = outSmooth[i]; // frecuencia a dibujar en t
    double hue = (double)i /m ; 
    Color color = ColorFromHSV(hue * 360, 1.0, 1.0); // calculamos el color de la frecuencia
    DrawRectangle(i * anchoFreq, ventana.alto - ventana.alto *2/3 * t, anchoFreq, ventana.alto *2/3 * t, color); // se dibujan la frecuencias
  }
  
}


// Crea la interfaz de usuario
void DibujarVentana(){

  SetTraceLogLevel(LOG_NONE);

  // Inicializa la Ventana
  InitWindow(ventana.ancho,ventana.alto,audio.nombreAudio);

  // Se obtienen las medidas maximas del monitor donde se este ejecutando la aplicación
  anchuraMax= GetMonitorWidth(0)-100;
  alturaMax = GetMonitorHeight(0)-100;

  // Se comprueba que las medidas de la ventena no sean mayor a las medidas maximas 
  if(anchuraMax < ventana.ancho){
    if(alturaMax < ventana.alto){
      printf("ENTROOO\n");
         SetWindowSize(anchuraMax, alturaMax);
	 ventana.alto= alturaMax;
	 ventana.ancho= anchuraMax;
    }else{
         SetWindowSize(anchuraMax, ventana.alto);
	 ventana.ancho= anchuraMax;
    }

  }else if(alturaMax < ventana.alto){
    SetWindowSize(ventana.ancho, alturaMax);
    ventana.alto= alturaMax;

  }
  
  
  // Mientras que la ventana no se cierre se dibuja
  while(!WindowShouldClose()){
    
    // Si se arrastra un nuevo audio se cambia la reprodución
    if(IsFileDropped()) CargarAudio();

    BeginDrawing();// Empezar a dibujar
    
    vFreq.dft=GetFrameTime(); // Se obtiene el tiempo que ha pasado desde la ultima iteración
    ventana.ancho=GetRenderWidth(); // se actualiza el ancho de la ventana
    ventana.alto=GetRenderHeight(); // se actualiza el alto de la ventana

    // Si el audio no ha terminado
   if(terminado==1){
     
    ClearBackground(WHITE);
    
    ActualizarCoordenadas(); // Se actualizan las coordenadas
    ChequearBotones();// Se chequea si se ha presionado un boton
    ChequearTeclado();// Se chequea si se ha presionado el teclado
    DibujarBotones();// Se dibujan los botones
    
    if(audio.spectogram==1) {
      DibujarFrecuencias(); // Se dibujan las frecuencias de audio
    }else{
      DibujarSpectogram();// Se dibujan las muestras de audio
    }
   }

   // Si ha acabado el sonido 
   if(SonidoTerminado()==0){
      ClearBackground(WHITE);
      DrawText("Arrastre para cargar otro Audio", (ventana.ancho/2) - 450, ventana.alto/2 -100 , 50, BLACK);
      terminado=0;
   }
   
    EndDrawing();

  }


  CloseWindow();

}


// Función para cambiar el audio que se este reproduciendo
void CargarAudio(){

  // Carga los path de los archivos de audio que fueron arrastrados
  FilePathList droppedFiles = LoadDroppedFiles();

  // No se pueden arrastrar mas de un audio
    if(droppedFiles.count != 1){
        fprintf(stderr, "%s No se puede arrastrar más de un archivo de audio a la vez\n", programa);
        if (droppedFiles.count > 0) UnloadDroppedFiles(droppedFiles);
        return;
    }

    // Se obtiene el path del archivo y se valida si es correcto
    char *fichero = droppedFiles.paths[0];
    if(!ValidarArchivo(fichero)){
        fprintf(stderr, "%s El archivo de audio %s no se puede leer\n", programa, fichero);
        UnloadDroppedFiles(droppedFiles);
        return;
    }
    
    // Se cambia el estado del programar para cargar un nuevo audio
    PrepararParaNuevoAudio();

    // Se obtienen los datos del nuevo fichero
    char *ficheroAudio = strdup(fichero);
    if( ObtenerDatosAudio(ficheroAudio)!=0 ) exit(1);
    int res;
    if(strcmp(audio.tipo, ".mp3") == 0 || strcmp(audio.tipo, ".wav") == 0 || strcmp(audio.tipo, ".flac")==0){
      res= ProcesarAudioWavMp3(); // Se procesa el audio
      if(res==1) {
	 UnloadDroppedFiles(droppedFiles);
	 CloseWindow();
	 exit(1);
      }else if(res==2){
	 UnloadDroppedFiles(droppedFiles);
	 FreeFFTW();
	 CerrarFreqRaw();
	 CloseWindow();
	 exit(1);
      }
      SetWindowTitle(audio.nombreAudio);
    } else if(strcmp(audio.tipo, ".raw") == 0){
      res= ProcesarAudioRaw();// Se procesa el audio
      if(res!=0){
	 UnloadDroppedFiles(droppedFiles);
	 CloseWindow();
	 exit(1);
      }
      SetWindowTitle(audio.nombreAudio);
    } else {
        fprintf(stderr, "%s Solo se permiten archivos de tipo: WAV, MP3, FLAC, RAW\n", programa);
        LiberarDatosAudio();
        UnloadDroppedFiles(droppedFiles);
        exit(1);
    }

     UnloadDroppedFiles(droppedFiles);
     if(audio.printInfo == 0) ImprimirInformacion(); // si esta activado se imprime la informacion del audio

    terminado=1;

}


// Incializa el sistema para cargar nuevo audio
void PrepararParaNuevoAudio(){
  // Si se estaba recortando alguno se cierra el recorte
    if(r.recortando == 0){
        r.recortando = 1;
        EjecutarFFTWForward(r.size, 1, r.muestras, 2);
        free(r.muestras);
        r.muestras = NULL;
        r.size = 0;
        if(r.version == 9) r.version = 0;
    }

    CerrarSound(); // Se cierra el sonido
    CerrarEngine();// Se cierra la engine
    CerrarDispositivo();// se cierra el dispositivo
    // Se destruye la FFT y se cierra el archivo de frecuencias 
    if(audio.spectogram == 1){
        FreeFFTW();
        CerrarFreqRaw();
    }
    // se inicializan los buffers a cero
    memset(buffer, 0, sizeof(buffer));
    memset(outBuffer, 0, sizeof(outBuffer));
    memset(outLog, 0, sizeof(outLog));
    memset(outSmooth, 0, sizeof(outSmooth));
    memset(smoothS, 0, sizeof(smoothS));
    buffer_index=0;
    
    LiberarDatosAudio();// Se libera la información del audio anterior
}

// Dibujar las muestras de audio
void DibujarSpectogram(){
  
  // Calcula el ancho de cada muestra
  float anchoMuestra= (float)ventana.ancho/tamanoBuffer;
  if(anchoMuestra <1) anchoMuestra=1;
  
  for(int i=0; i<tamanoBuffer; i++){
    
    float muestra= buffer[i]; // se obtiene la muestra a dibujar
    
    // Si es positiva se dibuja hacia arriba, de lo contrario se dibuja hacia abajo
    if(muestra>0){
      float t= muestra;
      DrawRectangle(i*anchoMuestra, ventana.alto/2 - ventana.alto/2 * t, 1,ventana.alto/2 * t, RED); 
     
    }else{
      float t= muestra;
      DrawRectangle(i*anchoMuestra, ventana.alto/2, 1, ventana.alto/2 * -t, RED);
    }

  }// for

}


void ActualizarCoordenadas(){

  // Coodenadas para Pausar y Renaudar el audio
    botonPR.x = ventana.ancho - (ventana.ancho-160);
    botonPR.y = 50;

  // Coordenadas botones de volumen +
     botonAV.x= ventana.ancho-160;
     botonAV.y = 50;

  // Coordenadas botones de volumen -
     botonDV.x= ventana.ancho -160;
     botonDV.y = 100;

  // Coordenadas boton de tono + 
     botonAT.x= ventana.ancho- 160;
     botonAT.y= 160;

  // Coodenadas boton de tono -
     botonDT.x= ventana.ancho - 160;
     botonDT.y= 210;
     
  // Coordenadas para dibujar volumen
     vol.x= ventana.ancho -80;
     vol.y=85;

  // Coodernadas para dibujar tono
     ton.x=ventana.ancho-80;
     ton.y=195;

  // Coordenadas para dibujar la duración del audio
     dur.x=ventana.ancho -(ventana.ancho -100);
     dur.y=60;
	
  // Coordenadas para dibujar el índice de inicio de visualización de frecuencia
     val1.x= ventana.ancho - (ventana.ancho- 10);
     val1.y= 10;

  // Coordenadas para dibujar el índice de fin de visualización de frecuencias
      val2.x= ventana.ancho - 60;
      val2.y=10;
	
   // Coordenadas para boton de recortar audio
      botonRE.x= ventana.ancho - (ventana.ancho - 350);
      botonRE.y= 50;
}



void ChequearBotones(){

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
	   //chequear si se quiere parar o renaudar musica
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){botonPR.x, botonPR.y, 150, 50}))
            {
	     
	       if(audio.pausado==0){
		audio.pausado=1;
		Renaudar();
	      }else{
		audio.pausado=0;
		Pausar();	
		}
	    //chequear si se quiere aumentar el volumen
	    }else if(CheckCollisionPointRec(GetMousePosition(), (Rectangle){botonAV.x,botonAV.y,60,45})){
                if (audio.volumen < 1.0){
                    audio.volumen += 0.1;
                    SetVolumen(audio.volumen);
                }
            //Chequear si se quiere disminuir el volumen
            }else if(CheckCollisionPointRec(GetMousePosition(), (Rectangle){botonDV.x,botonDV.y,60,45})){
                if (audio.volumen > 0.0){
                    audio.volumen -= 0.1;
                    SetVolumen(audio.volumen);
                }
	    //Chequer si se quiere aumentar el tono
            }else if(CheckCollisionPointRec(GetMousePosition(), (Rectangle){botonAT.x,botonAT.y,60,45})){
                    if(audio.tono < 6.0){
                        audio.tono += 0.1;
                        SetPitch(audio.tono);
                    }
	    //Chequear si se quiere disminuir el tono
            }else if(CheckCollisionPointRec(GetMousePosition(), (Rectangle){botonDT.x,botonDT.y,60,45})){
                if (audio.tono > 1.0){
                    audio.tono -= 0.1;
                    SetPitch(audio.tono);
                }
	   // Chequear si se quiere recortar el audio
            }else if( audio.spectogram==1 && CheckCollisionPointRec(GetMousePosition(), (Rectangle){botonRE.x, botonRE.y, 150, 50})){
	      if(r.recortando==1){
		r.recortando=0;
	      }else{
		r.recortando=1;
		EjecutarFFTWForward(r.size,1, r.muestras, 2);
		free(r.muestras);
		r.muestras=NULL;
		r.size=0;
		if(r.version==99) r.version=0;

	      }
	    }
	}
}

// Chequea si se ha preionado una tecla
void ChequearTeclado(){

  // Si se presiona la flecha hacia arriba se sube el tono
     if(IsKeyPressed(KEY_UP)){
       if(audio.tono<6.0){
            audio.tono+=0.1;
            SetPitch(audio.tono);
       }
 // Si se presiona la flecha hacia abajo se disminuye el tono
     }else if(IsKeyPressed(KEY_DOWN)){
            if(audio.tono>1.0){
                audio.tono-=0.1;
		SetPitch(audio.tono);
            }
 // Si se presiona la tecla "+" aumenta el volumen 
      }else if(IsKeyPressed(KEY_KP_ADD)){
            if (audio.volumen < 1.0){
                    audio.volumen += 0.1;
		    SetVolumen(audio.volumen);
                }
  // Si se presiona la tecla "-" disminuye el volumen
      }else if(IsKeyPressed(KEY_KP_SUBTRACT)){
            if (audio.volumen > 0.0){
                    audio.volumen -= 0.1;
                    SetVolumen(audio.volumen);
                }
  // Si se presiona enter se pausa o reanuda el audio
     }else if(IsKeyPressed(KEY_ENTER)){
	  if(audio.pausado==0){
		audio.pausado=1;
		Renaudar();
	      }else{
		audio.pausado=0;
		Pausar();	
	  }
     }
     
}//ChequearTeclado


void DibujarBotones(){

    char text[10];
    
    // Dibujar Botom para renaudar y pausar musica
    DrawRectangle(botonPR.x, botonPR.y, 150, 50, BLACK);
    if (audio.pausado==1){
         DrawText("Pausar", botonPR.x + 38, botonPR.y + 15, 20, WHITE);
     }else {
            DrawText("Reanudar", botonPR.x + 28, botonPR.y + 15, 20, WHITE);
        }

    if(audio.spectogram==1){
     DrawRectangle(botonRE.x, botonRE.y, 150, 50, BLACK);
     if(r.recortando==0){ DrawText("Recortando",botonRE.x + 20 ,botonRE.y+ 14,20, WHITE);}
     else { DrawText("Recortar",botonRE.x + 26 ,botonRE.y+ 14,20, WHITE);}
    }

     //Dibujar boton para aumentar volumen
      DrawRectangle(botonAV.x, botonAV.y, 60, 45, BLACK);
      DrawText("+",botonAV.x + 22 ,botonAV.y+ 9,30, WHITE);

        //Dibujar boton para disminuir volumen
        DrawRectangle(botonDV.x, botonDV.y, 60, 45, BLACK);
        DrawText("-",botonDV.x + 22 ,botonDV.y+ 9,30, WHITE);
	
        //Dibujar volumen
        sprintf(text, "%.1f", audio.volumen);        
        DrawText(text, vol.x, vol.y ,20, BLACK);

        //Dibujar boton para aumentar tono
        DrawRectangle(botonAT.x, botonAT.y, 60, 45, BLACK);
        DrawText("+",botonAT.x + 22 ,botonAT.y+ 9,30, WHITE);

        //Dibujar boton para disminuir tono
        DrawRectangle(botonDT.x, botonDT.y, 60, 45, BLACK);
        DrawText("-",botonDT.x + 22 ,botonDT.y+ 9,30, WHITE);

        //Dibujar tono
        sprintf(text, "%.1f", audio.tono);        
        DrawText(text, ton.x, ton.y ,20, BLACK);

    
       // Dibujar el tiempo de la canción
    ObtenerTiempo(); // Obtener tiempo de audio actual
    sprintf(text, "%.2f", audio.tiempo);
    DrawText(text, dur.x, dur.y, 20, BLACK);

    //Dibujar primer Indice Frecuencia
    sprintf(text, "%i", vFreq.inicio);
    if(audio.spectogram == 1) DrawText(text, val1.x, val2.y, 20, BLACK);

    //Dibujar ultimo Indice de FRecuencia
    sprintf(text, "%i", vFreq.fin);
    if(audio.spectogram == 1) DrawText(text, val2.x, val2.y, 20, BLACK);

}
