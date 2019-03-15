//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/types.h>  

int clientMg(int socket_fd){
  printf("cliente connectado %d\n", socket_fd);
  char buffer[1024];
  const char *message = "Coneccion Establecida\n\0";
  char nombre[50] = "";
  int valread;
  if(send(socket_fd, message, strlen(message),0)!=strlen(message)) {
    perror("send");
    return -1;
  }
  read(socked_fd,nombre,50);
  for(;;) {
    if( (valread = read(socket_fd, buffer, 1024) ) == 0 ) {
      /** disconnect **/
      printf("Cliente desconectado%d\n",socket_fd);
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
  int PORT = 0;
  if(argc<2){
    printf("usage: %s port\n",argv[0]);
    return -1;
  } else {
    PORT = atoi(argv[1]);
  }
  int server_fd, new_fd; 
  struct sockaddr_in address; 
  int opt = 1, addrlen = sizeof(address), pid;
  
  // Creating socket file descriptor 
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
    perror("socket failed"); 
    exit(EXIT_FAILURE); 
  } 
  
  // Forcefully attaching socket to the port 8080 
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
    perror("setsockopt"); 
    exit(EXIT_FAILURE); 
  } 
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY; 
  address.sin_port = htons(PORT); 
  
  // Forcefully attaching socket to the PORT
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
    perror("bind failed"); 
    exit(EXIT_FAILURE); 
  }
  
  if(listen(server_fd, 5)<0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  
  for(;;) {
    if( (new_fd = accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen)) <0) {
      //perror("acceptar nueva");
      return -1;
    } else {
      pid = fork();
      if(pid<0) {
        perror("fork");
        continue;
      }
      if(pid==0) {
        /**client**/
        close(server_fd);
        clientMg(new_fd);
      } else {
        close(new_fd);
      }
    }
  } /**for**/
  return 0; 
}
