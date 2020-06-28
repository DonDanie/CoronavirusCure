//Coronavirus Cure Readme.txt
//Daniel Rosa Danta

#Para compilar el codigo se necesita:

1)Tener instalada la libreria "libsdl1.2-dbg" y "libsdl1.2-dev".
2)Escribir los siguientes codigos en consola:
    2.1)Compilar proceso_envio.c
    -->gcc -o proceso_envio proceso_envio.c -lrt -lpthread -lSDL -std=c99
    2.2)Compilar proceso_pral.c
    -->gcc -o proceso_pral proceso_pral.c -lrt -lpthread -lSDL -std=c99 
    
#Para ejecutar el programa se necesita:

1)Ejecutar el proceso_envio con 3 argumentos los cuales:
  Argumento 1-->Numero de vidas del virus (maximo 5 por limite grafico)
  Argumento 2-->Segundos de timeout para que el programa se cierre si no has pulsado ninguna tecla (no tiene maximo)
  Argumento 3-->Milisegundos de ciclo del programa (maximo 1000 por especificaciones del enunciado)

#PARA JUGAR
1)Ejecutar programa
2)Pulsar P(Play) en consola
3)Pulsar S(Start) en consola