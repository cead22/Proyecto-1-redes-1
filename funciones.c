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
