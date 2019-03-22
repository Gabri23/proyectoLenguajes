#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <signal.h>

int pid = -1;

int main(int argc, char const *argv[]) 
{
  char nombre[50];
  int PORT = 0, mPORT = 12020;
  char *ip_serv;
  if(argc < 3){
    printf("usge: %s ip-ser port-ser port-cli\n",argv[0]);
    return -1;
  } else {
    mPORT = atoi(3[argv]);
    PORT = atoi(2[argv]);
    ip_serv = argv[1];
  }
  //printf("%d\n",PORT);
  struct sockaddr_in address; 
  int sock = 0, valread; 
  struct sockaddr_in serv_addr; 
  char *hello = "Hello from client"; 
  char buffer[802], dest[50], sendin[1024]; 
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
    printf("\n Socket creation error \n");
    return -1;
  }
  
  memset(&serv_addr, '0', sizeof(serv_addr)); 
   
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_port = htons(PORT); 
  
  // Convert IPv4 and IPv6 addresses from text to binary form 
  if(inet_pton(AF_INET, ip_serv, &serv_addr.sin_addr)<=0) { 
    printf("\nInvalid address/ Address not supported \n"); 
    return -1; 
  } 

  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY; 
  address.sin_port = htons(mPORT);
  if (bind(sock, (struct sockaddr*) &address, sizeof(struct sockaddr_in)) == 0){
    printf("Binded Correctly\n");
  } else {
    printf("Unable to bind\n");
    close(sock);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
    printf("\nConnection Failed \n"); 
    close(sock);
    return -1; 
  }
  printf("Connected\n");

  valread = read( sock, buffer, 1024); 
  printf("%s\n",buffer);
  
  printf("Indique un nombre de usuario: ");
  scanf("%s",nombre);
  send(sock,nombre,strlen(nombre),0);
  printf("User: %s\n",nombre);

  pid = fork();
  if(pid<0){return -4;}
  if(pid==0){ /**child, listen**/
    for(;;){ /**este ciclo escucha al socket**/
      if((valread=read(sock,buffer,1024))>0){ /**si recibe algo**/
        buffer[valread]='\0'; /**lo imprime **/
        printf(":: %s\n",buffer);
      } else if(valread==0){ /**esto no deberia entrar**/
        printf("ya se acabo\n");
      }
    }
  } else { /**parent, send**/
    for(;;){ /**este ciclo lee la stdin y envia mensajes**/
      scanf("%s",dest); /**primero lee el destinatario**/
      if(strcmp(dest,"quit")==0){break;} /**si es quit cierra el programa**/
      //dest[valread]='\0';
      while(scanf("%800[^\n]",buffer)){ /**mientras haya algo mas que leer, resto del mensaje**/
        if(strcmp(buffer," ")==0) continue; /**si es vacio lo ingora, espacio al final**/
        sprintf(sendin,"%s %s",dest,buffer); /**arma el mensaje**/
        printf("\t\t(snd): %s\n", sendin); /**imprime, debug**/
        //send(sock,sendin,strlen(sendin),0); /**envia el mensaje**/
        usleep(2000); /**un sleep para que el server no se caliente**/ //se podria omitir
      }
    }
    kill(pid,SIGTERM); /**termina y mana al hijo, el otro**/
    close(sock); /**cierra el soket**/
  }
  return 0; 
} 
