/*
 Remote.c
 Programa que se ejecutara localmente
 en las maquinas que se quieren supervisar
 para obtener informacion de las mismas y
 enviarla al proceso edolab
*/

#include <stdio.h>          
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "funciones.h" 

#define BACKLOG 2 /* El numero de conexiones permitidas */
#define MAXDATASIZE 100 

int main(int argc, char *argv[]){

  int PORT = (int)atoi(argv[1]);

  int fd, fd2, estado; /* los ficheros descriptores */

  struct sockaddr_in server;	 
  /* para la informacion de la direccion del servidor */

  struct sockaddr_in client; 
  /* para la informacion de la direccion del cliente */

  int sin_size, numbytes;

  FILE * comandos;

  char * cmd, * s;

  pid_t pid;

  FILE * salida;

  char buf[MAXDATASIZE];
  char out[MAXDATASIZE];

  /* A continuacion la llamada a socket() */
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){  
    printf("Error en la funcion socket \n");
    exit(EXIT_FAILURE);
  }
  server.sin_family = AF_INET;         

  server.sin_port = htons(PORT); 

  server.sin_addr.s_addr = INADDR_ANY; 

  bzero(&(server.sin_zero),8); 
  /* escribimos ceros en el resto de la estructura */

   
  /* A continuacion la llamada a bind() */
  if(bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
    printf("Error en la funcion bind.\n");
    exit(EXIT_FAILURE);
  }    

  if(listen(fd,BACKLOG) == -1) {  /* llamada a listen() */
    printf("Error en la funcion listen.\n");
    exit(EXIT_FAILURE);
  }

  sin_size=sizeof(struct sockaddr_in);
  /* A continuacion la llamada a accept() */
  if ((fd2 = accept(fd,(struct sockaddr *)&client,&sin_size)) == -1){
    printf("Error en la funcion accept().\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    if ((numbytes = recv(fd2,buf,MAXDATASIZE,0)) == -1){  
      /* llamada a recv() */
      printf("Error en la funcion recv.\n");
      exit(EXIT_FAILURE);
    }
   if (strcmp(buf,"fin") == 0){
      printf("saliendo\n");
      break;
    }

    buf[numbytes] = '\0';
    
    if (strcmp(buf,"\0") == 0) break;
    if (permitido(&buf,argv[2]) == 0) break; //no permitido 
    salida = popen(&buf,"r");

    while (!feof(salida)){
      fscanf(salida,"%[^\n]%*[\n]", out);
      printf("a: %s\n",out);
      send(fd2,strcat(out,"\n"),strlen(out)+1,0);
    }
    send(fd2,"fin",4,0);
    pclose(salida);
  }
  close(fd2); /* cierra fd2 */
  exit(EXIT_SUCCESS);

}
