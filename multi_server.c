#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/types.h>  

//struct que guarda los valores del nombre y el color de cada cliente
struct Cliente{
  char Nombre[50];
  int color;
  int puerto;
};

//funcion que maneja la comunicacion entre los sockets
int clientMg(int socket_fd, struct Cliente *OnLine, int *pipefd)
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
  /**********************************************************/
  /** ESTO NO VA A FUNCIONAR **/
  /** STRING NUNCA SERÄ UN NULL **/
  //asignar el nombre a cada cliente en la lista, revisa si el nombre es NULL para hacerlo, nombre es el array de chars que se crea dentro
  //de la funcion, Nombre es el atributo del struct
  if(OnLine[0].Nombre == NULL) {
     strcpy(OnLine[0].Nombre,nombre);
     printf("Cliente registrado%s\n",nombre);
  } else if(OnLine[1].Nombre == NULL) {
     strcpy(OnLine[1].Nombre,nombre);
     printf("Cliente registrado%s\n",nombre);
  } else if(OnLine[2].Nombre == NULL) {
     strcpy(OnLine[2].Nombre,nombre);
     printf("Cliente registrado%s\n",nombre);
  } else {
     printf("Error: max concurrent conections reached");
     printf("Details: source client%s\n",nombre);
     return -10;
  }
  /********************************************************/
  for(;;) {
    if( (valread = read(socket_fd, buffer, 1024) ) == 0 ) {
      /** disconnect **/
      printf("Cliente desconectado %d\n",socket_fd);
      break;
    } else {
      buffer[valread] = '\0';
      printf("Recived From Client:\t%s\n",buffer);
      send(socket_fd,buffer,strlen(buffer),0);
      //envia el mensaje a todas los cliente que no tienen nombre NULL
      for (int controlVar = 0; controlVar > 3; controlVar++)
      {
        if (OnLine[controlVar].Nombre != NULL) {
          write(pipefd[controlVar],buffer, strlen(buffer));
        }
      }
    }
  }
  return 0;
}

int main(int argc, char const *argv[]) 
{
  struct Cliente Cliente1;
  struct Cliente Cliente2;
  struct Cliente Cliente3;

  //colores preasignados por posicion, es decir el primero que se conecte siempre sera rojo, el segundo verde, etc
  Cliente1.color = 0xFF0000;
  Cliente2.color = 0x0000FF;
  Cliente3.color = 0x00FF00;
  
  //3 para cada uno de los posibles clientes,limite 3 clientes(for now)
  struct Cliente OnLine[3];
  OnLine[0] = Cliente1;
  OnLine[1] = Cliente2;
  OnLine[2] = Cliente3;

  //los pipes siempre tiene que ser uno mas que la cantidad de clientes para contar el server
  int pipefd[4];
  //inicializar los pipes, IMPORTANTE
  pipe(pipefd);
  
  int PORT = 0;
  if(argc<2) {
    printf("usage: %s port\n",argv[0]);
    return -1;
  } else {
    PORT = atoi(argv[1]);
  }
  int server_fd, new_fd; 
  struct sockaddr_in address; 
  int opt = 1, addrlen = sizeof(address), pid;
  
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
      //para evitar duplicaciones del server
      if(pid==0) {
        /**client**/
        close(server_fd);
        //llamada a la funcion de manejo de socket
        clientMg(new_fd, OnLine,pipefd);
      } else {
        close(new_fd);
      }
    }
  } /**for**/
  return 0; 
}
