#define _POSIX_C_SOURCE 200112L
#include <SDL/SDL.h>/*Para usar la API SDL*/
#define tamMsg 8192/*Tamaño de mensajes para buffer, tiene ser como minimo 8192 que es el tamaño de msgsize_max*/
#define NombreCola "/ColaMsg" /*Nombre de cola de mensajes que vamos a crear*/
#define MAXB 3 /*Maximo numero de balas en pantalla*/
#define MAXV 5 /*Maximo numero de vidas*/
#define MAXC 1000 /*Maximo milisegundos de ciclo*/
#define NC  800 /*Ancho (numero de columnas) de la ventana del juego*/
#define NF  600 /*Alto (numero de filas) de la ventana del juego*/
#define FC 500 /*Altura de la jeringa o posicion en y permanente*/
#define mascara 0,0,0 /*Mascara para quitar fondo negro de imagenes*/
#define mascaraB 255,255,255/*Mascara para quitar fondo blanco de imagenes*/
#define vel_max 30 /*Velocidad maxima a la que puede ir un objeto*/
#define tamMsg 8192 //Tamaño de mensajes para buffer, tiene ser como minimo 8192 que es el tamaño de msgsize_max

struct dat_objeto	//Contiene informacion de objetos como el virus(objetivo) o la jeringuilla(cañon)
{
  bool objeto_ON;		//Estado del objeto
  SDL_Rect dim;			//Dimension del objeto
  SDL_Rect centro;		//Centro del objeto
  int vel;			//Velocidad del objeto
  SDL_Surface* imagen;		//Imagen del objeto
};
struct dat_bala		//Contiene la informacion de la bala
{
  bool bala_ON;		//Estado del objeto
  int id;			//Identificador del objeto
  SDL_Rect dim_ini;		//Dimension inicial del objeto
  SDL_Rect dim;			//Dimension del objeto
  SDL_Rect centro;		//Centro del objeto
  pthread_t nombreHilo;		//Nombre del hilo del objeto
  SDL_Surface* imagen;		//Imagen del objeto
};
struct dat_capa		//Contiene informacion de las capas de imagenes que vamos a utilizar.
{
  SDL_Rect dim;			//Dimension del objeto
  SDL_Surface* imagen;		//Imagen del objeto
};

//Estructuras que hemos utilizado pero que están ya declaradas en sus librerias correspondientes. 
//Solamente escritas para consulta rapida

/*struct SDL_Rect
 * {
 * int x; 	//The x location of the rectangle's upper left corner
 * int y; 	//The y location of the rectangle's upper left corner
 * int w; 	//The width of the rectangle --> ancho
 * int h;	//The height of the rectangle --> largo
 * };*/

/*Struct timespec{
 * time_t tv_sec; 	//Campo de los segundos
 * long tv_nsec;	//Campo de los nanosegundos
 * };*/

/*struct itimespec{
  struct timespec it_value; 	//Primera vez
  struct timespec it_interval;	//Siguientes veces
  };*/

/*struct timeval {
  time_t      tv_sec;     // segundos 
  suseconds_t tv_usec;    // microsegundos
  };*/

/*struct termios {
  tcflag_t c_iflag; //modos de entrada
  tcflag_t c_oflag; //modos de salida
  tcflag_t c_cflag; //modos de control 
  tcflag_t c_lflag; //modos locales 
  cc_t c_cc [NCCS]; //caracteres especiales 
  speed_t c_ispeed;
  speed_t c_ospeed;
};*/

/*struct mq_attr {
  largo mq_flags; //Banderas: 0 o O_NONBLOCK 
  largo mq_maxmsg; //Máx. # de mensajes en la cola 
  largo mq_msgsize; //Máx. tamaño del mensaje (bytes) 
  largo mq_curmsgs; //# de mensajes actualmente en cola 
};*/

/*typedef union SDL_Event {
    Uint8 type;
    SDL_ActiveEvent active;
    SDL_KeyboardEvent key;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyBallEvent jball;
    SDL_JoyHatEvent jhat;
    SDL_JoyButtonEvent jbutton;
    SDL_ResizeEvent resize;
    SDL_ExposeEvent expose;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    SDL_SysWMEvent syswm;
} SDL_Event;*/