/*Proceso principal se trata de un proceso hijo generado por proceso_envio. Se encarga de leer por la cola de mensajes y ejecutar acciones dependiendo de que mensaje sea. 
 Se encarga ademas de ejecutar la parte gráfica del proyecto, la cual necesita de la instalacion previa de la libreria SDL del repositorio de Ubuntu.*/

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>/*Para muchas funciones como getchar,perror,etc*/
#include <termios.h>/*Para struct termios, funcion tcgetattr */
#include <unistd.h>/*Para API POSIX*/
#include <mqueue.h>/*Para usar colas de mensajes*/
#include <sys/stat.h> /*Para time_t*/
#include <sys/time.h> /*Para timeval*/
#include <sched.h>  /*Para pid_t time_t ...*/
#include <stdlib.h>/*Para atoi,rand...*/
#include <string.h>/*Para strings*/
#include <errno.h> /*Para usar errno*/
#include <pthread.h>/*para usar hilos*/
#include <fcntl.h> /* Para constantes O_* */
#include <stdbool.h>/*Para booleanos*/
#include <SDL/SDL.h>/*Para usar la API SDL*/
#include <math.h> /*Para sqrt, pow...*/
#include <signal.h>/*Para señales*/
#include <time.h>/*Para usar temporizadores*/
#include "procesos.h"/*Cabecera de nuestros procesos*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; //Condicion: vidas == 0

//Variables compartidas
static int INCV = 5;	//Pixeles por ciclo en incremento/decremento para velocidad de Jeringuilla
static int VVIRUS = 10 ;//Pixeles por ciclo en incremento/decremento para velocidad del Virus
static int VBALA = 12; 	//Pixeles por ciclo en incremento/decremento para velocidad de las balas
struct timespec nCiclo;	//tiempo de espera entre ciclos
int disparo = 0;	//Cantidad de balas que hay disparadas
int vidas;		//Numero de vidas restantes del virus
bool lee = 0;		//Indicador que usaremos para decidir cuando leer de la cola de mensajes
bool gameOver = 0;	//Indicador de fin de juego
struct dat_bala b_balas[MAXB];//datos de balas
struct dat_objeto o_jeringa , o_virus;//Datos de objetos
struct dat_capa c_fondo , c_portada , c_creditos , c_gOver , c_vida[MAXV];//Datos de capas

//Funciones
void *f_h_receptor(void *arg);//Funcion que lee en la cola de mensajes mandados por teclado
bool f_teclas(char *key , int *id , int *n_hiloscreados);//Funcion que maneja los mensajes leidos
void *f_h_jeringa();//Funcion de control de la jeringa
void *f_h_virus();//Funcion de control del virus
void *f_h_bala(void *n);//Funcion de control de balas
void  f_crear_bala(int n);//Funcion que pone las balas en juego (las "crea")
void  f_mover_bala(int n);//Funcion que actualiza la posicion de las balas
void  f_dest_bala(int n , bool impacto);//Funcion que elimina las balas
void *f_h_render();//Funcion que se encarga de renderizar las superficies en la ventana de juego
bool f_colision(int n);//Funcion que calcula distancia entre bala y virus determinando si ha habido impacto
void f_fin();	//Funcion manejador de la señal SIGRTMIN para terminar el programa por timeout
void f_envioOK ();//Funcion manejador de la señal SIGRTMAX para permitir lee de la cola de mensajes
void *f_h_cerrar();//Hilo que permite terminar el programa cuando pulsemos clickemos la "x" de la ventana

SDL_Surface* ventana = NULL;//Ventana de juego

