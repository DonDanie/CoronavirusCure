/*Trabajo INFI Daniel Rosa Danta*/
#define NC 	<valor numerico>
#define NF 	<valor numerico>
#define FC 	<valor numerico>
#define INCV 	<valor numerico>
#define VBALA 	<valor numerico>
#define MAXB 	<valor numerico>

/*Comando de juego*/
struct comando {
  int izquierda;	/*Puls. izquierda (0/1)*/
  int derecha;		/*Puls. derecha (0/1)*/
  int fuego;		/*Puls. de disparo (0/1)*/
};

/*Funciones (se suponen disponibles)*/
/*Hilo de control del blanco*/
void *h_blanco(void *p);

/*Hilo de control de cañon*/
/*Argumento: Periodo del ciclo en milisegundos (entero convertido a void *)*/
void *h_gun(void *p);

/*Crear bala en fila fil y columna col*/
/*Devuelve el identificador de l bala*/
int crear_bala(int fil,int col);

*/Destruir bala con identificador inala*/
void dest_bala(int ibala);

/*Mover bala con identificador ibala a la fila fil y columna col*/
void mover_bala(int ibala,int fil,int col);