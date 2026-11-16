/*
 * ISWZ1102 - Programacion 1
 * Sistema de Planificacion de Produccion (SIPLAPRO)
 * Fabrica de componentes electronicos
 *
 * Calcula el tiempo total de fabricacion y los recursos necesarios para
 * cumplir la demanda de hasta 5 productos, y determina si la fabrica puede
 * cumplirla con el tiempo y los recursos disponibles.
 *
 * Restricciones respetadas:
 *  - Sin estructuras (struct) ni archivos: los datos viven en arreglos
 *    paralelos y se manipulan mediante punteros.
 *  - Busqueda, comparacion de cadenas y ordenamiento con logica propia
 *    (no se usan strcmp, strstr, qsort, bsearch, etc.).
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_PRODUCTOS 5
#define MAX_NOMBRE    40
#define MAX_LINEA     100

/* ---------- Prototipos ---------- */
/* Entrada de datos con validacion */
void leerLinea(char *buffer, int tam);
int  leerEntero(const char *mensaje, int minimo, int maximo);
double leerDecimal(const char *mensaje, double minimo);
void leerNombre(const char *mensaje, char *destino);

/* Utilidades de cadenas (logica propia) */
int  longitudCadena(const char *s);
void copiarCadena(char *destino, const char *origen);
char aMinuscula(char c);
int  sonIguales(const char *a, const char *b);

/* Gestion de productos */
int  buscarProducto(char nombres[][MAX_NOMBRE], int n, const char *buscado);
void registrarProducto(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                       double *recursos, int *n);
void listarProductos(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                     double *recursos, int n);
void editarProducto(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                    double *recursos, int n);
void eliminarProducto(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                      double *recursos, int *n);

/* Capacidad y calculos */
void configurarCapacidad(double *tiempoDisp, double *recursosDisp);
double calcularTiempoTotal(int *demanda, double *tiempo, int n);
double calcularRecursosTotales(int *demanda, double *recursos, int n);
void analizarFactibilidad(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                          double *recursos, int n, double tiempoDisp,
                          double recursosDisp);
void ordenarIndicesPorTiempo(int *indices, int *demanda, double *tiempo, int n);

void mostrarMenu(int n, double tiempoDisp, double recursosDisp);
void pausar(void);

/* ================================================================ */
int main(void)
{
    /* Arreglos paralelos: la posicion i describe al mismo producto */
    char   nombres[MAX_PRODUCTOS][MAX_NOMBRE];
    int    demanda[MAX_PRODUCTOS];     /* unidades demandadas            */
    double tiempo[MAX_PRODUCTOS];      /* horas por unidad               */
    double recursos[MAX_PRODUCTOS];    /* unidades de recurso por unidad */
    int    n = 0;                      /* productos registrados          */

    double tiempoDisp = 0.0;           /* horas disponibles en la linea  */
    double recursosDisp = 0.0;         /* recursos disponibles           */
    int opcion;

    do {
        mostrarMenu(n, tiempoDisp, recursosDisp);
        opcion = leerEntero("Seleccione una opcion: ", 0, 6);

        switch (opcion) {
        case 1:
            registrarProducto(nombres, demanda, tiempo, recursos, &n);
            break;
        case 2:
            listarProductos(nombres, demanda, tiempo, recursos, n);
            break;
        case 3:
            editarProducto(nombres, demanda, tiempo, recursos, n);
            break;
        case 4:
            eliminarProducto(nombres, demanda, tiempo, recursos, &n);
            break;
        case 5:
            configurarCapacidad(&tiempoDisp, &recursosDisp);
            break;
        case 6:
            analizarFactibilidad(nombres, demanda, tiempo, recursos, n,
                                 tiempoDisp, recursosDisp);
            break;
        case 0:
            printf("\nGracias por usar SIPLAPRO. Hasta pronto.\n");
            break;
        }
        if (opcion != 0) {
            pausar();
        }
    } while (opcion != 0);

    return 0;
}

/* ================================================================ */
/*                    ENTRADA DE DATOS VALIDADA                      */
/* ================================================================ */

/* Lee una linea completa y elimina el salto de linea final */
void leerLinea(char *buffer, int tam)
{
    int i = 0;
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
        if (i < tam - 1) {
            buffer[i] = (char)c;
            i++;
        }
    }
    buffer[i] = '\0';
    if (c == EOF && i == 0) {
        /* Fin de entrada: se termina el programa de forma ordenada */
        printf("\nFin de la entrada de datos.\n");
        exit(0);
    }
}

