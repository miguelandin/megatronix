/*Autores :
Miguel Cabrera
Alejandro Mamán
*/

// Librerias
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Define's
#define TAM_LINEA 16 // bytes por linea
#define BITS_BUS 12 // bits del bus de direcciones
#define TAM_DIR_MEMORIA 3 // cifras hex que tiene una direccion de memoria
#define Contenido_RAM "CONTENTS_RAM.bin"
#define Accesos_Memoria "accesos_memoria.txt"
#define NUM_FILAS 8 // Numero de filas que tiene la cache
// valores de coste de acierto y fallo
#define CosteAcierto 1
#define CosteFallo 20



// Struct dado por el enunciado que define linea de la caché
typedef struct{
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

// Variables Globales dadas por el enunciado
int globaltime = 0; // Tiempo de accesos
int numFallos = 0;  // Numero de fallos de acceso a la memoria
const int TAM_MEMORIA_RAM = (1 << BITS_BUS); // Bytes (usando desplazamiento de bits)
T_CACHE_LINE NuestraCache[NUM_FILAS]; // Array de lineas de cache -> estructura de la cache

// Declaracion de funciones
  // funciones del enunciado
  void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
  void VolcarCACHE(T_CACHE_LINE *tbl);
  void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
  void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

  // funciones adiccionales usadas
  T_CACHE_LINE InicializarTCL();
  unsigned char *crearSimulRam();
  char *LeerDireccionMemoria(FILE *fptr);

// Definicion de funciones
//Devuelve un struct T_CACHE_LINE inicializado con Etiqueta en xFF y datos en 0x23
T_CACHE_LINE InicializarTCL() { 
	T_CACHE_LINE res;

	res.ETQ = 0xFF; // inicializar etiqueta
	for(int i = 0; i < TAM_LINEA; i++) // inicializar linea
		res.Data[i] = 0x23;

	return res;
}

// Crea el array de Simul_RAM con el contenido de CONTENTS_RAM.bin
unsigned char* crearSimulRam() {
  FILE *fptr = fopen(Contenido_RAM, "rb"); // abre el archivo en modo lectura BINARIA
  if(fptr == NULL) // en caso de que no se encuentre el archivo
    return NULL;

	int valorDelFichero; // char actual que se este leyendo
	int i = 0; // contador para el array
	unsigned char *Simul_RAM = (unsigned char*)malloc(TAM_MEMORIA_RAM * sizeof(unsigned char));

  // Comprobacion de malloc
  if(Simul_RAM == NULL) {
    fclose(fptr); // cerrar el archivo antes de salir
    return NULL; // en caso de error al asignar memoria
  }
  
	
	while((valorDelFichero = fgetc(fptr)) != EOF && i < TAM_MEMORIA_RAM){ // lee hasta que se llene la RAM o termine el archivo
		Simul_RAM[i++] = (unsigned char)valorDelFichero; // mete el contenido del archivo en el array parseandolo a char
	}
	
  fclose(fptr);
	return Simul_RAM;
}

// Devuelve la siguiente direccion de memoria de accesos_memoria.txt
char* LeerDireccionMemoria(FILE * fptr){ // fptr: puntero que lea el archivo
  if(fptr == NULL) // en caso de que no se encuentra el archivo
    return NULL;

  int caracter; // caracter actual que se este leyendo en int para detectar EOF
  int i = 0; // contador para el array
  char *direccionMemoria; // array que contendra la direccion de memoria de la linea actual

  caracter = fgetc(fptr); // lee el primer caracter

  // si se ha llegado al final del archivo
  if(caracter == EOF)
    direccionMemoria = NULL; // se devolvera null
  else{
    // reservar como `char` para coincidir con el tipo de retorno `char *`
    direccionMemoria = malloc((TAM_DIR_MEMORIA + 1) * sizeof(char));
    // COmprobacion de malloc
    if(direccionMemoria == NULL) {
      return NULL; // en caso de error al asignar memoria
    }

    while(caracter != '\n' && i < TAM_DIR_MEMORIA && caracter != EOF){ // hasta el final de linea o llege al machimo de la memoria o encuentre un EOF
      direccionMemoria[i++] = caracter; // mete la fila de direccion actual en el array
      caracter = fgetc(fptr);
    }
    direccionMemoria[i] = '\0'; // añadimos el \0 par indicar el finar de la direccion 
  }

  return direccionMemoria;
}

// Limpia/Inicializa la cache: ETQ = 0xFF // datos = 0x23
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
  // Si la tabla es NULL, no hacemos nada (por seguridad)
  if(tbl == NULL)
    return;
  // Inicializamos cada linea de la cache
  for(int i = 0; i < NUM_FILAS; i++){
    tbl[i] = InicializarTCL(); // llamando a la funcion que inicializa una linea
  }
}

