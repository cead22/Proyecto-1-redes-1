char* leer_linea(FILE* f) {
  char* linea = (char*)calloc(0,sizeof(char));
  char c;
  int tam = 0;
  
  while ((c = fgetc(f)) != EOF && c != '\n'){
    linea = (char*) realloc(linea, sizeof(char) * (tam + 2));
    linea[tam++] = c;
    linea[tam] = '\0';
  } 
  return linea;
}

int permitido(char *comando, char *archivo) {

  FILE * f, * aux;
  char * linea;


  if ((f = fopen(archivo,"r")) == NULL) {
    printf("Error al abrir archivo en fun. pemitido()\n");
    exit(-1);
  }

  
  while (!feof(f)){
    linea = leer_linea(f);
    if (strcmp(comando,linea) == 0) {
      //fclose(aux);
      return 1;
    }
  }
  
  return 0;
}
