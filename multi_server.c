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
#include <signal.h>

//struct que guarda los valores del nombre y el color de cada cliente
struct Cliente{
  char Nombre[50];
  char color[10];
  int puerto;
  int sockfd;
};

char*COLORS[]={CRED,CGREEN,CYELLOW,CBLUE,CMAGENTA};
struct Cliente OnLine[5];
int cli_st[5];
int p_ctop[5][2], p_ptoc[5][2]; //pipes para comunicar client->parent, parent->client;

int main(int argc, char const *argv[]) 
{
  struct Cliente Cliente1, Cliente2, Cliente3, Cliente4, Cliente5;
  //colores preasignados por posicion, es decir el primero que se conecte siempre sera rojo, el segundo verde, etc
  strcpy(Cliente1.color,CRED);
  strcpy(Cliente2.color,CYELLOW);
  strcpy(Cliente3.color,CBLUE);
  
  //3 para cada uno de los posibles clientes,limite 3 clientes(for now)
  OnLine[0] = Cliente1;
  OnLine[1] = Cliente2;
  OnLine[2] = Cliente3;
  OnLine[3] = Cliente4;
  OnLine[4] = Cliente5;

  for(int i=0;i<5;++i){ 
    cli_st[i]=-1;
    pipe(p_ptoc[i]);
    pipe(p_ctop[i]);
    fcntl(p_ptoc[i][0],F_SETFL, fcntl(p_ptoc[i][0],F_GETFL,0)|O_NONBLOCK); //non block read end
    fcntl(p_ptoc[i][1],F_SETFL, fcntl(p_ptoc[i][1],F_GETFL,0)|O_NONBLOCK); //non block read end
    fcntl(p_ctop[i][0],F_SETFL, fcntl(p_ctop[i][0],F_GETFL,0)|O_NONBLOCK); //non block write end
    fcntl(p_ctop[i][1],F_SETFL, fcntl(p_ctop[i][1],F_GETFL,0)|O_NONBLOCK); //non block read end
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
 
  /**buffer de lectura, y string para montar el mensaje completo**/
  char buffer[1025], sendin[4000],dest[50];
  /**hace fork**/
  pid = fork();
  if(pid<0){return -4;}
  if(pid>0){ 
    /**EL Padre se encarga de escuchar los sockets y procesar el mensaje**/
    for(int i=0;i<5;++i){ close(p_ctop[i][1]); close(p_ptoc[i][0]); } /** cierra los extremos de pipes no usados :1->write - 0->read: **/
    /**manage multiple pipes **/
    for(;;){
      usleep(2000); /**O si no el cpu podrá cocinar huevos**/
      for(int i=0;i<5;++i){
        /**Revisa cada uno de los pipes de lectura**/
        /**A estos escriben los hijos del hijo**/
        if((valread = read(p_ctop[i][0],buffer,1024))<0) {/**<0, no lee**/ continue; } else {
          /**something was actually read**/
          if(strncmp(buffer,"disconected",strlen("disconected"))==0){ /**si un cliente se desconecto**/
            printf("Ja,%d c mamo\n",i);
            cli_st[i]=-10; /**libera el espacio**/
            continue;
          }
          if(cli_st[i]<0) { /**si estaba libre**/
            cli_st[i]=100; /**lo reclama**/
            strncpy(OnLine[i].Nombre,buffer,44); /**es la primera lectura, es el nombre**/
            printf("Nombrado\n");
          } else { /**si no**/
            buffer[valread]='\0';
            //printf("Aqui %s",buffer);
            { /**local j,k**/
              int j,k;
              for(j=0;buffer[j]!=' ';j++) dest[j]=buffer[j];
              dest[j++]='\0';
              for(k=0;buffer[j]!='\0';) buffer[k++]=buffer[j++];
              buffer[k]='\0';
            }
            sprintf(sendin,"%s%s says:: %s\e[30;0m",COLORS[i],OnLine[i].Nombre,buffer); /**arma el mensaje**/
            printf("\t%s\t%s\n",dest,sendin); /**imprime, debug**/
            if(strcmp(dest,"everyone")==0) {
              for(int j=0;j<5;j++) { /**busca al destino**/
                if(j!=i && cli_st[j]>0) write(p_ptoc[j][1],sendin,strlen(sendin)); //escribe en el pipe del destino
              }
            } else {
              for(int j=0;j<5;++j) {
                if(cli_st[j]>0 && strcmp(OnLine[j].Nombre,dest)==0){
                  write(p_ptoc[j][1],sendin,strlen(sendin));
                }
              }
            }
          }
        }
      }
    }
    kill(pid,SIGTERM); /**es el padre, mata a los hijos antes de morir el**/
  } else {
    /**child**/
    /**manage server sockets**/
    int i;
    const char *mess = "Connection Sucesstion\n"; /**Mensaje de conecti+on**/
    while(1){ /**Ciclo principal**/
      if( (new_fd = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) <0){ /**espera nuevas conecciones*/
        perror("accept");
        continue;
      }
      for(i=0;i<5;++i){ /**coneccion nueva, busca un lugar donde ponerla**/
        if(cli_st[i]<0){
          cli_st[i]=10;
          break;
        }
      }
      pid = fork(); /**fork, padre continua escuchando, hijos procesan los clientes**/
      if(pid>0){ //perent
        close(new_fd); //cierra nueva coneccion
      } else { //child
        close(server_fd); //cierra server,eso es del padre
        close(p_ctop[i][0]); //read /**cierra las pipes no necesarias
        close(p_ptoc[i][1]); //write //
        send(new_fd,mess,strlen(mess),0); //envia mensaje de prueba
        if((valread = read(new_fd,buffer,1024))>0){ //espera por respuesta, deberia ser el nombre de usuario
          buffer[valread] = '\0'; //añade un EOL
          write(p_ctop[i][1],buffer,1024); //y lo manda el abuelo para que lo procese, primer mensaje
        }
        /**Ajusta el pipe a no bloqueo: so no hay algo que leer no espera, continua**/
        fcntl(new_fd,F_SETFL,fcntl(new_fd,F_GETFL,0)|O_NONBLOCK);
        for(;;){ //ciclo para procesar los clientes
          usleep(2000); //para que el ventilador no estalle
          if((valread = read(new_fd,buffer,1024))>0){ //intenta leer el socket, <0 significa que no hay nada nuevo
            buffer[valread] = '\0'; //ajusta el EOL
            printf("(1) %s",buffer);
            write(p_ctop[i][1],buffer,1024); //y se lo manda al abuelo para que lo procese
          } else if(valread == 0){ //si es 0 significa que ya no hay nada mas, desconectado, EOF
            /**disconect**/
            strcpy(buffer,"disconected"); //ajusta mensaje
            write(p_ctop[i][1],buffer,strlen(buffer)); //y se lo comunica al abuelo
            printf("Efectivamente, %d c mamo\n",i); //
            break; //cierra el ciclo
          }
          if( (valread = read(p_ptoc[i][0],sendin,4000))>0){ //lee si el abuelo le mandó algo
            sendin[valread] = '\0'; //ajusta el mensaje
            send(new_fd,sendin,strlen(sendin),0); //y lo manda al cliente
          }
        }
        close(new_fd); //cierra el socket
        return 0; //y muere
      }
    }
  }
  return 0;
}
