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
#include <string.h> 

#define MAXCON 5 /* numero de conexiones permitidad */
#define MAXDATASIZE 100

int main(int argc, char *argv[]) {

  /* Variables */
  int PORT;
  int fd;
  int fd2;
  int estado;
  int sin_size;
  int numbytes;
  struct sockaddr_in server;	 
  struct sockaddr_in client; 
  FILE *comandos;
  FILE *salida;
  char *cmd;
  char *s;
  char *com;
  char buf[MAXDATASIZE];
  char out[MAXDATASIZE];
  pid_t pid;

  /* Revision de llamada */
  if (argc != 5) {
    printf("Uso: ./remote -p <puerto> -f <comandosPermitidos>");
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-f") == 0) {
    PORT = (int)atoi(argv[2]);
    com = argv[4];
  }
  else if (strcmp(argv[1],"-f") == 0 && strcmp(argv[3],"-p") == 0) {
    PORT = (int)atoi(argv[4]);
    com = argv[2];
  }
  else {
    printf("Uso: ./remote -p <puerto> -f <comandosPermitidos>");
    exit(EXIT_FAILURE);
  }

  /* Se crea el socket */
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){  
    printf("Error en la funcion socket \n");
    exit(EXIT_FAILURE);
  }

  /* Se establecen los atributos del socket */
  server.sin_family = AF_INET;         
  server.sin_port = htons(PORT); 
  server.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server.sin_zero),8); 
   
  /* A continuacion la llamada a bind() */
  if(bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
    printf("Error en la funcion bind.\n");
    exit(EXIT_FAILURE);
  }    

  /* Se establece la cantidad de maquinas a aceptar*/
  if(listen(fd,MAXCON) == -1) {
    printf("Error en la funcion listen.\n");
    exit(EXIT_FAILURE);
  }

  sin_size=sizeof(struct sockaddr_in);

  while(1){
    if ((fd2 = accept(fd,(struct sockaddr *)&client,&sin_size)) == -1){
      printf("Error en la funcion accept().\n");
      exit(EXIT_FAILURE);
    }
    
    if ((pid = fork()) < 0) {
      printf("Error fork()");
      exit(EXIT_FAILURE);
    }
    if (pid == 0) {
      /* proceso hijo */
      send(fd2,"remote",6,0);
      while(1){
 	if ((numbytes = recv(fd2,buf,MAXDATASIZE,0)) == -1){  
	  printf("Error en la funcion recv.\n");
	  exit(EXIT_FAILURE);
	}
	
	buf[numbytes] = '\0';
	if (strcmp(buf,"salir") == 0) break;
	if (strcmp(buf,"\0") == 0) break;
	
	/* Se verifica que el comando esta permitido */
	if (permitido((char *)&buf,com) == 0){
	  send(fd2,"fin_c",5,0);
	}
	
	salida = popen((char *)&buf,"r");
	fscanf(salida,"%[^\n]%*[\n]", out);
	send(fd2,strcat(out,"\n"),strlen(out)+1,0);
	pclose(salida);
	send(fd2,"fin_c",5,0);
      }
      exit(EXIT_SUCCESS);
    }
    else {
      /* proceso padre */
      wait(&estado);
    }
  } 
}