// Vuelca el contenido de la cache por pantalla y escribe CONTENTS_CACHE.bin
void VolcarCACHE(T_CACHE_LINE *tbl){
  // Si la tabla es NULL, no hacemos nada (por seguridad)
  if(tbl == NULL)
    return;
  // Volcado por pantalla
  printf("\n--- Volcado de la CACHE ---\n");
  // Mostrar cada linea
  for(int i = 0; i < NUM_FILAS; i++){
    printf("Linea %d  ETQ=0x%02X  ", i, tbl[i].ETQ);
    // Mostrar bytes en orden
    for(int b = TAM_LINEA - 1; b >= 0; b--){
      printf("%02X ", tbl[i].Data[b]);
    }
    printf("\n");
  }

  // Volcar el binario -> 128 bytes en orden
  FILE *fout = fopen("CONTENTS_CACHE.bin", "wb");
  // Comprobacion de fopen
  if(fout != NULL){
    // escribir cada linea en orden
    for(int i = 0; i < NUM_FILAS; i++){
      fwrite(tbl[i].Data, 1, TAM_LINEA, fout);
    }
    fclose(fout);
  }
  else {
    // Error al abrir el archivo
    fprintf(stderr, "Aviso: no se pudo crear CONTENTS_CACHE.bin\n");
  }
}

/* ParsearDireccion: descompone una direccion de 12 bits en los campos:
   - palabra: offset dentro de la linea (4 bits)
   - linea: indice de fila en la cache (3 bits)
   - ETQ: etiqueta (5 bits)
   - bloque: numero de bloque (direccion >> 4)
*/
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque){
  // Comprobacion de punteros nulos (por segurida)
  if(ETQ == NULL || palabra == NULL || linea == NULL || bloque == NULL)
    return;

  // Limitamos la dirección a BITS_BUS bits para evitar que valores mayores corrompan la ETQ al parsear --> pista dada por Claude
  addr &= ((1u << BITS_BUS) - 1u); // recorta la dirección para dejar solo bits menos significativos 

  // Declramos como lo vamos a plaicar a cada campo
  unsigned int parseoPalabra = (1u << 4) - 1u; // 0xF
  unsigned int parseoLinea  = (1u << 3) - 1u; // 0x7
  unsigned int parseoETQ    = (1u << 5) - 1u; // 0x1F

  // Los aplicamos
  *palabra = addr & parseoPalabra;
  *linea   = (addr >> 4) & parseoLinea;
  *ETQ     = (addr >> (4 + 3)) & parseoETQ;
  *bloque  = addr >> 4; // bloque = direccion / TAM_LINEA
}


