#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include "funciones.h"        
      

#define MAXDATASIZE 100 
#define MAXMAQUINAS 25


int main(int argc, char *argv[]) {

  int fd, numbytes, i;
  int PORT = (int)atoi(argv[2]); 
 
  FILE *maquinas;
  
  char *nodo[MAXMAQUINAS];
 
  char buf[MAXDATASIZE];  
 
  struct hostent *he;         
 
  struct sockaddr_in server;  

  if (argc != 3) {
    printf("Uso: %s archivo puerto",argv[0]);
    exit(EXIT_FAILURE);
  }

  if ((maquinas = fopen(argv[1], "r")) == NULL) {
    printf("Error al abrir archivo %s", argv[1]);
    exit(EXIT_FAILURE);
  }
  
  i = 0;
  int equipos = 1;

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
   
    if ((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){  
      printf("Error en la creacion del socket: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); 
    server.sin_addr = *((struct in_addr *)he->h_addr);  
    bzero(&(server.sin_zero),8);

    printf("Equido: %d\n",equipos-i);

    printf("Nombre: %s",he->h_name);
    
    printf("\tIp: %s \n",inet_ntoa(*((struct in_addr *)he->h_addr)));

    if(connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){ 
      /* llamada a connect() */
      printf("La conexion de la red al equipo esta: no operativa %d\n",strerror(errno));
      exit(EXIT_FAILURE);
    }

    printf("La conexion de la red al equippo esta: operativa\n");

    char nsl[50] = "nslookup ";
    
    send(fd,"uptime | grep user",strlen("uptime | grep user"),0);
      
    numbytes = 1;
    
    while (1){
      if ((numbytes = recv(fd,buf,MAXDATASIZE,0)) == -1){  
	/* llamada a recv() */
	printf("Error en la funcion recv: %s \n", strerror(errno));
	exit(EXIT_FAILURE);
      }
      if (numbytes == 0) break;
      if (strcmp(buf,"fin") == 0){
       printf("saliendo22\n");
       send(fd,"fin",4,0);
       break;
      }
      buf[numbytes]='\0';
      printf("%s",buf); 
      /* muestra el mensaje de bienvenida del servidor =) */
    }
    i--;
    
    close(fd);
  }   /* cerramos fd =) */
  exit(EXIT_SUCCESS);
}

/// CERRAR MAQUINAS FCLOSE