int main(int argc, char **argv)
{//Hilo principal
  printf("SUERTE!\n");
  bool fin = 0;
  //Tratamiento de argumentos de entrada
  int cfin = atoi(argv[0]); 		//cfin--> numero de impactos necesarios [vidas] <=3
  int timout = atoi(argv[1]);		//timout--> tiempo maximo sin recibir caracteres [segundos]
  int ciclo = atoi(argv[2]);		//ciclo--> Periodo del ciclo [milisegundos] <=1000
  if (cfin >5)//Tratamiento de errores argumento 1
  {
    printf("Error de primer argumento de entrada. El máximo de vidas permitidas es %d.\n",MAXV);
    fflush(stdout);
    exit(2);
  }  
  vidas = cfin;
  if (ciclo > 1000)//Tratamiento de errores argumento 3
  {
    printf("Error de tercer argumento de entrada. El máximo milisegundos de ciclo es %d.\n",MAXC);
    fflush(stdout);	
    exit(3);
  }  
  nCiclo.tv_nsec = ciclo *1000000;//acciones por milisegundos que vamos a realizar *1000000 porque nosotros necesitamos nanosegundos para el tv_nsec.
  
  {  //Seccion grafica
  SDL_Init(SDL_INIT_EVERYTHING);//Inicializamos los graficos
  
  //Cargamos las imagenes
  c_portada.imagen = SDL_LoadBMP("portada.bmp");
  c_creditos.imagen = SDL_LoadBMP("creditos.bmp");
  c_fondo.imagen = SDL_LoadBMP("fondo.bmp");
  o_virus.imagen = SDL_LoadBMP("virus.bmp");
  o_jeringa.imagen = SDL_LoadBMP("jeringa.bmp");
  c_gOver.imagen = SDL_LoadBMP("Gover.bmp");
  
  //Modificamos las imagenes con mascaras de color
  SDL_SetColorKey (o_virus.imagen, SDL_SRCCOLORKEY,SDL_MapRGB(o_virus.imagen->format,mascara));
  SDL_SetColorKey (o_jeringa.imagen, SDL_SRCCOLORKEY,SDL_MapRGB(o_jeringa.imagen->format,mascara));
  SDL_SetColorKey (c_gOver.imagen, SDL_SRCCOLORKEY,SDL_MapRGB(c_gOver.imagen->format,mascara));

  //Cargamos valores iniciales 
  o_jeringa.objeto_ON = o_virus.objeto_ON = 0;
  o_virus.dim.w = o_virus.dim.h =120;	
  o_virus.dim.x = 200;
  o_virus.dim.y = 200;
  o_jeringa.dim.x = NC/2-50;
  o_jeringa.dim.y = FC;
  o_jeringa.vel = 0;
  o_jeringa.dim.w = 70;	
  o_jeringa.dim.h = 100;
  c_fondo.dim.x = c_fondo.dim.y = c_portada.dim.x = c_portada.dim.y = c_creditos.dim.x = c_creditos.dim.y = 0;
  c_gOver.dim.x = NC/2 - 170 ; 
  c_gOver.dim.y = NF/2 - 96;
  for(int j = 0; j < vidas; j++)
  {
    c_vida[j].dim.x = NC -100 -100*j;
    c_vida[j].dim.y = 10;
    c_vida[j].imagen = SDL_LoadBMP("corazon.bmp");
    SDL_SetColorKey (c_vida[j].imagen, SDL_SRCCOLORKEY,SDL_MapRGB(c_vida[j].imagen->format,mascara));
  }
  for(int i = 0; i < MAXB; i++)
  {
    b_balas[i].dim_ini.x = 5 + 20*i;
    b_balas[i].dim_ini.y = 10;
    b_balas[i].dim.x = b_balas[i].dim_ini.x;
    b_balas[i].dim.y = b_balas[i].dim_ini.y;
    b_balas[i].dim.w = 15;//Anchos de las balas
    b_balas[i].dim.h =30;//Largos de las balas
    b_balas[i].bala_ON = 0;	//Estados iniciales de las balas
    b_balas[i].imagen = SDL_LoadBMP("gota.bmp");
    SDL_SetColorKey ( b_balas[i].imagen , SDL_SRCCOLORKEY,SDL_MapRGB( b_balas[i].imagen ->format,mascara));
  }
  ventana = SDL_SetVideoMode(NC,NF,32, SDL_SWSURFACE);//Creamos nuestra ventana grafica 
  SDL_WM_SetCaption("CORONARIUS CURE","CORONAVIRUS CURE");//Nombre de la ventana y del proceso
  SDL_BlitSurface(c_portada.imagen, NULL, ventana, &c_portada.dim);//Cargamos imagen inicial
  SDL_Flip (ventana);	//Volcamos los buffers de pantalla
  }
  {  //Seccion señales
  sigset_t conjunto;//Creamos un conjunto de señales
  sigemptyset (&conjunto); //Eliminamos todas las señales del conjunto
  sigaddset (&conjunto, SIGRTMIN);//Añadimos la señal SIGRTMIN al conjunto
  sigaddset (&conjunto, SIGRTMAX);//Añadimos la señal SIGRTMAX al conjunto
  sigprocmask (SIG_BLOCK, &conjunto, NULL); //Mascara bloqueo de señales
  
  struct sigaction actuador1;      //actuador de señal SIGRTMIN  
  actuador1.sa_sigaction = f_fin; //Manejador de la señal SIGRTMIN
  sigemptyset(&(actuador1.sa_mask));
  actuador1.sa_flags = SA_SIGINFO;//Para señales POSIX 1003.1b
  sigaction(SIGRTMIN, &actuador1, NULL);//accion de la señal
  
  struct sigaction actuador2;  //actuador de señal SIGRTMAX
  actuador2.sa_sigaction = f_envioOK; //Manejador de la señal SIGRTMAX
  sigemptyset(&(actuador2.sa_mask));
  actuador2.sa_flags = SA_SIGINFO;
  sigaction(SIGRTMAX, &actuador2, NULL);
  }
  
  //Abrimos hilos
  pthread_t h_receptor; //Creamos hilo receptor
  pthread_create(&h_receptor, NULL, *f_h_receptor, (void*)timout);
  
  pthread_t h_cerrar; //Creamos hilo cerrar
  pthread_create(&h_cerrar, NULL, *f_h_cerrar, (void*)timout);
  
  pthread_t h_jeringa;//Creamos el hilo de control de jeringa cuando se nos de permiso pulsando "P"
  while (!o_jeringa.objeto_ON){nanosleep(&nCiclo,NULL);}
  pthread_create(&h_jeringa, NULL, *f_h_jeringa, NULL);
  
  pthread_t h_virus;//Creamos el hilo de control del virus cuando se nos de permiso pulsando "S"
  while (!o_virus.objeto_ON){nanosleep(&nCiclo,NULL);}
  pthread_create(&h_virus, NULL, *f_h_virus, NULL);
  
  pthread_t h_render;//Creamos hilo de renderizado
  pthread_create (&h_render, NULL, *f_h_render, NULL);
  
  pthread_mutex_lock (&mutex);//Bloqueamos el mutex para usar la variable de condicion
  while (vidas != 0 && !gameOver)//Pintamos las posiciones mientras queden vidas
  {
    pthread_cond_wait (&cond, &mutex);//Esperamos a que desbloqueen el cond wait
  }
  pthread_mutex_unlock (&mutex);
  
  //Terminacion de juego
  {
  while(gameOver == 1 && fin == 0)//Bucle que espera a que los hilos de balas terminen
  {
    //Esperamos que se cierren los hilos de control de balas si los hubiera
    if (!b_balas[0].bala_ON && !b_balas[1].bala_ON && !b_balas[2].bala_ON)
    {
      fin = 1;
    }
    printf("Esperando que hilos terminen...\n");
    fflush(stdout);
    nanosleep(&nCiclo,NULL); 
  }
  //Cancelamos los hilos restantes
  if(pthread_cancel (h_render) == 0){printf("Hilo de renderizado cancelado\n");}
  if(pthread_cancel (h_cerrar) == 0){printf("Hilo de cerrado por ventana cancelado\n");}
  
  printf("Juego terminadondo en:\n");
  fflush(stdout);

  for (int i = 3; i >= 0; i --)//Cuenta atras de 3 segundos para terminar para que se puedan leer los creditos
  {
    SDL_BlitSurface(c_creditos.imagen, NULL, ventana, &c_creditos.dim);
    SDL_Flip (ventana);
    sleep(1);
    printf("%d\n",i);
    fflush(stdout);
  }
  printf("Gracias por jugar a Coronavirus Cure!\n");
  fflush(stdout);
  }
  mq_unlink (NombreCola);//Eliminamos la cola de mensajes
  exit(0);//Devolvemos 0 a proceso padre
}

