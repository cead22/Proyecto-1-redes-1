/*
 remote.c
   servidor que se ejecutara en las maquinas
   que se quieren supervisar y que enviaran
   informacion de estas a edolab 
 Parametros:
   -f comandos: archivo de comandos que tienen
   permiso para ser ejecutados en la maquina
   -p puerto: puerto usado para escuchar comandos
   enviados por el cliente edolab
*/

#include <stdio.h>          
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h> 

/* numero de conexiones permitidad */
#define MAXCON 5

/* maximo de bytes a transferir por el socket */
#define MAXDATASIZE 300

/*
  funcion que verifica si un comando puede ser
  ejecutado en la maquina
  Parametros:
    comando: nombre del comando a ejecutar.
    archivo: nombre del archivo donde se
    especifican los comandos permitidos.
  Valor de Retorno:
     0 si la ejecucion del comando esta permitida.
    -1 en caso contrario

*/

int permitido(char *comando, char *archivo, int fd) {

  FILE *f;
  char linea[100];

  /* se abre el archivo */
  if ((f = fopen(archivo,"r")) == NULL) {
    //printf("Error al abrir archivo de comandos permitidos\n");
    return -1;
  }
  
  /* se verifica exitencia del comando */
  while (!feof(f)){
    fscanf(f,"%[^\n]%*[\n]",linea);
    if (strcmp(comando,linea) == 0) {
      /* comando permitido */
      if(fclose(f) != 0)
	perror("Error al cerrar archivo de comandos permitidos\n");
      return 0;
    }
  }

  /* comando no permitido */
  if(fclose(f) != 0)
    perror("Error al cerrar archivo de comandos permitidos\n");
  return 1;
}


void servidor(char *comans, int PORT){

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
  char buf[MAXDATASIZE];
  char out[MAXDATASIZE];
  pid_t pid;

  /* Se crea el socket */
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){  
    perror("Error en la funcion socket");
    exit(EXIT_FAILURE);
  }

  /* Se establecen los atributos del socket */
  server.sin_family = AF_INET;         
  server.sin_port = htons(PORT); 
  server.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server.sin_zero),8); 
   
  /* bind() */
  if(bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
    if (errno == EADDRINUSE){
      fprintf(stderr, "Puerto %d ya usado, escoga otro.\n", PORT);
    }
    else{
      perror("Error en la funcion bind");
    }
    exit(EXIT_FAILURE);
  }    

  /* Se establece la cantidad de maquinas a aceptar*/
  if(listen(fd,MAXCON) == -1) {
    perror("Error en la funcion listen");
    exit(EXIT_FAILURE);
  }

  sin_size=sizeof(struct sockaddr_in);

  while(1){
    /* se acepta la solicitud de conexion del cliente */
    if ((fd2 = accept(fd,(struct sockaddr *)&client,&sin_size)) == -1){
      perror("Error en la funcion accept()");
      exit(EXIT_FAILURE);
    }
    
    /* se crea hijo para atender solicitud */
    if ((pid = fork()) < 0) {
      perror("Error fork()");
      exit(EXIT_FAILURE);
    }
    if (pid == 0) {
      /* proceso hijo */

      if (send(fd2,"remote",6,0) == -1)
      	perror("Error send (1)");

      while(1){
 	if ((numbytes = recv(fd2,buf,MAXDATASIZE,0)) == -1){  
	  perror("Error en la funcion recv");
	  exit(EXIT_FAILURE);
	}
	
	buf[numbytes] = '\0';
	if (strcmp(buf,"salir") == 0) break;
	if (strcmp(buf,"\0") == 0) break;
	
	/* Se verifica que el comando esta permitido */
	switch (permitido((char *)&buf,comans,fd2)){
	case 1:
	  /* comando no permitido */
	  if (send(fd2,"fin_c",5,0) == -1)
	    perror("Error send (2)");
	  continue;
	case -1:
	  //perror("Error al abrir el archivo de comandos");
	  break;
	  //exit(EXIT_FAILURE);
	}
	
	/* se ejecuta el comando se lee la salida 
	 y se envia al cliente */
	salida = popen((char *)&buf,"r");
	fscanf(salida,"%[^\n]%*[\n]", out);
	if (send(fd2,strcat(out,"\n"),strlen(out)+1,0) == -1)
	  perror("Error send (3)");
	pclose(salida);
	if (send(fd2,"fin_c",5,0) == -1)
	  perror("Error send (4)");
      }
      exit(EXIT_SUCCESS);
    }
    else {
      /* proceso padre */
      wait(&estado);
    }
  } 
}

int main(int argc, char *argv[]) {

  int PORT;
  char *comans;
  
  /* Revision de llamada */
  if (argc != 5) {
    printf("Uso: ./remote -p <puerto> -f <comandosPermitidos>");
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-f") == 0) {
    PORT = (int)atoi(argv[2]);
    comans = argv[4];
  }
  else if (strcmp(argv[1],"-f") == 0 && strcmp(argv[3],"-p") == 0) {
    PORT = (int)atoi(argv[4]);
    comans = argv[2];
  }
  else {
    printf("Uso: ./remote -p <puerto> -f <comandosPermitidos>");
    exit(EXIT_FAILURE);
  }

  servidor(comans, PORT);
 
}
