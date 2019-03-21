#define CCLEAR "\e[30;0m"
#define CRED "\e[30;31m"
#define CGREEN "\e[30;32m"
#define CYELLOW "\e[30;33m"
#define CBLUE "\e[30;34m"
#define CMAGENTA "\e[30;35m"

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/types.h>  
#include <fcntl.h>

//struct que guarda los valores del nombre y el color de cada cliente
struct Cliente{
  char Nombre[50];
  char color[10];
  int puerto;
  int sockfd;
};

struct Cliente OnLine[5];
int cli_st[5], pipefd[5][2];

//funcion que maneja la comunicacion entre los sockets
int clientMg(int socket_fd, int *pipefd)
{
  printf("cliente connectado %d\n", socket_fd);
  //guardar el nombre del cliente nuevo que se conecto
  //struct cliente
  char buffer[1024];
  const char *message = "Coneccion Establecida\n\0";
  char nombre[50] = "";
  int valread;
  if(send(socket_fd, message, strlen(message),0)!=strlen(message)) {
    perror("send");
    return -1;
  }
  if( ( valread = read(socket_fd,nombre,50) ) == 0 ) {
    printf("Error en coneccón\n");
    return -4;
  }
  for(;;) {
    if( (valread = read(socket_fd, buffer, 1024) ) == 0 ) {
      /** disconnect **/
      printf("Cliente desconectado %d\n",socket_fd);
      break;
    } else {
      buffer[valread] = '\0';
      printf("Recived From Client:\t%s\n",buffer);
      send(socket_fd,buffer,strlen(buffer),0);
    }
  }
  return 0;
}

int main(int argc, char const *argv[]) 
{
  struct Cliente Cliente1, Cliente2, Cliente3;
  //colores preasignados por posicion, es decir el primero que se conecte siempre sera rojo, el segundo verde, etc
  strcpy(Cliente1.color,CRED);
  strcpy(Cliente2.color,CYELLOW);
  strcpy(Cliente3.color,CBLUE);
  
  //3 para cada uno de los posibles clientes,limite 3 clientes(for now)
  OnLine[0] = Cliente1;
  OnLine[1] = Cliente2;
  OnLine[2] = Cliente3;
  for(int i=0;i<5;++i){ 
    cli_st[i]=-1;
    pipe(pipefd[i]);
    fcntl(pipefd[i][0],F_SETFL, fcntl(pipefd[i][0],F_GETFL,0)|O_NONBLOCK); //non block read end
    fcntl(pipefd[i][1],F_SETFL, fcntl(pipefd[i][1],F_GETFL,0)|O_NONBLOCK); //non block write end
  }
  
  int PORT = 0;
  if(argc<2) {
    printf("usage: %s port\n",argv[0]);
    return -1;
  } else {
    PORT = atoi(argv[1]);
  }
  int server_fd, new_fd; 
  struct sockaddr_in address; 
  int opt = 1, valread, addrlen = sizeof(address), pid;
  
  // crear el socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
    perror("socket failed"); 
    exit(EXIT_FAILURE); 
  } 
  
  // pegar el socket al puerto que puede ser 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
    perror("setsockopt"); 
    exit(EXIT_FAILURE); 
  } 
  address.sin_family = AF_INET; 
  //especificar la ip
  address.sin_addr.s_addr = INADDR_ANY; 
  //especificar el puerto que se usa
  address.sin_port = htons(PORT); 
  
  // pegar el socket al PORT a la fuerza si es necesario
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
    perror("bind failed"); 
    exit(EXIT_FAILURE); 
  }
  
  if(listen(server_fd, 5)<0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  
  char buffer[1025];
  pid = fork();
  if(pid<0){return -4;}
  if(pid>0){ //parent
    /**manage multiple pipes **/
    for(;;){
      for(int i=0;i<5;++i){
        if(read(pipefd[i][0],buffer,1024)<0) {
          /**read error, no info**/
          continue;
        } else {
          /**something was actually read**/
          printf("%d:: %s\n",i,buffer);
        }
      }
    }
  } else {
    /**child**/
    /**manage server sockets**/
    int i;
    const char *mess = "Hello there\n";
    while(1){
      if( (new_fd = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) <0){
        perror("accept");
        continue;
      }
      for(i=0;i<5;++i){
        if(cli_st[i]<0){
          cli_st[i]=10;
          break;
        }
      }
      pid = fork();
      if(pid>0){ //parent
        close(new_fd);
      } else { //child
        close(server_fd);
        send(new_fd,mess,strlen(mess),0);
        if((valread = read(new_fd,buffer,1024))>0){
          buffer[valread] = '\0';
          //write(pipefd[i][1],buffer,1024);
        }
        fcntl(new_fd,F_SETFL,fcntl(new_fd,F_GETFL,0)|O_NONBLOCK);
        for(;;){
          if((valread = read(new_fd,buffer,1024))>0){
            buffer[valread] = '\0';
            //printf("Recibeb %s\n",buffer);
            //send(new_fd,buffer,1024,0);
            write(pipefd[i][1],buffer,1024);
          } else if(valread == 0){
            /**disconect**/
            printf("Cliente %d desconectado\n",i);
            break;
          }
          if( (valread = read(pipefd[i][0],buffer,1024))>0){
            buffer[valread] = '\0';
            send(new_fd,buffer,1024,0);
          }
        }
        close(new_fd);
        return 0;
      }
    }
  }

  return 0; 
}
