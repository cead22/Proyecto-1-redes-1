#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include "funciones.h"  
#include <string.h>      
      
#define MAXDATASIZE 600
#define MAXMAQUINAS 50
#define PORT_WEB 80

int main(int argc, char *argv[]) {

  /* Variables */
  int fd, web;
  int numbytes;
  int i;
  int k;
  int c;
  int equipos;
  int PORT;
  FILE *maquinas;
  char *maq;
  char *nodo[MAXMAQUINAS];
  char *comandos[] = {"uptime",
		      "grep MemTotal /proc/meminfo",
		      "grep \"model name\" /proc/cpuinfo"};
  char buf[MAXDATASIZE];  
  struct hostent *he;         
  struct sockaddr_in server, server_aux;
  struct timeval tv;
  fd_set rfds;

  /* Revision de llamada */
  if (argc != 5) {
    printf("Uso: ./edolab -f <maquinas> -p <puertoRemote>");
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1],"-f") == 0 && strcmp(argv[3],"-p") == 0) {
    PORT = (int)atoi(argv[4]);
    maq = argv[2];
  }
  else if (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-f") == 0) {
    PORT = (int)atoi(argv[2]);
    maq = argv[4];
  }
  else {
    printf("Uso: edolab -f <maquinas> -p <puertoRemote>");
    exit(EXIT_FAILURE);
  }

  /* Se abre el archivo que indican las maquinas deonde esta el remote*/
  if ((maquinas = fopen(maq, "r")) == NULL) {
    printf("Error al abrir archivo %s", maq);
    exit(EXIT_FAILURE);
  }
  
  i = 0;
  equipos = 1;

  while(!feof(maquinas)) {
    nodo[i] = leer_linea(maquinas);
    if (strcmp(nodo[i],"\0") == 0) break;
    equipos++;
    i++;
  }
  
  i--;
  equipos--;
  
  printf("Numero de equipos registrados es: %d\n", equipos);
  
  while(i >= 0) {
   
    if ((he = gethostbyname(nodo[i])) == NULL){       
      perror("Nombre de servidor desconocido");
      exit(EXIT_FAILURE);
    }
   
    /* Se crea el socket para establecer conexion con el remote*/
    if ((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){  
      printf("Error en la creacion del socket: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
    /* SE crea el socket para verificar el servidor web */
    if ((web=socket(AF_INET,SOCK_STREAM,0)) == -1){
      printf("Error en la creacion del socket");
      exit(EXIT_FAILURE);
    }
    
    /* Se establecen los atributos para la conexion con el remote */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); 
    server.sin_addr = *((struct in_addr *)he->h_addr);  
    bzero(&(server.sin_zero),8);

    /* Se establecen los atributos para la verificacion del servidor web*/
    server_aux.sin_family = AF_INET;
    server_aux.sin_port = htons(PORT_WEB);
    server_aux.sin_addr = *((struct in_addr *) he->h_addr);
    bzero(&(server_aux.sin_zero),8);

   

    printf("Equipo: %d\n",equipos-i);

    printf("Nombre: %s",he->h_name);
    
    printf("\tIp: %s \n",inet_ntoa(*((struct in_addr *)he->h_addr)));

    /* Se verifica que se haya podido establecer conexion con el remote */
    if(connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){ 
      printf("No se pudo establecer comunicacion con remote.\n");
      i--;
      continue;
    }
    /* Time out */
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if ((c = select(FD_SETSIZE, &rfds, (fd_set *)NULL, (fd_set *)NULL, &tv)) == -1){
      printf("Error select\n");
      exit(EXIT_FAILURE);
    }
    /* Transcurrio el tiempo de espera, no hizo conexion con el remote */
    if (c == 0) {
      printf("No se pudo establecer comunicacion con remote\n");
      i--;
      continue;
    }
    /* Se establecio conexion con el remote */
    else {
      /* Se verifica que se haya obtenido el mensaje enviado por el remote */
      if ((numbytes = recv(fd,buf,MAXDATASIZE,0)) != 6 || strcmp(buf,"remote") != 0) {
	printf("No se pudo establecer comunicacion con remote.\n");
	i--;
	continue;
      }
      /* Se envian los comandos a ejecutar al remote */
      for (k = 0; k < 3; k++){
	numbytes = 1;
	send(fd,comandos[k],strlen(comandos[k]),0);
	while ((numbytes = recv(fd,buf,MAXDATASIZE,0)) >= 0){
	  if (numbytes == 0) break;
	  buf[numbytes]='\0';
	  /* Se verifica que se haya terminado de enviar el mensaje desde el remote*/
	  if (strcmp(buf,"fin_c") == 0) {
	    break;
	  }
	  /* Se imprimen los datos obtenidos */
	  printf("%s", buf); 
	}
      }
      printf("El servidor remote del equipo esta: operativo.\n");
      
      /* Se verifica que si hay servidor web activo */
      if (connect(web,(struct sockaddr *)&server_aux, sizeof(struct sockaddr)) == -1) {
	printf("El servidor web del equipo esta: no operativo\n");
      }
      else {
	printf("El servidor web del equipo esta: operativo.\n");
      }
      send(fd,"salir",5,0);
      
      
      i--;
      //close(fd);
    }
  }
  exit(EXIT_SUCCESS);
}

/// CERRAR MAQUINAS FCLOSE