void *f_h_receptor(void *timout)
{//Funcion de hilo receptor
  //Variables locales
  int id = 0;			//Identificador de bala que se va a tratar
  int n_hiloscreados = 0;	//Cantidad de hilos de control de bala que se han creado en una ejecucion
  int tout = (int)timout;	//Tiempo para el temporizador
  mqd_t mqdes; 			//descriptor de cola de mensaje
  char buf[tamMsg]; 		//buffer de dimension de caracteres tamMsg
  int  errnum;
  errnum = errno;
  bool tecla = 0;  
  
  //Temporizador que salta una vez a los tout segundos
  struct timespec tiempo = {tout , 0};	//Tiempo{segundos,nanosegundos};
  struct timespec inter = {0 , 0};	//Tiempo{segundos,nanosegundos};
  struct itimerspec tempor; //Estructura que meteremos al temporizador
  tempor.it_value = tiempo;//Tiempo hasta primer salto
  tempor.it_interval = inter;//Intervalo de temporizador
  struct sigevent temp_signal;
  temp_signal.sigev_notify = SIGEV_SIGNAL;
  temp_signal.sigev_signo = SIGRTMIN;//Señal que activará el temporizador
  timer_t tempo;//Temporizador
  timer_create (CLOCK_REALTIME, &temp_signal, &tempo); //Creamos el temporizador
  struct itimerspec queda;//Estructura donde guardaremos la situacion del temporizador
  
  while(!gameOver)
  {//Bucle de f_h_receptor
    printf("Hijo-->Esperando señal de tecla pulsada\n------------------------------------\n");
    fflush(stdout);
    if (o_virus.objeto_ON == 1)
    {
      printf("Iniciando cuenta atras...\n");
      fflush(stdout);
      timer_settime(tempo, 0, &tempor, NULL);//Iniciamos temporizador
      fflush(stdout);
    }
    fflush(stdout);
    while (!lee){//Hasta que no llegue un dato y tengamos que leer
      if (o_jeringa.objeto_ON && o_virus.objeto_ON && !gameOver)
      {
	timer_gettime (tempo, &queda);//Mostramos el tiempo restante del temporizador
	printf("Quedan %d segundos para que la pandemia sea inevitable!\r",queda.it_value.tv_sec);
	fflush(stdout);
      }
      nanosleep(&nCiclo,NULL); 
    }
    fflush(stdout);
    mqdes = mq_open(NombreCola, O_RDONLY);
    if (mqdes == -1)
    {   //Si falla
      printf("No se ha creado la cola de mensajes adecuadamente desde el receptor.\nValor del descriptor=%d \n",mqdes);
      fprintf(stderr, "Valor de errno: %d\n",errno);
      perror("Errror impreso por perror");
      fprintf(stderr ,"Error: %s\n",strerror(errnum));
      fflush(stdout);
    }
    mq_receive(mqdes, buf, tamMsg, NULL);
    mq_close(mqdes);
    pthread_mutex_lock (&mutex);
    tecla = f_teclas(buf,&id,&n_hiloscreados);//Llamamos a gestionador de teclas que nos devuelve si se ha pulsado o no una tecla valida
    if(tecla == 1 && o_virus.objeto_ON == 1)
    {
      timer_gettime(tempo, NULL);//Paramos el temporizador porque hemos pulsado tecla
      tecla = 0;
    }
    lee = 0;
    pthread_mutex_unlock (&mutex);
  }
  timer_delete (tempo); //Destruimos el temporizador
  pthread_join(b_balas[0].nombreHilo,NULL);//Esperamos a que terminen los hilos de bala
  pthread_join(b_balas[1].nombreHilo,NULL);
  pthread_join(b_balas[2].nombreHilo,NULL);
  pthread_detach (pthread_self());//Liberamos el espacio que puediera quedar del hilo y lo cerramos
}

