#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>/*Para usar colas de mensajes*/
#include <sys/stat.h>
#include <string.h>
#include <errno.h> /*Para usar errno*/
#include <pthread.h>/*para usar hilos*/

//Tamaño de mensajes para buffer, tiene ser como minimo 8192 que es el tamaño de msgsize_max
#define tamMsg 8192

mqd_t mqdes; //descriptor de cola de mensaje
void inicializar_cola(struct mq_attr atr_send)
{
  int  errnum;
  errnum = errno;
  mqdes = mq_open("/ColaMensaje",O_CREAT | O_WRONLY, 0664, &atr_send,errno);//Creamos cola, si se crea nos manda un descriptor de cola, si se crea mal, el descriptor valdrá -1
    
    if (mqdes != -1){
      printf("La cola de mensajes se ha creado con éxito.\nValor del descriptor=%d \n",mqdes);
    }
    else{    
      printf("No se ha creado la cola de mensajes adecuadamente.\nValor del descriptor=%d \n",mqdes);
      fprintf(stderr, "Valor de errno: %d\n",errno);
      perror("Errror impreso por perror");
      fprintf(stderr ,"Error: %s\n",strerror(errnum));
    }
}
void set_mode(int want_key)
{
	static struct termios old, new;
	if (!want_key) {
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		return;
	}
	tcgetattr(STDIN_FILENO, &old);
	new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new);
}
 
int get_key()
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;
 
	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);
	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);
 
	if (FD_ISSET(STDIN_FILENO, &fs)) {
		c = getchar();
		set_mode(0);
	}
	return c;
}

void mandar_msg()
{
  int c;
  char cchar[2];
    
  while(1) {
	  set_mode(1);
	  while (!(c = get_key())) usleep(10000);
	  printf("key %d\n", c);
	  cchar[0] = (char)c;
	  cchar[1] = '0';
	  printf("A char=%c\n",cchar);
	  mq_send(mqdes, cchar, strlen(cchar)+1, 0);
  }
}


//Funcion realizada por el hilo de recepcion de la cola de mensajes
void *funcion_hilo_receptor(void *arg)
{
  char buf[2];
  int n_bytes_recibidos;
  mqdes = mq_open("/ColaMensaje", O_RDONLY);
  while(1)
  {
    n_bytes_recibidos = mq_receive(mqdes, buf, tamMsg, NULL);//Si todo va bien, nos devolvera el numero de bytes que han sido leidos de esa cola de mensajes
    //printf("Mensaje recibido %s \nNumero de bytes recibidos %d \n",buf, n_bytes_recibidos);
    printf("%d\n",buf);
    mq_close(mqdes);
  }
}
//HILO PRINCIPAL
int main()
{
    int valor;
    struct mq_attr atr_send;
    atr_send.mq_msgsize = tamMsg; //Maximo tamaño de bytes del mensaje
    atr_send.mq_maxmsg = 5; //Maximo numeros de mensajes en cola
    inicializar_cola(atr_send);
  //Creamos el hilo receptor
    pthread_t hilo_receptor;
    pthread_create(&hilo_receptor, NULL, funcion_hilo_receptor, &valor);
    mandar_msg();
    mq_close(mqdes);
    return 0;
}