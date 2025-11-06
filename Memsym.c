/*Autores : Miguel Cabrera
            Alejandro Mamán
*/
// Defines
#define TAM_LINEA 16 // bytes por linea

// Struct
typedef struct{
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

// Variables Globales
int globaltime = 0;
int numFallos = 0;

// Declaracion de funciones
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

int main(int argc, char *argv[]){
    // ARRANQUE DEL PROGRAMA
        // Inicializar los campos ETQ a 0xFF
        // TO-DO

        // Inicializar todos los bytes de datos de la cache a 0x23
        // TO-DO

        // leer el fichero binario CONTENTS_RAM.bin en la variable Simul_RAM (un array de 4026 unsigned char)
        // TO-DO

        // Abrir y leer el fichero de texto dirs_memoria.txt (lista de direcciones en hexadecimal UNA POR LINEA)
        // TO-DO

        // Si uno de los ficheros no existe devolver return(-1)

        // Logica del main
        /*

        1. MEMsym lee una dirección del fichero de accesos.
        
        2. Parsear la dirección en sus campos (ETQ, palabra, línea, bloque).
       
        3. Comprobar si la etiqueta (ETQ) de la dirección es igual al campo `ETQ` de la línea de la caché correspondiente.
        
        4. Si la etiqueta no coincide (fallo de caché):
        - Incrementar `numfallos`.
        - Imprimir una línea con el formato:
		 `T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X`.
		 (T es el instante actual `globaltime`).
         - Incrementar `globaltime` en 20.
         - Copiar el bloque correspondiente desde `Simul_RAM` a la línea de la caché.
         - Imprimir un mensaje indicando que se está cargando el bloque X en la línea Y.
         - Actualizar el campo `ETQ` y los 16 bytes de datos de la línea.
         
         5. Si la etiqueta coincide (acierto de caché):
         - Imprimir por pantalla:
		 `T: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X`.
         - Cada carácter leído se añade a una variable `texto` (array de hasta 100
		 caracteres; no es necesario usar memoria dinámica).

         6. Volcar el contenido de la caché por pantalla con el formato pedido en el
         enunciado. Los datos se imprimen de izquierda a derecha de mayor a menor
         peso: el byte más a la izquierda es el byte 15 de la línea y el de la
         derecha es el byte 0.
         
         7. Hacer `sleep(1)` antes de procesar la siguiente dirección.

        Al finalizar (cuando se hayan leído todas las direcciones) se imprimirá:
        - Número total de accesos.
        - Número de fallos.
        - Tiempo medio de acceso.

        Debajo, otro mensaje con el texto leído carácter a carácter desde la caché.

        Antes de salir, el programa volcará los contenidos de los 128 bytes de la caché
        (8 líneas × 16 bytes) a un fichero binario llamado `CONTENTS_CACHE.bin`. El
        formato de este fichero ha de respetar el orden: el byte 0 corresponde al byte 0
        de la línea 0 de la caché y el byte 127 corresponde al byte 15 de la línea 7.
        */

    return 0;
}