bool f_teclas(char *key , int *id , int *n_hiloscreados)
{//Funcion que gestiona las teclas pulsadas
  bool teclaPulsada;
  int nKey = atoi(key);//La gestionaremos en int
  printf("Hijo-->Tecla leida: %c\nHijo-->Mensaje recibido %s\n",nKey,key);
  fflush(stdout);
  
  {//Logica para manejar tecla pulsada
  if (!o_jeringa.objeto_ON)//Si no esta activada la jeringa 
  {
    if ( nKey == 80 || nKey == 112 )
    {//Si pulsamos tecla P (play)
      o_jeringa.objeto_ON = 1;//Activamos jeringa
    }
    else if ( nKey == 48)
      {//Si pulsamos tecla 0 (virus)
	printf("Juego terminado por pulsar tecla %c.\n",nKey);
	fflush(stdout);
	o_jeringa.objeto_ON = 1;//Activamos jeringa
	o_virus.objeto_ON = 1;//Activamos virus
	gameOver = 1;//Finalizamos hilos
	vidas = 0;//Eliminamos las vidas
	pthread_cond_broadcast (&cond);
      }
    else
    {
      printf("Tecla incorrecta.\nPulse P (Play) para comenzar partida.\n");
      fflush(stdout);
    }
  }
  else//Si ya esta activada la jeringa
  {
    if (!o_virus.objeto_ON)//Si no esta activado el virus
    {
      if ( nKey == 83 || nKey == 115 )
      {//Si pulsamos tecla S 
	o_virus.objeto_ON = 1;
      }
      else if ( nKey == 48)
      {//Si pulsamos tecla 0 (virus)
	printf("Juego terminado por pulsar tecla %c.\n",nKey);
	fflush(stdout);
	gameOver = 1;
	vidas = 0;
	pthread_cond_broadcast (&cond);
      }
       else
      {
	printf("Tecla incorrecta.\nPulse S (Start) para comenzar una pandemia.\n");
	fflush(stdout);
      }
    }
    else//Si ya esta activado el virus
    {
      if ( nKey == 65 || nKey == 97 )
      {//Si pulsamos tecla A
	if(o_jeringa.vel > -vel_max)
	{
	  o_jeringa.vel -= INCV;//Velocidad negativa
	}   
      }
      else if ( nKey == 68 || nKey == 100 )
      {//Si pulsamos tecla D
	if(o_jeringa.vel < vel_max)
	{
	  o_jeringa.vel += INCV;//Velocidad positiva
	}
      }
      else if ( nKey == 87 || nKey == 119 )
      {//Si pulsamos tecla W
	if (disparo <=MAXB -1)
	{
	  //Iniciamos hilo de bala
	  *id = *n_hiloscreados %3;//Vemos el id de la bala que toca disparar
	  *n_hiloscreados += 1;//Actualizamos el numero de balas disparadas
	  pthread_create(&b_balas[*id].nombreHilo, NULL, *f_h_bala, (void*)*id);//Creamos hilo de bala
	  disparo += 1;//Sumamos un disparo activo
	}
      }
      else if ( nKey == 48)
      {//Si pulsamos tecla 0 (virus) 
	printf("Juego terminado por pulsar tecla %c.\n",nKey);
	fflush(stdout);
	gameOver = 1;
	vidas = 0;
	pthread_cond_broadcast (&cond);
      }
      else
      {
	printf("Tecla incorrecta.\n");
	fflush(stdout);
      }
    }
  }
  teclaPulsada = 1;//Activamos tecla pulsada
  }
return teclaPulsada;
}

