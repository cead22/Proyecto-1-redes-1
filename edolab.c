#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "funciones.h"        
      
/* netbd.h es necesitada por la estructura hostent ;-) */

/* El Puerto Abierto del nodo remoto */

#define MAXDATASIZE 100 
#define MAXMAQUINAS 25
/* El nï¿½mero mï¿½ximo de datos en bytes */



int main(int argc, char *argv[]) {

  int fd, numbytes, i;
  int PORT = (int)atoi(argv[2]); 
 
  FILE *maquinas;
  
  char *nodo[MAXMAQUINAS];
 
  char buf[MAXDATASIZE];  
 
  struct hostent *he;         
 
  struct sockaddr_in server;  
    
  if (argc != 3) {
    printf("Uso: %s <Direccion IP>\n puerto",argv[0]);
    exit(-1);
  }

  if ((maquinas = fopen(argv[1], "r")) == NULL) {
    perror("Error al abrir archivo");
    exit(-1);
  }
  
  i = 0;
  int equipos = 1;

  while(!feof(maquinas)) {
    nodo[i] = leer_linea(maquinas);
    if (strcmp(nodo[i],"\0") == 0) break;
    i++;
    equipos++;
  }
  
  i--;
  equipos--;

  printf("Numero de equipos registrados es: %d\n",equipos);
 
  while(i >= 0) {
    
    if ((he = gethostbyname(nodo[i])) == NULL){       
      printf("gethostbyname() error\n");
      exit(-1);
    }
   
    if ((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){  
      /* llamada a socket() */
      printf("socket() error\n");
      exit(-1);
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); 
    server.sin_addr = *((struct in_addr *)he->h_addr);  
    //server.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(server.sin_zero),8);

    printf("Equido %d\n",equipos-i);

    printf("Nombre %s\n",he->h_name);
    
    printf("Ip %s \n",inet_ntoa(*((struct in_addr *)he->h_addr)));



    if(connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){ 
      /* llamada a connect() */
      printf("La conexion de la red al equippo esta: no operativa\n");
      exit(0);
    }

    
    printf("La conexion de la red al equippo esta: operativa\n");

    char nsl[50] = "nslookup ";
    system(strcat(nsl,nodo[i]));
    system("ps -e | grep remote");
    
    numbytes = 1;
    
    while (1){
      if ((numbytes = recv(fd,buf,MAXDATASIZE,0)) == -1){  
	/* llamada a recv() */
	printf("Error en recv() \n");
	exit(-1);
      }
      if (numbytes == 0) break;
      //printf("numbytes %d:\n",numbytes);
      
      buf[numbytes]='\0';
      
      printf("%s",buf); 
      /* muestra el mensaje de bienvenida del servidor =) */
    }
    i--;
    
    close(fd);
  }   /* cerramos fd =) */
}

/// CERRAR MAQUINAS FCLOSE
