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
#define CosteAcierto 1
#define CosteFallo 20



// Struct que define linea de la caché
typedef struct{
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

// Variables Globales
int globaltime = 0; // Tiempo de accesos
int numFallos = 0;  // Numero de fallos de acceso a la memoria
const int TAM_MEMORIA_RAM = (1 << BITS_BUS); // Bytes (usando desplazamiento de bits)
T_CACHE_LINE NuestraCache[NUM_FILAS]; // Array de lineas de cache -> estructura de la cache

// Declaracion de funciones
  // funciones del enunciado del proyecto
  void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
  void VolcarCACHE(T_CACHE_LINE *tbl);
  void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
  void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

  // Función de testing
  void testingFuncionesAdd(); // Solo testing de funciones desarrolladas por Miguel
  // funciones adiccionales
  T_CACHE_LINE InicializarTCL();
  unsigned char *crearSimulRam();
  char *LeerDireccionMemoria(FILE *fptr);
  int contarFilasMem(); // No se usa
  char **crearMtrzDir(); // No se usa
  void printMtrzDir(char **dirMtrx); // No se usa
  void borrarMtrzDir(char **dirMtrx);  // no se usa

//Devuelve un struct T_CACHE_LINE inicializado con Etiqueta en xFF y datos en 0x23
T_CACHE_LINE InicializarTCL() { 
	T_CACHE_LINE res;

	res.ETQ = 0xFF; // inicializar etiqueta
	for(int i = 0; i < TAM_LINEA; i++) // inicializar linea
		res.Data[i] = 0x23;

	return res;
}

// Crea el array de chars Simul_RAM con el contenido de CONTENTS_RAM.bin
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

// devuelve el numero de filas que hay en accesos_memoria.txt
int contarFilasMem(){
  FILE * fptr = fopen(Accesos_Memoria, "r"); // abre el archivo
  if(fptr == NULL) // en caso de no encontrarse el archivo
    return -1;

  int cont = 0;
  char buffer[256];

  /* Contamos líneas leyendo con fgets, así manejamos correctamente
     líneas incluso si la última no termina en '\n'. */
  while(fgets(buffer, sizeof buffer, fptr) != NULL){
    cont++;
  }

  fclose(fptr);
  return cont;
}


// en caso de querer cargar todas las direccines de memoria
char** crearMtrzDir(){ // filas: saber el número de filas hay
  FILE *fptr = fopen(Accesos_Memoria, "r"); // abre el archivo
  if(fptr == NULL) // en caso de que no se encuentre el archivo
    return NULL;

  int filas = contarFilasMem();
  if(filas <= 0){
    fclose(fptr);
    return NULL;
  }

  char **dirMtrx = (char **)malloc((filas + 1) * sizeof(char *)); // +1 para NULL terminator
  if(dirMtrx == NULL){
    fclose(fptr);
    return NULL;
  }

  int cont = 0;
  for(int i = 0; i < filas; i++){
    char *dir = LeerDireccionMemoria(fptr);
    if(dir == NULL){
      break; // EOF inesperado
    }
    dirMtrx[cont++] = dir;
  }

  dirMtrx[cont] = NULL; // terminar la matriz
  fclose(fptr);
  return dirMtrx;
}

// imprime la matriz de direcciones que se le pase
void printMtrzDir(char ** dirMtrx){
  if(dirMtrx == NULL) return;

  printf("Número de filas: (según matriz)\n");
  for(int f = 0; dirMtrx[f] != NULL; f++){
    printf("%d: ", f);
    for(int c = 0; c < TAM_DIR_MEMORIA && dirMtrx[f][c] != '\0'; c++)
      printf("%c", dirMtrx[f][c]);
    printf("\n");
  }
}

// libera la memoria de la matriz de direcciones que se pase
void borrarMtrzDir(char **dirMtrx){
  if(dirMtrx == NULL) return;
  for(int f = 0; dirMtrx[f] != NULL; f++){
    free(dirMtrx[f]); // libera memoria de cada fila
  }
  free(dirMtrx); // libera el puntero a las filas
}

// Limpia/Inicializa la caché: ETQ = 0xFF, datos = 0x23
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
  if(tbl == NULL) return;
  for(int i = 0; i < NUM_FILAS; i++){
    tbl[i] = InicializarTCL();
  }
}