void *f_h_jeringa ()
{//Funcion de hilo de contro de jeringa
  //Cargamos imagenes principales
  SDL_BlitSurface(c_fondo.imagen, NULL, ventana, &c_fondo.dim);
  SDL_BlitSurface(o_jeringa.imagen, NULL, ventana, &o_jeringa.dim);
  SDL_Flip (ventana);
  
  while(!gameOver)
  {//Bucle de f_h_jeringa
    if (o_jeringa.vel != 0)
    {
      if(o_jeringa.vel >0 && o_jeringa.vel >= NC - o_jeringa.dim.x - o_jeringa.dim.w )//Si chocamos con pared derecha
      {
	pthread_mutex_lock (&mutex);
	o_jeringa.dim.x = NC - o_jeringa.dim.w;//Nos posicionamos en posicion mas pegada a la derecha
	o_jeringa.vel = 0;//Ponemos velocidad a cero
	pthread_mutex_unlock (&mutex);
      }
      else if (o_jeringa.vel < 0 && o_jeringa.dim.x <= fabs(o_jeringa.vel))//Si chocamos con pared izquierda
      {
	pthread_mutex_lock (&mutex);
	o_jeringa.dim.x = 0; //Nos posicionamos en posicion mas pegada a la izquierda
	o_jeringa.vel = 0;//Ponemos velocidad a cero
	pthread_mutex_unlock (&mutex);
      }
      else//Si podemos movernos
      {
	pthread_mutex_lock (&mutex);
	o_jeringa.dim.x = o_jeringa.dim.x + o_jeringa.vel; //Actualizamos la posicion en funcion de la velocidad 
	pthread_mutex_unlock (&mutex);
      }
    }
    nanosleep(&nCiclo,NULL);
  }
  pthread_detach (pthread_self());//Liberamos espacio que pudiera dejar el hilo
}