/* Solicita un entero dentro de [minimo, maximo] hasta que sea valido */
int leerEntero(const char *mensaje, int minimo, int maximo)
{
    char linea[MAX_LINEA];
    char *fin;
    long valor;

    while (1) {
        printf("%s", mensaje);
        leerLinea(linea, MAX_LINEA);
        valor = strtol(linea, &fin, 10);
        if (fin != linea && *fin == '\0' && valor >= minimo && valor <= maximo) {
            return (int)valor;
        }
        printf("  [!] Valor invalido. Ingrese un entero entre %d y %d.\n",
               minimo, maximo);
    }
}

/* Solicita un numero real mayor que 'minimo' */
double leerDecimal(const char *mensaje, double minimo)
{
    char linea[MAX_LINEA];
    char *fin;
    double valor;

    while (1) {
        printf("%s", mensaje);
        leerLinea(linea, MAX_LINEA);
        valor = strtod(linea, &fin);
        if (fin != linea && *fin == '\0' && valor > minimo) {
            return valor;
        }
        printf("  [!] Valor invalido. Ingrese un numero mayor que %.2f.\n", minimo);
    }
}

/* Lee un nombre no vacio */
void leerNombre(const char *mensaje, char *destino)
{
    do {
        printf("%s", mensaje);
        leerLinea(destino, MAX_NOMBRE);
        if (longitudCadena(destino) == 0) {
            printf("  [!] El nombre no puede estar vacio.\n");
        }
    } while (longitudCadena(destino) == 0);
}

/* ================================================================ */
/*                UTILIDADES DE CADENAS (LOGICA PROPIA)              */
/* ================================================================ */

int longitudCadena(const char *s)
{
    int len = 0;
    while (*(s + len) != '\0') {
        len++;
    }
    return len;
}

void copiarCadena(char *destino, const char *origen)
{
    while (*origen != '\0') {
        *destino = *origen;
        destino++;
        origen++;
    }
    *destino = '\0';
}

char aMinuscula(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return (char)(c + ('a' - 'A'));
    }
    return c;
}

