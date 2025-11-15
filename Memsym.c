/*Autores :
Miguel Cabrera
Alejandro Mamán
*/

/*Comentarios para el compañero
- Alex :
  - He hecho retoques en varias funciones
  - la mayoria son fallos tontos como que los arrays hay que añadirles el \0 apra poder detectar cuando termina dicho array
  - Comprobacciones de malloc que suele petar por x razones
  - en vez de usar pow usar desplazamiento de bits que tiene mas sentido y a Carlos seguro que le gusta más
  - Ademas a la hora de leer ficheros es mejor usar caracteres en formato int porque depende de donde lo ejecutes no te pilla en EOF
*/
//importaciones
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define's
#define TAM_LINEA 16 // bytes por linea
#define BITS_BUS 12 // bits del bus de direcciones
#define TAM_DIR_MEMORIA 3 // cifras hex que tiene una direccion de memoria

// Struct
typedef struct{
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

// Variables Globales
int globaltime = 0;
int numFallos = 0;
const int TAM_MEMORIA_RAM = (1 << BITS_BUS); // Bytes (Mejor usar desplazamiento de bits)
const int NUM_FILAS = 8; // Numero de filas que tiene la cache

// Declaracion de funciones
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

/*Devuelve un struct T_CACHE_LINE inicializado
con Etiqueta en xFF y datos en 0x23*/
T_CACHE_LINE InicializarTCL() { 
	T_CACHE_LINE res;

	res.ETQ = 0xFF; // inicializar etiqueta
	for(int i = 0; i < TAM_LINEA; i++) // inicializar linea
		res.Data[i] = 0x23;

	return res;
}

// Crea el array de chars Simul_RAM con el contenido de CONTENTS_RAM.bin
unsigned char* crearSimulRam() {
	FILE *fptr = fopen("CONTENTS_RAM.bin", "rb"); // abre el archivo en modo lectura BINARIA
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
    direccionMemoria = (unsigned char*)malloc((TAM_DIR_MEMORIA + 1) * sizeof(unsigned char)); // se le da una memoria
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
  FILE * fptr = fopen("accesos_memoria.txt", "r"); // abre el archivo
  if(fptr == NULL) // en caso de no encontrarse el archivo
    return -1;

  int cont = 0;
  int caracter; // caracter actual que se este leyendo en int para detectar EOF

  while((caracter = fgetc(fptr)) != EOF) // mientras no llegue al final
    if(caracter == '\n') // si es fin de linea
      cont++; // suma una linea
  
    if(caracter == EOF && cont > 0) // si el archivo no termina en salto de linea, suma una linea mas
      cont++;

  fclose(fptr);
  return cont;
}


// en caso de querer cargar todas las direccines de memoria
char** crearMtrzDir(){ // filas: saber el número de filas hay
  FILE *fptr = fopen("accesos_memoria.txt", "r"); // abre el archivo
  if(fptr == NULL) // en caso de que no se encuentre el archivo
    return NULL;

  char** dirMtrx; // matriz de direcciones que contiene todas las direcciones
  char * dir; // direccion actual
  int cont = 0; // contador para la matriz
  dirMtrx = (char **)malloc(contarFilasMem() * sizeof(char *)); // asigna memoria a la matriz

  if(dirMtrx == NULL) { // comprobacion de malloc
    fclose(fptr);
    return NULL; // en caso de error al asignar memoria
  }

  dir = LeerDireccionMemoria(fptr); // saca la nueva direccion de memoria
  while(dir != NULL){ // mientras no se llegue al final del archivo
    dirMtrx[cont] = dir; // añade la dir en la fila cont
    dir = LeerDireccionMemoria(fptr); // saca la nueva dir
    cont++;
  }

  fclose(fptr);
  return dirMtrx;
}

// imprime la matriz de direcciones que se le pase
void printMtrzDir(char ** dirMtrx){
  if(dirMtrx != NULL) {
    int filas = contarFilasMem();

    printf("Número de filas: %d\n", filas); // muestra el numero de filas tambien

    for(int f = 0; f < filas; f++){
      for(int c = 0; c < TAM_DIR_MEMORIA; c++)
        printf("%c", dirMtrx[f][c]);
        printf("\n");
    }
  }
}

// libera la memoria de la matriz de direcciones que se pase
void borrarMtrzDir(char **dirMtrx){
  for(int f = 0; dirMtrx[f] != NULL; f++)
    free(dirMtrx[f]); // libera memoria de cada fila

  free(dirMtrx); // libera el puntero a las filas
}

int main(void){
  /* Test básico de InicializarTCL y crearSimulRam
     - InicializarTCL: imprime etiqueta y los 16 bytes de la línea
     - crearSimulRam: intenta leer CONTENTS_RAM.bin y muestra los primeros bytes
  */
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
    return -1;
  }
  int mostrar = (TAM_MEMORIA_RAM < 32) ? TAM_MEMORIA_RAM : 32;
  printf("Primeros %d bytes de Simul_RAM:\n", mostrar);
  for(int i = 0; i < mostrar; i++){
    printf("En hexa --> %02X || ", ram[i]);
    printf("En char --> %c \n", ram[i]);
  }
  free(ram);
  printf("////////////////////////////////////////////////////////////\n");
  printf("\n== Testing LeerDireccionMemoria ==\n");
  FILE *fdir = fopen("accesos_memoria.txt", "r");
  if(fdir == NULL){
    fprintf(stderr, "Aviso: no se encontro'accesos_memoria.txt'\n");
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
  printMtrzDir(crearMtrzDir());
  return 0;
}