void *f_h_virus ()
{//Funcion del hilo de control del virus;
  while(!gameOver)
  {
    int m_virus=rand()%2;//Variable aleatoria para ver si se movera a la izquierda o a la derecha 
		  
    if(m_virus == 0)//Se mueve a la izquierda
    {
      while (o_virus.dim.x >= 120)//Hasta que se choque con la pared izquierda
      {
	pthread_mutex_lock (&mutex);
	o_virus.dim.x = o_virus.dim.x - VVIRUS;//Actualizamos posicion
	pthread_mutex_unlock (&mutex);
	nanosleep(&nCiclo,NULL);
      }
    }
    else//Se mueve a la derecha
    {
      while (o_virus.dim.x <= NC - 120)//Hasta que se choque con la pared derecha
      {
	pthread_mutex_lock (&mutex);
	o_virus.dim.x = o_virus.dim.x + VVIRUS;//Actualizamos posicion
	pthread_mutex_unlock (&mutex);
	nanosleep(&nCiclo,NULL);
      }
    }
  }
  pthread_detach (pthread_self());//Liberamos espacio que pudiera dejar el hilo
}

void *f_h_bala(void *id)
{//Funcion de hilo de control de bala
  bool impacto;
  int n = (int)id;
  pthread_mutex_lock (&mutex);
  f_crear_bala(n);//Llamamos a crear bala con su id
  pthread_mutex_unlock (&mutex);
  while (b_balas[n].dim.y >= b_balas[n].dim.h &&  impacto == 0) //Mientras no se salga ni impacte
  {//Bucle de f_h_bala
    pthread_mutex_lock (&mutex);
    f_mover_bala(n);//Llamamos a mover bala con su id
    pthread_mutex_unlock (&mutex);
    impacto = f_colision(n);//Llamamos a colisiones con el id de la bala
    nanosleep(&nCiclo,NULL);
  }
  f_dest_bala(n, impacto);//Llamamos a destruir bala con su id
  return (void*)n;
}

void f_crear_bala (int n)
{//Funcion que crea una bala
  b_balas[n].dim.x = o_jeringa.dim.x;//Ponemos la posicion de la jeringa en x
  b_balas[n].dim.y = o_jeringa.dim.y - b_balas[n].dim.h;//Ponemos la posicion de la jeringa en y desplazada
  b_balas[n].bala_ON = 1;//Activamos bala
}

void  f_mover_bala (int n)
{//Funcion que actualiza posicion de bala
    b_balas[n].dim.y = b_balas[n].dim.y - VBALA;//Actualizamos posicion de bala segun su velocidad
}

void f_dest_bala (int n, bool impacto)
{//Funcion que destruye la bala
  pthread_mutex_lock (&mutex);
  if (impacto == 1)//Si ha impactado con el virus
  {
    vidas -= 1;//Restamos una vida
    printf("Has acertado!!-->Quedan %d vidas\n",vidas);
    fflush(stdout);
    impacto = 0;//Quitamos el impacto
  }
  else//Si no ha impactado con el virus
  {
    printf("Has fallado!-->Quedan %d vidas\n",vidas);
    fflush(stdout);
  }
  b_balas[n].dim.x = b_balas[n].dim_ini.x;//Actualizamos la posicion de la bala a las iniciales (la recamara)
  b_balas[n].dim.y = b_balas[n].dim_ini.y;
  b_balas[n].bala_ON = 0;//Desactivamos la bala
  disparo -= 1;//Restamos un disparo activo
  if (vidas == 0)//Si se terminan las vidas
  {
    pthread_cond_broadcast (&cond);//Desbloqueamos cond
    gameOver = 1;//Finalizamos programa
  }
  pthread_mutex_unlock (&mutex);
  pthread_detach (pthread_self());//Liberamos los recursos del hilo
}