/* Compara dos cadenas sin distinguir mayusculas: 1 si son iguales */
int sonIguales(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (aMinuscula(*a) != aMinuscula(*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* ================================================================ */
/*                      GESTION DE PRODUCTOS                         */
/* ================================================================ */

/* Busqueda secuencial propia: devuelve el indice o -1 si no existe */
int buscarProducto(char nombres[][MAX_NOMBRE], int n, const char *buscado)
{
    int i;
    for (i = 0; i < n; i++) {
        if (sonIguales(nombres[i], buscado)) {
            return i;
        }
    }
    return -1;
}

void registrarProducto(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                       double *recursos, int *n)
{
    char nombre[MAX_NOMBRE];

    printf("\n=== REGISTRAR PRODUCTO ===\n");
    if (*n >= MAX_PRODUCTOS) {
        printf("  [!] Ya se registraron los %d productos permitidos.\n",
               MAX_PRODUCTOS);
        return;
    }

    printf("Producto %d de %d\n", *n + 1, MAX_PRODUCTOS);
    leerNombre("Nombre del producto: ", nombre);
    while (buscarProducto(nombres, *n, nombre) != -1) {
        printf("  [!] Ya existe un producto con ese nombre.\n");
        leerNombre("Nombre del producto: ", nombre);
    }

    copiarCadena(nombres[*n], nombre);
    *(demanda + *n)  = leerEntero("Cantidad demandada (unidades): ", 1, 1000000);
    *(tiempo + *n)   = leerDecimal("Tiempo de fabricacion por unidad (horas): ", 0.0);
    *(recursos + *n) = leerDecimal("Recursos requeridos por unidad: ", 0.0);
    (*n)++;

    printf("  [OK] Producto registrado correctamente.\n");
}

void listarProductos(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                     double *recursos, int n)
{
    int i;

    printf("\n=== PRODUCTOS REGISTRADOS ===\n");
    if (n == 0) {
        printf("  No hay productos registrados.\n");
        return;
    }
    printf("%-3s %-20s %10s %12s %12s\n", "#", "Producto", "Demanda",
           "Horas/u", "Recursos/u");
    printf("-------------------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        printf("%-3d %-20s %10d %12.2f %12.2f\n", i + 1, nombres[i],
               *(demanda + i), *(tiempo + i), *(recursos + i));
    }
}

void editarProducto(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                    double *recursos, int n)
{
    char buscado[MAX_NOMBRE];
    char nuevo[MAX_NOMBRE];
    int pos, campo, otro;

    printf("\n=== EDITAR PRODUCTO ===\n");
    if (n == 0) {
        printf("  No hay productos para editar.\n");
        return;
    }

    leerNombre("Nombre del producto a editar: ", buscado);
    pos = buscarProducto(nombres, n, buscado);
    if (pos == -1) {
        printf("  [!] El producto \"%s\" no existe.\n", buscado);
        return;
    }

    printf("  Encontrado: %s | Demanda: %d | Horas/u: %.2f | Recursos/u: %.2f\n",
           nombres[pos], demanda[pos], tiempo[pos], recursos[pos]);
    printf("  1. Nombre\n  2. Cantidad demandada\n  3. Tiempo por unidad\n"
           "  4. Recursos por unidad\n  5. Todos los datos\n");
    campo = leerEntero("Dato a modificar: ", 1, 5);

    if (campo == 1 || campo == 5) {
        leerNombre("Nuevo nombre: ", nuevo);
        otro = buscarProducto(nombres, n, nuevo);
        while (otro != -1 && otro != pos) {
            printf("  [!] Ya existe otro producto con ese nombre.\n");
            leerNombre("Nuevo nombre: ", nuevo);
            otro = buscarProducto(nombres, n, nuevo);
        }
        copiarCadena(nombres[pos], nuevo);
    }
    if (campo == 2 || campo == 5) {
        *(demanda + pos) = leerEntero("Nueva cantidad demandada: ", 1, 1000000);
    }
    if (campo == 3 || campo == 5) {
        *(tiempo + pos) = leerDecimal("Nuevo tiempo por unidad (horas): ", 0.0);
    }
    if (campo == 4 || campo == 5) {
        *(recursos + pos) = leerDecimal("Nuevos recursos por unidad: ", 0.0);
    }
    printf("  [OK] Producto actualizado.\n");
}

void eliminarProducto(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                      double *recursos, int *n)
{
    char buscado[MAX_NOMBRE];
    int pos, i, confirmar;

    printf("\n=== ELIMINAR PRODUCTO ===\n");
    if (*n == 0) {
        printf("  No hay productos para eliminar.\n");
        return;
    }

    leerNombre("Nombre del producto a eliminar: ", buscado);
    pos = buscarProducto(nombres, *n, buscado);
    if (pos == -1) {
        printf("  [!] El producto \"%s\" no existe.\n", buscado);
        return;
    }

    printf("  Se eliminara: %s\n", nombres[pos]);
    confirmar = leerEntero("Confirmar (1 = Si, 0 = No): ", 0, 1);
    if (confirmar == 0) {
        printf("  Operacion cancelada.\n");
        return;
    }

    /* Desplaza los elementos posteriores una posicion a la izquierda
       en TODOS los arreglos paralelos para mantenerlos alineados */
    for (i = pos; i < *n - 1; i++) {
        copiarCadena(nombres[i], nombres[i + 1]);
        *(demanda + i)  = *(demanda + i + 1);
        *(tiempo + i)   = *(tiempo + i + 1);
        *(recursos + i) = *(recursos + i + 1);
    }
    (*n)--;
    printf("  [OK] Producto eliminado.\n");
}

/* ================================================================ */
/*                      CAPACIDAD Y CALCULOS                         */
/* ================================================================ */

void configurarCapacidad(double *tiempoDisp, double *recursosDisp)
{
    printf("\n=== CAPACIDAD DE LA LINEA DE PRODUCCION ===\n");
    *tiempoDisp   = leerDecimal("Tiempo de produccion disponible (horas): ", 0.0);
    *recursosDisp = leerDecimal("Recursos disponibles (unidades): ", 0.0);
    printf("  [OK] Capacidad configurada.\n");
}

/* T = sumatoria(demanda_i * tiempo_i) */
double calcularTiempoTotal(int *demanda, double *tiempo, int n)
{
    double total = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        total += *(demanda + i) * *(tiempo + i);
    }
    return total;
}

/* R = sumatoria(demanda_i * recursos_i) */
double calcularRecursosTotales(int *demanda, double *recursos, int n)
{
    double total = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        total += *(demanda + i) * *(recursos + i);
    }
    return total;
}

/* Ordenamiento burbuja propio (descendente) sobre un arreglo de indices,
   segun el tiempo total que consume cada producto. No altera los datos. */
void ordenarIndicesPorTiempo(int *indices, int *demanda, double *tiempo, int n)
{
    int i, j, aux, huboCambio;
    for (i = 0; i < n - 1; i++) {
        huboCambio = 0;
        for (j = 0; j < n - 1 - i; j++) {
            double tA = demanda[indices[j]] * tiempo[indices[j]];
            double tB = demanda[indices[j + 1]] * tiempo[indices[j + 1]];
            if (tA < tB) {
                aux = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = aux;
                huboCambio = 1;
            }
        }
        if (!huboCambio) {
            break;   /* ya esta ordenado */
        }
    }
}