// Vuelca el contenido de la caché por pantalla y escribe CONTENTS_CACHE.bin
void VolcarCACHE(T_CACHE_LINE *tbl){
  if(tbl == NULL) return;

  printf("\n--- Volcado de la CACHE ---\n");
  for(int i = 0; i < NUM_FILAS; i++){
    printf("Linea %d  ETQ=0x%02X  ", i, tbl[i].ETQ);
    for(int b = TAM_LINEA - 1; b >= 0; b--){
      printf("%02X ", tbl[i].Data[b]);
    }
    printf("\n");
  }

  // Volcar binario: 128 bytes en el orden linea0.byte0 ... linea7.byte15
  FILE *fout = fopen("CONTENTS_CACHE.bin", "wb");
  if(fout != NULL){
    for(int i = 0; i < NUM_FILAS; i++){
      // escribir bytes en orden byte0..byte15
      fwrite(tbl[i].Data, 1, TAM_LINEA, fout);
    }
    fclose(fout);
  } else {
    fprintf(stderr, "Aviso: no se pudo crear CONTENTS_CACHE.bin\n");
  }
}

/* ParsearDireccion: descompone una direccion (12 bits) en campos:
   - palabra: offset dentro de la linea (4 bits)
   - linea: indice de fila en la cache (3 bits)
   - ETQ: etiqueta (5 bits)
   - bloque: numero de bloque (direccion >> 4)
*/
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque){
  if(ETQ == NULL || palabra == NULL || linea == NULL || bloque == NULL) return;
  // Limitamos la dirección a BITS_BUS bits para evitar que valores mayores corrompan la ETQ al parsear
  addr &= ((1u << BITS_BUS) - 1u);

  // parseos que vamos a aplicar en cada campo
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
  - cargar desde MRAM el bloque indicado en la linea de cache
  - actualizando ETQ, copiando TAM_LINEA bytes, e incrementando numFallos y aumentando globaltime
  (no se porque añadimos 20s al globaltime, copilot casi q me obliga a que lo ponga)
*/
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque){
  if(tbl == NULL) return;
  if(linea < 0 || linea >= NUM_FILAS) return;

  /* Contabilizamos el fallo y sumar el coste aunque no podamos leer MRAM. */
  numFallos++;
  globaltime += CosteFallo; // coste por traer el bloque desde memoria principal

  if(MRAM == NULL){
    fprintf(stderr, "TratarFallo: MRAM es NULL, no se pueden cargar datos para bloque %d\n", bloque);
    /* actualizar ETQ para reflejar reemplazo lógico y dejar la línea con ceros */
    for(unsigned int i = 0; i < TAM_LINEA; i++) tbl[linea].Data[i] = 0x00;
    tbl[linea].ETQ = (unsigned char)(ETQ & 0xFF);
    printf("Funcion TratarFallo --> linea %d ETQ=0x%02X (sin datos, t=%d, fallos=%d)\n",
           linea, tbl[linea].ETQ, globaltime, numFallos);
    return;
  }

  // Ahora vamos a copiar los datos desde MRAM a la cache
  unsigned int inicio = (unsigned int)bloque * (unsigned int)TAM_LINEA;
  unsigned int dispo = 0;
  // comprobar si hay suficientes bytes en MRAM desde inicio
  if(inicio < (unsigned int)TAM_MEMORIA_RAM)
    dispo = (unsigned int)TAM_MEMORIA_RAM - inicio;

  // determinar cuantos bytes copiar
  unsigned int tocopy = (dispo >= TAM_LINEA) ? TAM_LINEA : dispo;

  // copiar los bytes desde MRAM a la cache
  for(unsigned int i = 0; i < tocopy; i++){
    tbl[linea].Data[i] = (unsigned char)MRAM[inicio + i];
  }
  // si no se han copiado todos los bytes, rellenar con ceros
  if(tocopy < TAM_LINEA){
    for(unsigned int i = tocopy; i < TAM_LINEA; i++)
      tbl[linea].Data[i] = 0x00; // rellenar con ceros si faltan bytes
  }
  // actualizar la ETQ
  tbl[linea].ETQ = (unsigned char)(ETQ & 0xFF);

  // mostrar info de TratarFallo
  printf("Funcion TratarFallo --> linea %d cargada ETQ=0x%02X desde bloque %d (t=%d, fallos=%d)\n",linea, tbl[linea].ETQ, bloque, globaltime, numFallos);
  printf("//////////////////////////////////////////////////////////////////\n");

}

