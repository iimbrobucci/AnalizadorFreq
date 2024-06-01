# AnalizadorFreq
Implementación de una Aplicación de Procesado Digital de la Señal para Analizar Frecuencias de Sonido (Audio) en Lenguaje C en Entorno GNU/Linux.

## Archivos soportados
- WAV
- MP3
- FLAC
- RAW

## Formato de Archivos de audio soportado
- Tamaño de muestra: 8, 16, 24, 32 bits y punto flotante.
- Número de canales: 1 o 2.

## Compilación y Ejecución
Descargue el código en una carpeta.  
En la carpeta "src" ejecute los siguientes comandos:
- **make**
- **sudo make install**
- **ejecutable -f fichero [opciones]**

## Uso del programa:

- **-f \<fichero\>**:
  - Especifica el fichero de audio. (Obligatorio)
  - Solo se permiten audios de tipo WAV, MP3, FLAC y RAW.
    - *Tipo*: WAV, MP3, FLAC, RAW.
    - *Formato*: 8 bits, 16 bits, 24 bits, 32 bits y punto flotante.
    - *Canales*: 1, 2.
  - Los archivos tipo MP3 y WAV serán codificados, reproducidos y mostrarán las frecuencias presentes en ellos a través de una interfaz.
  - Los archivos tipo RAW deben obtener solo valores de frecuencias de audio en formato 'double complex', a partir de ellos será posible crear un archivo de audio que luego se reproducirá y mostrará sus muestras a través de una interfaz.

- **-v \<volumen\>**:
  - Ajusta el volumen del audio (rango [0.0, 1.0]). (Por defecto 0.5).

- **-t \<tono\>**:
  - Ajusta el tono del audio (rango [1.0, 6.0]). (Por defecto 1.0).

- **-p**:
  - Imprime la información del audio (Formato, número de canales, tamaño de muestra, número total de frames).

- **-h \<alto\>**:
  - Especifica la altura de la ventana de interfaz de usuario (rango [600, 1400]). (Por defecto 800).

- **-w \<ancho\>**:
  - Especifica el ancho de la ventana de interfaz de usuario (rango [800, 2400]). (Por defecto 1200).

- **-n \<tamaño\>**:
  - Especifica el tamaño del buffer de procesamiento de muestras (rango [1024, 8192]).
  - Solo se permiten potencias de dos (Por defecto 8192).
  - Establece el tamaño del buffer que se usará para calcular la Transformada de Fourier de las muestras y, por ende, el número de muestras que serán representadas a la vez en la interfaz, es decir, las muestras serán procesadas en tamaños de ventana que pueden variar según (-n).

- **-i \<inicio\>**:
  - Especifica el índice de inicio de visualización de frecuencias (rango [0, 200]). (Por defecto 1.0).

- **-e \<fin\>**:
  - Especifica el índice de fin de visualización de frecuencias (rango [64, 4096]). (Por defecto 4096).
  - Si el índice de fin es mayor que la mitad del tamaño de la ventana (-n) se establecerá a ser igual al tamaño de la ventana/2. Si el índice de inicio es mayor que el índice de fin, el índice de inicio pasará a ser 1.

- **-q \<canal\>**:
  - Especifica el canal del audio que se quiere procesar (1 o 2). 1 corresponde al izquierdo y 2 al derecho. Por defecto 1.

- **-r**:
  - Recorta una trama del audio. (Si se habilita esta opción se debe especificar el rango a cortar (-a) (-l)).

- **-a \<inicio\>**:
  - Especifica el número de frame a partir de la cual se quiere empezar a recortar la trama (>= 0).

- **-l \<fin\>**:
  - Especifica el número de la última frame que se quiere recortar de la trama (>= 0).

- **-s \<frecuencia\>**:
  - Especifica la frecuencia de muestreo para la creación de audios (rango [8000, 96000]). (Por defecto 44100).

- **-c \<canales\>**:
  - Especifica el número de canales para la creación del audio (1 o 2). (Por defecto 1).

- **-b \<bloque\>**:
  - Especifica el tamaño de bloque al que se le debe aplicar la FFT en la creación de audio (rango [1024, 8192]). (Por defecto es el tamaño del archivo).

- **-m \<formato\>**:
  - Especifica el formato para la creación del audio (8, 16 o 32 bits). (Por defecto 32).

- **-d \<nombre\>**:
  - Activa el guardado de las frecuencias en el nombre del archivo que se pase.

- **-u**:
  - Muestra la información de uso del programa.

## Instalación de Dependencias

### Raylib
- Abrir terminal en máquina Linux
- Actualización de paquetes:
   - **sudo apt update**
- Instalación de dependencias:
  - **sudo apt install git build-essential cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxinerama-dev libxcursor-dev**
- Clonación del repositorio:
  - **git clone https://github.com/raysan5/raylib.git**
- Navegación al directorio de raylib:
  - **cd raylib**
- Configuración de la biblioteca:
  - **cmake .**
- Compilación e instalación:
  - **sudo make install**

### FFTW3
- Abrir terminal en máquina Linux.
- Actualización de paquetes:
  - **sudo apt update**
- Instalación de biblioteca:
  - **sudo apt install libfftw3-dev**

### GLFW
- **sudo apt-get install libglfw3-dev**

### PKG-CONFIG:
- **sudo apt-get install pkg-config**