void *f_h_render()
{//Funcion de renderizado
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);//Habilitamos la cancelacion asincrona desde otros hilos
  while(1)
  {//Bucle de f_h_render
  //Cargamos imagenes principales
    SDL_BlitSurface(c_fondo.imagen, NULL, ventana, &c_fondo.dim);
    pthread_mutex_lock (&mutex);
    SDL_BlitSurface(o_virus.imagen, NULL, ventana, &o_virus.dim);
    SDL_BlitSurface(o_jeringa.imagen, NULL, ventana, &o_jeringa.dim);
    for (int i= 0; i < MAXB ; i++)
    {//Imagenes de balas
      SDL_BlitSurface(b_balas[i].imagen, NULL, ventana, &b_balas[i].dim);
    }
    for (int j = 0; j < vidas ; j++)
    {//Imagenes de vidas restantes
      SDL_BlitSurface(c_vida[j].imagen, NULL, ventana, &c_vida[j].dim);
    }
    if(gameOver == 1)
    {//Imagen de GameOver
      SDL_BlitSurface(c_gOver.imagen, NULL, ventana, &c_gOver.dim);
    }
    SDL_Flip (ventana);//Volcamos a pantalla el buffer
    pthread_mutex_unlock (&mutex);
    nanosleep(&nCiclo,NULL);
  }
}

bool f_colision(int n)
{//Funcion que calcula distancia entre bala n y virus y devuelve si colisiona
  bool impacto = 0;
  pthread_mutex_lock (&mutex);
  o_virus.centro.x = o_virus.dim.x + (o_virus.dim.w/2);//Coordenadas de centro del virus en x
  o_virus.centro.y = o_virus.dim.y + (o_virus.dim.h/2);//Coordenadas de centro del virus en y
  b_balas[n].centro.x = b_balas[n].dim.x + (b_balas[n].dim.w/2);//Coordenadas de centro de la bala en x
  b_balas[n].centro.y = b_balas[n].dim.y + (b_balas[n].dim.h/2);//Coordenadas de centro de la bala en y
  
  int distancia = sqrt(pow ((double)o_virus.centro.x -  b_balas[n].centro.x,2) + pow((double)o_virus.centro.y - b_balas[n].centro.y,2));//Calculamos distancia de ambos centros
  //Este calculo lo hemos hecho una vez y lo ponemos en comentario ya que las imagenes seran siempre las mismas
  //dis_crit= ((o_virus.dim.w/2^2 + o_virus.dim.h/2)^2)^(1/2) + (b_balas[n].dim.w^2 + b_balas[n].dim.h^2)^(1/2)
  //dist_crit = 101.85;
  if (distancia <= 102)// Si impacta
  {
    impacto = 1;//Activamos impacto para esta bala
  }
  pthread_mutex_unlock (&mutex);
  return impacto;
}

void f_fin()
{//Funcion que se activa al llegar la señal SIGRTMIN.En nuestro caso la manda el temporizador al terminar
  pthread_mutex_lock (&mutex);
  printf("\nLA PANDEMIA HA CONSEGUIDO EXPANDIRSE.\nEl programa se ha terminado por inactividad.\n");
  fflush(stdout);
  gameOver = 1;
  vidas = 0;
  pthread_cond_broadcast (&cond);
  pthread_mutex_unlock (&mutex);
}
void f_envioOK ()
{//Funcion que se activa al llegar la señal SIGRTMAX. En nuestro caso la envia el proceso padre
  pthread_mutex_lock (&mutex);
  lee = 1;//Activamos la lectura de datos por la cola de mensajes
  pthread_mutex_unlock (&mutex);
}
void *f_h_cerrar()
{//Funcion que sirve para manejar los eventos de la ventana de juego
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);//Habilitamos la cancelabilidad asincrona desde otros hilos
  SDL_Event event;//Evento
  while (1)
  {//Bucle de f_h_cerrar
    while (SDL_PollEvent(&event))//Mientras haya un evento
    {
      if (event.type == SDL_QUIT)//Si el evento es clickar en la x de la ventana
      {//Cerramos el programa abruptamente
	printf("Salida forzada por cerrar la ventana.\n");
	fflush(stdout);
	mq_unlink (NombreCola);
	exit(0);
      }
      else if (event.type == SDL_KEYDOWN)//Si el evento es pulsar una tecla
      {
	if (event.key.keysym.sym == SDLK_ESCAPE)//Si la tecla es ESC
	{//Cerramos el programa abruptamente
	  printf("Salida forzada por pulsar ESC.\n");
	  fflush(stdout);
	  mq_unlink (NombreCola);
	  exit(0);
	}
	else
	{//Notificamos que solo ESC es una tecla valida en la ventana de juego
	  printf("Tecla pulsada en ventana incorrecta, pulse teclas en consola o ESC en ventana para terminar\n");
	}
      }
    }
    nanosleep(&nCiclo,NULL);
  }
}
