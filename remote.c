/*
 Remote.c
 Programa que se ejecutara localmente
 en las maquinas que se quieren supervisar
 para obtener informacion de las mismas y
 enviarla al proceso edolab
*/

#include <stdio.h>          
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "funciones.h" 

/* El puerto que sera abierto */
#define BACKLOG 2 /* El numero de conexiones permitidas */

int main(int argc, char *argv[]){

  int PORT = (int)atoi(argv[1]);

  int fd, fd2, estado; /* los ficheros descriptores */

  struct sockaddr_in server;	 
  /* para la informacion de la direccion del servidor */

  struct sockaddr_in client; 
  /* para la informacion de la direccion del cliente */

  int sin_size;

  FILE * comandos;

  char * cmd, * s;

  pid_t pid;

  FILE * salida;

  /* A continuacion la llamada a socket() */
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){  
    printf("error en socket()\n");
    exit(-1);
  }
  server.sin_family = AF_INET;         

  server.sin_port = htons(PORT); 

  server.sin_addr.s_addr = INADDR_ANY; 

  bzero(&(server.sin_zero),8); 
  /* escribimos ceros en el resto de la estructura */

   
  /* A continuacion la llamada a bind() */
  if(bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
    printf("error en bind() \n");
    exit(-1);
  }    

  if(listen(fd,BACKLOG) == -1) {  /* llamada a listen() */
    printf("error en listen()\n");
    exit(-1);
  }

  sin_size=sizeof(struct sockaddr_in);
  /* A continuacion la llamada a accept() */
  if ((fd2 = accept(fd,(struct sockaddr *)&client,&sin_size)) == -1){
    printf("error en accept()\n");
    exit(-1);
  }

  // redireccion de la salida estandar
  /*if (dup2(fd2,1) < 0){
    printf("error al redireccionar la salida");
    exit(-1);
    }*/

  if ((comandos = fopen(argv[2],"r")) == NULL){
    printf("Error al abrir archivo de comandos");
    exit(-1);
  }

  while(!feof(comandos)){
    cmd = leer_linea(comandos);
    if (strcmp(cmd,"\0") == 0) break;
    salida = popen(cmd,"r");
    while (!feof(salida)){
      s = leer_linea(salida);
      send(fd2,strcat(s,"\n"),strlen(s)+1,0);
    }
    pclose(cmd);
  }
  fclose(comandos);

  close(fd2); /* cierra fd2 */
}