/* TratarFallo:
  - carga desde MRAM el bloque indicado en la linea de cache
  - Pasos:
    actualizando ETQ -> copiando TAM_LINEA bytes -> incrementando numFallos -> aumentando globaltime
*/
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque){
  // Comprobacion de punteros nulos (por segurida)
  if(tbl == NULL)
    return;
    // Comprobacion de rango de linea
  if(linea < 0 || linea >= NUM_FILAS)
    return;

  // Contamos el fallo
  numFallos++;

  // Comprobacion de MRAM
  if(MRAM == NULL){
    // No podemos cargar datos desde MRAM
    fprintf(stderr, "TratarFallo: MRAM es NULL, no se pueden cargar datos para bloque %d\n", bloque);

    // actualizamos la  ETQ para reflejar reemplazo y dejar la línea con ceros
    // Copylot me insistió en que pusiera unsigned int en el bucle for
    for(unsigned int i = 0; i < TAM_LINEA; i++){
      tbl[linea].Data[i] = 0x00;
    }
    // actualizamos la etiqueta --> aunque no haya datos
    tbl[linea].ETQ = (unsigned char)(ETQ & 0xFF); // aseguramos que ETQ es un byte

    // mostramos info del tratado del fallo
    printf("Funcion TratarFallo --> linea %d ETQ=0x%02X (sin datos, t=%d, fallos=%d)\n",linea, tbl[linea].ETQ, globaltime, numFallos);

    return;
  }

  // Ahora vamos a copiar los datos desde MRAM a la cache
  unsigned int inicio = (unsigned int)bloque * (unsigned int)TAM_LINEA; // direccion inicial en MRAM
  unsigned int dispo = 0; // bytes disponibles para copiar desde MRAM
  
  // Comprobamos si hay suficientes bytes en MRAM desde inicio
  if(inicio < (unsigned int)TAM_MEMORIA_RAM) // Si hay bytes disponibles
    dispo = (unsigned int)TAM_MEMORIA_RAM - inicio; // calculamos los bytes disponibles desde inicio
  
  // Determinamos cuantos deberiamos de copiar
  unsigned int tocopy = (dispo >= TAM_LINEA) ? TAM_LINEA : dispo;

  // Copiamos los bytes desde MRAM a la cache
  for(unsigned int i = 0; i < tocopy; i++){
    tbl[linea].Data[i] = (unsigned char)MRAM[inicio + i];
  }

  // Si no se han copiado TODOS los bytes, rellenar con ceros --> Idea desarrollada por Copilot
  if(tocopy < TAM_LINEA){
    for(unsigned int i = tocopy; i < TAM_LINEA; i++)
      tbl[linea].Data[i] = 0x00; // rellenar con ceros si faltan bytes
  }

  // actualizamos la ETQ
  tbl[linea].ETQ = (unsigned char)(ETQ & 0xFF);

    // mostramos info del tratado del fallo
  printf("Funcion TratarFallo --> linea %d cargada ETQ=0x%02X desde bloque %d (t=%d, fallos=%d)\n",linea, tbl[linea].ETQ, bloque, globaltime, numFallos);
  printf("//////////////////////////////////////////////////////////////////\n");

}
int main(void){
  // Inicializamos la cache
  LimpiarCACHE(NuestraCache);

  // Stats
  int totalAccesses = 0;
  int hits = 0;
  numFallos = 0;
  globaltime = 0;

  // Variables para parsear la direccion
  int ETQ= 0, palabra= 0, linea= 0, bloque = 0;


  // Cargamos la memoria principal simulada en MRAM
  unsigned char *MRAM = crearSimulRam();
  // Comprobacion de MRAM
  if(MRAM == NULL){
    fprintf(stderr, "Aviso: no se pudo leer 'CONTENTS_RAM.bin'\n");
    return -1;
  }

  // Abrimos el fichero con los accesos a memoria
  FILE *ficheroAccesos = fopen(Accesos_Memoria, "r");
  // Comprobacion de fopen
  if(ficheroAccesos == NULL){
    fprintf(stderr, "ERROR: no se pudo abrir '%s'\n", Accesos_Memoria);
    if(MRAM)
    free(MRAM);
    return -1;
  }

  //Bucle de accesos --> leemos linea a linea con LeerDireccionMemoria
  char *dir;
  // Mientras haya direcciones que leer...
  while((dir = LeerDireccionMemoria(ficheroAccesos)) != NULL){
    // IMPORTANTE --> parsear dirección hex a entero
    // convertimos el strng a unsigned int porque las direcciones son hexadecimales
    unsigned int addr = (unsigned int)strtoul(dir, NULL, 16); // Entero en base 16
    free(dir); // liberamos la direccion leida
    
    // Parseamos la direccion
    ParsearDireccion(addr, &ETQ, &palabra, &linea, &bloque);

    // Mostramos la info sobre el intento de acceso
    printf("Acceso Numero--> %d: addr=0x%03X -> ETQ=%d linea=%d palabra=%d bloque=%d\n",totalAccesses, addr, ETQ, linea, palabra, bloque);

    sleep(2); //retardo entre accesos
    // Vemos si hemos aciertado
    if(NuestraCache[linea].ETQ == (unsigned char)ETQ){ // hit
      hits++;
      globaltime += CosteAcierto;
      printf(" -> HIT (t=%d)\n", globaltime);
      printf("//////////////////////////////////////////////////////////////////\n");
    } else { // Miss
      globaltime += CosteFallo;
      printf(" -> MISS - Calling TratarFallo() ...\n");
      sleep(3); //retardo entre fallos
      TratarFallo(NuestraCache, (char*)MRAM, ETQ, linea, bloque);
    }

    totalAccesses++;
    // retardo entre accesos
    sleep(2);
  }
  // Cuando hayamos terminado de leer las direcciones, cerramos el fichero
  fclose(ficheroAccesos);

  // Volcamos el estado final de la cache
  VolcarCACHE(NuestraCache);

  // Mostramos stats finales
  printf("\n--- Estadisticas de Accesos ---\n");
  printf("Accesos totales: %d\n", totalAccesses);
  printf("Aciertos: %d\n", hits);
  printf("Fallos: %d\n", numFallos);
  printf("Tiempo global: %d\n", globaltime);
  printf("Tiempo medio por acceso: %.2f\n", (float)globaltime / (float)totalAccesses);

  // Liberamos la memoria de MRAM
  if(MRAM)
    free(MRAM);
    
  return 0;
}