void analizarFactibilidad(char nombres[][MAX_NOMBRE], int *demanda, double *tiempo,
                          double *recursos, int n, double tiempoDisp,
                          double recursosDisp)
{
    int indices[MAX_PRODUCTOS];
    double tiempoTotal, recursosTotales, factor, fT, fR;
    int i, k, cumpleT, cumpleR, sugerida;

    printf("\n=== ANALISIS DE PRODUCCION ===\n");
    if (n == 0) {
        printf("  [!] Registre al menos un producto (opcion 1).\n");
        return;
    }
    if (tiempoDisp <= 0.0 || recursosDisp <= 0.0) {
        printf("  [!] Configure primero la capacidad de la linea (opcion 5).\n");
        return;
    }

    tiempoTotal = calcularTiempoTotal(demanda, tiempo, n);
    recursosTotales = calcularRecursosTotales(demanda, recursos, n);

    /* Detalle por producto, ordenado de mayor a menor consumo de tiempo */
    for (i = 0; i < n; i++) {
        indices[i] = i;
    }
    ordenarIndicesPorTiempo(indices, demanda, tiempo, n);

    printf("%-20s %8s %12s %8s %12s\n", "Producto", "Demanda", "Horas", "%Tiempo",
           "Recursos");
    printf("-------------------------------------------------------------------\n");
    for (k = 0; k < n; k++) {
        i = indices[k];
        printf("%-20s %8d %12.2f %7.1f%% %12.2f\n", nombres[i], demanda[i],
               demanda[i] * tiempo[i],
               100.0 * demanda[i] * tiempo[i] / tiempoTotal,
               demanda[i] * recursos[i]);
    }
    printf("-------------------------------------------------------------------\n");

    /* 1. Tiempo total y 2. Recursos necesarios */
    printf("1) Tiempo total requerido : %12.2f h  | Disponible: %12.2f h\n",
           tiempoTotal, tiempoDisp);
    printf("2) Recursos necesarios    : %12.2f    | Disponibles: %11.2f\n",
           recursosTotales, recursosDisp);
    printf("   Uso de tiempo   : %6.1f%%\n", 100.0 * tiempoTotal / tiempoDisp);
    printf("   Uso de recursos : %6.1f%%\n", 100.0 * recursosTotales / recursosDisp);

    /* 3. Factibilidad */
    cumpleT = (tiempoTotal <= tiempoDisp);
    cumpleR = (recursosTotales <= recursosDisp);

    printf("3) Resultado: ");
    if (cumpleT && cumpleR) {
        printf("LA FABRICA SI PUEDE CUMPLIR CON LA DEMANDA.\n");
        printf("   Holgura de tiempo  : %.2f h\n", tiempoDisp - tiempoTotal);
        printf("   Holgura de recursos: %.2f\n", recursosDisp - recursosTotales);
        return;
    }

    printf("LA FABRICA NO PUEDE CUMPLIR CON LA DEMANDA.\n");
    if (!cumpleT) {
        printf("   - Faltan %.2f horas de produccion.\n", tiempoTotal - tiempoDisp);
    }
    if (!cumpleR) {
        printf("   - Faltan %.2f unidades de recursos.\n",
               recursosTotales - recursosDisp);
    }

    /* Produccion sugerida: se escala la demanda por el factor limitante
       f = min(Tdisp/Treq, Rdisp/Rreq) para no exceder ninguna capacidad */
    fT = tiempoDisp / tiempoTotal;
    fR = recursosDisp / recursosTotales;
    factor = (fT < fR) ? fT : fR;
    printf("   Restriccion limitante: %s\n", (fT < fR) ? "TIEMPO" : "RECURSOS");
    printf("   Se puede atender el %.1f%% de la demanda. Produccion sugerida:\n",
           100.0 * factor);
    for (i = 0; i < n; i++) {
        sugerida = (int)(demanda[i] * factor);   /* se trunca: unidades enteras */
        printf("     %-20s %8d de %d unidades\n", nombres[i], sugerida, demanda[i]);
    }
}

/* ================================================================ */
/*                          INTERFAZ                                 */
/* ================================================================ */

void mostrarMenu(int n, double tiempoDisp, double recursosDisp)
{
    printf("\n==============================================\n");
    printf("   SIPLAPRO - Planificacion de Produccion\n");
    printf("   Fabrica de Componentes Electronicos\n");
    printf("==============================================\n");
    printf(" Productos: %d/%d | Horas disp.: %.2f | Recursos disp.: %.2f\n",
           n, MAX_PRODUCTOS, tiempoDisp, recursosDisp);
    printf("----------------------------------------------\n");
    printf(" 1. Registrar producto\n");
    printf(" 2. Listar productos\n");
    printf(" 3. Editar producto\n");
    printf(" 4. Eliminar producto\n");
    printf(" 5. Configurar capacidad (tiempo y recursos)\n");
    printf(" 6. Calcular y analizar produccion\n");
    printf(" 0. Salir\n");
    printf("----------------------------------------------\n");
}

void pausar(void)
{
    char linea[MAX_LINEA];
    printf("\nPresione ENTER para continuar...");
    leerLinea(linea, MAX_LINEA);
}