/* Implementacion de testingFuncionesAdd: agrupa los tests que antes estaban en main */
void testingFuncionesAdd(void){
    printf("////////////////////////////////////////////////////////////\n");
    printf("== Testing InicializarTCL ==\n");
    T_CACHE_LINE linea = InicializarTCL();
    printf("ETQ = 0x%02X\n", linea.ETQ);
    printf("Data: ");
    for(int i = 0; i < TAM_LINEA; i++)
        printf("%02X ", linea.Data[i]);
    printf("\n\n");

    printf("////////////////////////////////////////////////////////////\n");

    printf("== Testing crearSimulRam ==\n");
    unsigned char *ram = crearSimulRam();
    // Comprobación
    if(ram == NULL){
      fprintf(stderr, "ERROR: no se pudo leer CONTENTS_RAM.bin o alloc falló\n");
    } else {
      int mostrar = (TAM_MEMORIA_RAM < 32) ? TAM_MEMORIA_RAM : 32;
      printf("Primeros %d bytes de Simul_RAM:\n", mostrar);
      for(int i = 0; i < mostrar; i++){
        printf("En hexa --> %02X || ", ram[i]);
        printf("En char --> %c \n", ram[i]);
      }
      free(ram);
    }

    printf("////////////////////////////////////////////////////////////\n");
    printf("\n== Testing LeerDireccionMemoria ==\n");
    FILE *fdir = fopen(Accesos_Memoria, "r");
    if(fdir == NULL){
      fprintf(stderr, "Aviso: no se encontro 'accesos_memoria.txt'\n");
    } else {
      char *linea;
      int idx = 0;
      while((linea = LeerDireccionMemoria(fdir)) != NULL){
        printf("Linea %d: '%s'\n", idx++, linea);
        free(linea);
      }
      fclose(fdir);
    }

    printf("////////////////////////////////////////////////////////////\n");
    printf("\n== Testing contarFilasMem ==\n");
    int filas = contarFilasMem();
    if(filas == -1){
      fprintf(stderr, "Aviso: no se encontro 'accesos_memoria.txt'\n");
    } else {
      printf("Número de filas en 'accesos_memoria.txt': %d\n", filas);
    }

    printf("////////////////////////////////////////////////////////////\n");
    printf("\n== Testing de crearMtrzDir ==\n");
    char **m = crearMtrzDir();
    if(m == NULL){
      fprintf(stderr, "Aviso: crearMtrzDir devolvio NULL\n");
    } else {
      printMtrzDir(m);
      borrarMtrzDir(m);
    }

    printf("////////////////////////////////////////////////////////////\n");
    printf("\n== Testing ParsearDireccion ==\n");
    {
      unsigned int addr = 0xABC; /* ejemplo dentro de 12 bits */
      int ETQ, palabra, lineaIdx, bloque;
      ParsearDireccion(addr, &ETQ, &palabra, &lineaIdx, &bloque);
      printf("Addr 0x%03X -> ETQ=%d palabra=%d linea=%d bloque=%d\n", addr, ETQ, palabra, lineaIdx, bloque);
    }

    printf("////////////////////////////////////////////////////////////\n");
    printf("\n== Testing TratarFallo ==\n");
    {
      unsigned char *fakeRAM = (unsigned char*)malloc(TAM_MEMORIA_RAM);
      if(fakeRAM == NULL){
        fprintf(stderr, "Aviso: no se pudo alloc fakeRAM para test TratarFallo\n");
      } else {
        for(int i = 0; i < TAM_MEMORIA_RAM; i++)
          fakeRAM[i] = (unsigned char)(i & 0xFF);

        /* reset cache y ejecutar fallo de prueba */
        LimpiarCACHE(NuestraCache);
        int ETQ, palabra, lineaIdx, bloque;
        ParsearDireccion(0xABC, &ETQ, &palabra, &lineaIdx, &bloque);
        TratarFallo(NuestraCache, (char*)fakeRAM, ETQ, lineaIdx, bloque);

        printf("Linea %d ETQ=0x%02X Datos primeros bytes: ", lineaIdx, NuestraCache[lineaIdx].ETQ);
        for(int i = 0; i < TAM_LINEA; i++)
          printf("%02X ", NuestraCache[lineaIdx].Data[i]);
        printf("\n");

        free(fakeRAM);
      }
    }
}

