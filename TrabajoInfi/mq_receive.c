#include <mqueue.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
//Tamaño de mesanjes para buffer, tiene ser como minimo 8192 que es el tamaño de msgsize_max
#define tamMsg 8192 

mqd_t mqdes; //descriptor de cola de mensaje
char buf[tamMsg]; //buffer de dimension de caracteres tamMsg
int n_bytes_recibidos;

int main(void)
{
  mqdes = mq_open("/ColaMensaje", O_RDONLY);
  //if (mqdes != -1){printf("La cola de mensajes se ha creado con éxito.\n");}
  //else{    printf("No se ha creado la cola de mensajes adecuadamente.\n");}
  
  n_bytes_recibidos = mq_receive(mqdes, buf, tamMsg, NULL);//Si todo va bien, nos devolvera el numero de bytes que han sido leidos de esa cola de mensajes
  
  printf("Mensaje recibido %s \nNumero de bytes recibidos %d \n",buf, n_bytes_recibidos);
  
  mq_close(mqdes);
  
  return 0;
}
