# Proyecto MEGATRONIX

**Autores del proyecto**: Alejandro Maman && Miguel Cabrera

---

## Resumen y valor de la práctica

Esta práctica vale un 30% de la nota final de la asignatura. Es necesario obtener
un 4 para poder aprobar esta materia.

En esta práctica se pide desarrollar un simulador de memoria caché de un sistema
ficticio de los años 80 llamado MEGATRONIX.

## Especificaciones del sistema

- Bus de memoria: 12 bits.
- Memoria física.
- Caché: 8 líneas (correspondencia directa).
- Tamaño de línea: 16 bytes por línea.

Con estos parámetros, cada dirección de memoria se interpreta en tres campos:
- palabra: 4 bits
- línea: 3 bits
- etiqueta: 5 bits

El desarrollador debe implementar el programa en C sobre Linux llamado `MEMsym`.

## Estructuras y variables principales

Se dispone de un array de 8 elementos del tipo `T_CACHE_LINE`, definido como:

```c
typedef struct {
		unsigned char ETQ;
		unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;
```

Variables globales obligatorias:
- `globaltime` (inicializada a 0)
- `numfallos` (inicializada a 0)

Inicialización al arranque:
- Inicializar los campos `ETQ` a `0xFF`.
- Inicializar todos los bytes de datos de la caché a `0x23`.
- Leer el fichero binario `CONTENTS_RAM.bin` en la variable `Simul_RAM`, que es
	un array de 4096 `unsigned char`.
- Abrir y leer el fichero de texto `dirs_memoria.txt` (lista de direcciones en
	hexadecimal, una por línea). En el enunciado aparece también el nombre
	`accesos_memoria.txt` para el fichero de accesos; asegúrese de usar el nombre
	correcto en su implementación o documentar la elección.

Si alguno de los ficheros no existe, el proceso debe notificarlo con un mensaje
apropiado y terminar con `return(-1)`.

## Protocolo de funcionamiento (por cada dirección leída)

1. MEMsym lee una dirección del fichero de accesos.
2. Parsear la dirección en sus campos (ETQ, palabra, línea, bloque).
3. Comprobar si la etiqueta (ETQ) de la dirección es igual al campo `ETQ` de la
	 línea de la caché correspondiente.
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

## Funciones obligatorias a implementar

Se deberán desarrollar obligatoriamente las siguientes funciones con los
prototipos indicados:

```c
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);
```

## Entrega

Cada equipo (obligatoriamente 2 alumnos salvo excepción) desarrollará el
proyecto en GitHub y presentará dos entregas:

1) Entrega a través de GitHub (privado) invitando al usuario facilitado por el
	 profesor. Ficheros a subir:
	 - `README.txt` con los nombres de los estudiantes.
	 - El fuente `MEMsym.c`.
	 - El volcado de la salida de la ejecución en `logcache.txt` (ejecutar
		 `./MEMsym > logcache.txt`).
	 - Debe haber commits de ambos integrantes demostrando participación.

2) Entrega en Blackboard que incluya todo lo anterior y además:
	 - Un PDF con: tabla de verificación de apartados de evaluación (si se
		 realizó cada apartado, problemas y cómo se solucionaron), los nombres de
		 los integrantes y una sección con los prompts utilizados con IA (si se
		 empleó), versión de la IA, respuestas y explicación en sus propias
		 palabras. Si se ha usado IA y no queda documentado, la práctica se
		 calificará con cero.
	 - Un enlace a un vídeo mp4 (máx. 10 minutos) donde se muestre la ejecución
		 del programa con explicación oral de ambos integrantes, el repositorio
		 GitHub y el PDF con la tabla de evaluación.

## Criterios de evaluación (resumen)

- Repositorio existe, es privado y profesor tiene acceso; programa compila;
	README con autores y entregas hechas (si no, penalización) — 1 punto.
- Arranque correcto, inicializaciones y control de errores al abrir
	`CONTENTS_RAM.bin` y `accesos_memoria.txt`/`dirs_memoria.txt` con
	`return(-1)` si no existen — 1 punto.
- Volcado por pantalla del contenido de la caché — 1 punto.
- Cálculo correcto de la línea de caché y comprobación de la etiqueta — 1 punto.
- Escritura por pantalla del byte seleccionado — 0.5 puntos.
- Uso correcto de `sleep(1)` — 0.5 puntos.
- Número de accesos, fallos, tiempo medio y texto leído — 1 punto.
- Terminación correcta tras leer última dirección y completar acciones — 0.5 puntos.
- Commits documentados que permiten trazar la participación — 1 punto.
- Escritura final `CONTENTS_CACHE.bin` correcta — 1 punto.
- Implementación de las 4 funciones obligatorias con los prototipos — 1 punto.
- Limpieza y documentación del código (constantes, comentarios, máscaras de
	bits) — 1 punto.
- Calidad y adecuación del vídeo (duración, claridad) — 1 punto.

> Nota: La evaluación se hará con ficheros `accesos_memoria.txt` y
> `CONTENTS_RAM.bin` diferentes de los proporcionados para desarrollo. No
> hardcodear salidas.

## Cómo compilar y ejecutar (ejemplo)

Compilar:

```bash
gcc -Wall -std=c99 -o MEMsym MEMsym.c
```

Ejecutar y guardar log:

```bash
./MEMsym > logcache.txt
```

El programa debe generar `CONTENTS_CACHE.bin` al finalizar.

## Archivos importantes

- `MEMsym.c`   : código fuente principal (debe incluir las funciones obligatorias).
- `CONTENTS_RAM.bin` : fichero binario con la memoria principal (4096 bytes).
- `dirs_memoria.txt` o `accesos_memoria.txt` : fichero de direcciones (hex, 1 por línea).
- `CONTENTS_CACHE.bin` : volcado binario de los 128 bytes de la caché generado por el programa.

## Siguientes pasos y sugerencias

- Implementar tests simples con distintos archivos de accesos para verificar
	fallos y aciertos.
- Añadir un pequeño script de comprobación que valide que `CONTENTS_CACHE.bin`
	tiene 128 bytes y que el formato es correcto.
- Documentar en el repositorio los commits de cada integrante para la entrega.

---

Para más detalles ver el enunciado original (`enunciado.pdf`).