/* Programa principal: integra el bucle de accesos, estadisticas y volcado final */
int main(void){
  // Inicializamos la caché
  LimpiarCACHE(NuestraCache);
  // Estadisticas
  int totalAccesses = 0;
  int hits = 0;
  numFallos = 0;
  globaltime = 0;

  // Variables para parsear dirección
  int ETQ= 0, palabra= 0, linea= 0, bloque = 0;


  //Cargamos la memoria principal simulada
  unsigned char *MRAM = crearSimulRam();
  if(MRAM == NULL){
    fprintf(stderr, "Aviso: no se pudo leer 'CONTENTS_RAM.bin'. Algunos fallos no se podrán simular.\n");
  }

  /* Abrir fichero de accesos */
  FILE *ficheroAccesos = fopen(Accesos_Memoria, "r");
  if(ficheroAccesos == NULL){
    fprintf(stderr, "ERROR: no se pudo abrir '%s'\n", Accesos_Memoria);
    if(MRAM) free(MRAM);
    return 1;
  }

  //Bucle de accesos --> leemos linea a linea con LeerDireccionMemoria
  char *dir;
  while((dir = LeerDireccionMemoria(ficheroAccesos)) != NULL){
    // IMPORTANTE --> parsear dirección hex a entero
    // convertimos el strng a unsigned int
    unsigned int addr = (unsigned int)strtoul(dir, NULL, 16);
    free(dir);
    // parseamos la direccion
    ParsearDireccion(addr, &ETQ, &palabra, &linea, &bloque);

    // Mostrar la info respecto al intento de acceso
    printf("Acceso Numero--> %d: addr=0x%03X -> ETQ=%d linea=%d palabra=%d bloque=%d\n",totalAccesses, addr, ETQ, linea, palabra, bloque);

    // Compobamios si hemos aciertado
    sleep(2); //retardo entre accesos
    if(NuestraCache[linea].ETQ == (unsigned char)ETQ){ // hit
      hits++;
      globaltime += CosteAcierto;
      printf(" -> HIT (t=%d)\n", globaltime);
      printf("//////////////////////////////////////////////////////////////////\n");
    } else { // Fallo
      printf(" -> MISS - Calling TratarFallo() ...\n");
      sleep(3); //retardo entre fallos
      TratarFallo(NuestraCache, (char*)MRAM, ETQ, linea, bloque);
    }

    totalAccesses++;

    // retardo entre accesos
    sleep(2);
  }

  fclose(ficheroAccesos);

  // Volcamos el estado final de la cache
  VolcarCACHE(NuestraCache);

  // Mostramos stats finales
  printf("\n--- Estadisticas de Accesos ---\n");
  printf("Accesos totales: %d\n", totalAccesses);
  printf("Aciertos: %d\n", hits);
  printf("Fallos: %d\n", numFallos);
  printf("Tiempo global: %d\n", globaltime);

  if(MRAM)
    free(MRAM);
    
  return 0;
}

