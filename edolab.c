/* 
   edolab.c
     cliente que permitira supervisar varias
     maquinas de una red, conectandose a ellas
     y solicitando informacion de las mismas
   para generar un reporte.
   Parametros:
     -f maquinas: archivo que contiene
     los nombres o direcciones ip de las maquinas
     que se desean supervisar.
     -p puerto: puerto mediante el cual se va a 
     establecer la comunicacion con el servidor.
   Valor de Retorno:
     0 exito.
     1 error
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>      
      
/* maximo de bytes a transferir por el socket */
#define MAXDATASIZE 300

/* maximo de maquinas a supervisar */
#define MAXMAQUINAS 50

/* maximo tamano del nombre de una maquina */
#define MAXNOMMAQ 25

/* puerto de conexion con servidor web */
#define PORT_WEB 80

/* Rutina principal del proceso supervisor */
int main(int argc, char *argv[]) {

  /* Variables */
  int fd;          /* descriptor de socket servidor remote */
  int web;         /* descriptor de socket servidor web    */
  int numbytes;    /* bytes recibidos mediante recv()      */
  int i;           /* para recorrer arreglo de nodos       */
  int k;           /* para recorrer arreglo de comandos    */
  int c;           /* control de funcion select()          */
  int equipos;     /* cantidad de maquinas supervisadas    */
  int PORT;
  FILE *maquinas;
  FILE *ping;
  char conexion[250]; /* almacena salida del comando ping  */
  char *ip;
  char sw[5];
  char *comando_ping = malloc(sizeof(char)*26);
  char *maq;
  char nodo[MAXMAQUINAS][MAXNOMMAQ]; /* maquinas a supervisar */
  char *comandos[] = {"uptime",
		      "grep MemTotal /proc/meminfo",
		      "grep \"model name\" /proc/cpuinfo"};
  char buf[MAXDATASIZE];  /* almacena informacion de las maquinas */
  struct hostent *he;         
  struct sockaddr_in server;
  struct sockaddr_in server_aux;
  struct timeval tv;      /* para poner timeouts en funcion recv() */
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

  /* Maquinas a supervisar */
  if ((maquinas = fopen(maq, "r")) == NULL) {
    fprintf(stderr,"Error al abrir archivo %s: %d", maq, errno);
    exit(EXIT_FAILURE);
  }
  
  i = 0;
  equipos = 1;

  while(!feof(maquinas)) {
    fscanf(maquinas,"%[^\n]%*[\n]",nodo[i]);
    printf("blah : %d",strlen(nodo[i]));
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
   
    /* Se establecen los atributos para la conexion con el remote */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); 
    server.sin_addr = *((struct in_addr *)he->h_addr);  
    bzero(&(server.sin_zero),8);

   
    /* Se crea el socket para verificar el servidor web */
    if ((web=socket(AF_INET,SOCK_STREAM,0)) == -1){
      printf("Error en la creacion del socket");
      exit(EXIT_FAILURE);
    }
    
    /* Se establecen los atributos para la verificacion del servidor web*/
    server_aux.sin_family = AF_INET;
    server_aux.sin_port = htons(PORT_WEB);
    server_aux.sin_addr = *((struct in_addr *) he->h_addr);
    bzero(&(server_aux.sin_zero),8);

    /* Se obtiene ip del servidor */
    ip = inet_ntoa(*((struct in_addr *)he->h_addr));

    printf("Equipo: %d\n",equipos-i);
    printf("Nombre: %s",he->h_name);
    printf("\tIp: %s \n",ip);

    /* Se crea el comando para verificar la conexion a la red del servidor.*/
    comando_ping = strcat(comando_ping,"ping -c 3 ");
    comando_ping = strcat(comando_ping,ip);
    //comando_ping = strcat(comando_ping,"| grep received | awk '{print $4}'");
    
    ping = popen(comando_ping,"r");
    fscanf(ping,"%[^\n]%*[\n]",conexion);
    fscanf(ping,"%[^\n]%*[\n]",conexion);
    pclose(ping);
    conexion[2]='\0';
   
    /* Si la salida obtenida fue 0 se concluye que no hay red dado que esa
       es la cantidad de paquetes recibidos */
    if (strcmp(conexion,"64") != 0) {
      printf("La conexion a la red del equipo esta: no operativa\n");
      i--;
      continue;
    }
    printf("La conexion a la red del equipo esta: operativa\n", conexion);
    

    /* Se verifica que se haya podido establecer conexion con el remote */
    if(connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){ 
      printf("No se pudo establecer comunicacion con remote.\n");
      i--;
      continue;
    }
    
    /* Timeout para verificacion de que el puerto esta siendo usado efectivamente 
       por el remote */
    
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if ((c = select(FD_SETSIZE, &rfds, (fd_set *)NULL, (fd_set *)NULL, &tv)) == -1){
      printf("Error select\n");
      exit(EXIT_FAILURE);
    }

    /* Transcurrio el tiempo y no se recibio ningun mensaje (Se espera un 
       mensaje del remote para asegurar que la conexion se hizo con este). */
    if (c == 0) {
      printf("No se pudo establecer comunicacion con remote\n");
      i--;
      continue;
    }
    /* Se recibio un mensaje */
    else {
      /* Se verifica el mensaje obtenido ha sido efectivamente enviado por el remote */
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
	  switch(k){
	  case 0: // uptime
	    sscanf(buf,"%*s %*s %s%*c",buf);
	    buf[strlen(buf)-1] = '\0';
	    printf("El equipo esta operando desde hace: %s\n", buf); 
	    break;
	  case 1: // memoria
	    sscanf(buf,"%*s %s",buf);
	    buf[strlen(buf)] = '\0';
	    printf("Cantidad de memoria: %s kb\n", buf); 
	    break;
	  case 2: // procesador
	    sscanf(buf,"%*s %*s %*s %[^\n]%*",buf);
	    buf[strlen(buf)] = '\0';
	    printf("Modelo del Procesador: %s\n", buf); 
	    break;
	  }

	}
      }
      printf("El servidor remote del equipo esta: operativo.\n");
      
      /* Se verifica si hay servidor web activo */
      if (connect(web,(struct sockaddr *)&server_aux, sizeof(struct sockaddr)) == -1) {
	printf("El servidor web del equipo esta: no operativo.\n");
      }
      else {
	send(web,"GET / HTTP/1.0\r\n\r\n",22,0);
	FD_ZERO(&rfds);
	FD_SET(web,&rfds);
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	if ((c = select(FD_SETSIZE, &rfds, (fd_set *)NULL, (fd_set *)NULL, &tv)) == -1){
	  printf("Error select\n");
	  exit(EXIT_FAILURE);
	}
	/* Transcurrio el tiempo del espera del mensaje a recibir */
	if (c == 0) {
	  printf("El servidor web del equipo esta: no operativo. n\n");
	  i--;
	  continue;
	}
	/* Se recibio un mensaje */
	else {
	  /* Se verifica que el mensaje recibido sea del servidor web  */
	  bzero(&buf,8);
	  if ((numbytes = recv(web,buf,MAXDATASIZE,0)) == 0){
	    printf("El servidor web del equipo esta: no operativo. n\n");
	    i--;
	    continue;
	  }
	 
	  buf[4] = '\0';
	  if (strcmp(buf,"HTTP") != 0){
	    printf("El servidor web del equipo esta: no operativo.\n");
	  }
	  else{
	      printf("El servidor web del equipo esta: operativo.\n");
	  }
	}
      }
      send(fd,"salir",5,0);
      i--;
      close(fd);
    }
  }
  close(maquinas);
  exit(EXIT_SUCCESS);
}

/// CERRAR MAQUINAS FCLOSE

