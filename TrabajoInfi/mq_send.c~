#include <mqueue.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h> /*Para usar errno*/


//Tamaño de mensajes para buffer, tiene ser como minimo 8192 que es el tamaño de msgsize_max
#define tamMsg 8192

mqd_t mqdes; //descriptor de cola de mensaje
char buf[tamMsg]; //buffer de dimension de caracteres tamMsg
extern int errno;

int main(void)
{
  int  errnum;
  errnum = errno;
  struct mq_attr atr_send;
  atr_send.mq_msgsize = tamMsg; //Maximo tamaño de bytes del mensaje
  atr_send.mq_maxmsg = 2; //Maximo numeros de mensajes en cola
  
  //cola_pet = mq_open(arg4, O_CREAT | O_RDONLY, S_IRWXU, &atr_pet); 
  mqdes = mq_open("/ColaMsg",O_CREAT | O_WRONLY, 0664, &atr_send,errno);//Creamos cola, si se crea nos manda un descriptor de cola, si se crea mal, el descriptor valdrá -1
  if (mqdes != -1){
    printf("La cola de mensajes se ha creado con éxito.\nValor del descriptor=%d \n",mqdes);
    printf("Introduzca mensaje.\n");
    scanf("%s", buf);
    mq_send(mqdes, buf, strlen(buf) + 1, 0);
  }
  else{    
    printf("No se ha creado la cola de mensajes adecuadamente.\nValor del descriptor=%d \n",mqdes);
    fprintf(stderr, "Valor de errno: %d\n",errno);
    perror("Errror impreso por perror");
    fprintf(stderr ,"Error: %s\n",strerror(errnum));
  }
  
  mq_close(mqdes);
  
  return 0;
}
