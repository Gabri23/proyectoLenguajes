// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 

int main(int argc, char const *argv[]) 
{
  char *nombre = "";
  int PORT = 0, mPORT = 12020;
  char *ip_serv = "";
  if(argc < 3){
    printf("usge: %s ip-ser port-ser port-cli\n",argv[0]);
    return -1;
  } else {
    mPORT = atoi(3[argv]);
    PORT = atoi(2[argv]);
    ip_serv = 1[argv];
  }
  //printf("%d\n",PORT);
  struct sockaddr_in address; 
  int sock = 0, valread; 
  struct sockaddr_in serv_addr; 
  char *hello = "Hello from client"; 
  char buffer[1024] = {0}; 
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

  valread = read( sock, buffer, 1024); 
  printf("%s\n",buffer);
  
  printf("Indique un nombre de usuario: ");
  scanf("%s",nombre);
  send(sock,nombre,strlen(nombre),0);


  for(;;){
    scanf("%s",buffer);
    if(strcmp(buffer,"quit")==0){break;}
    do{
      printf("to:sent:\t%s\n", buffer);
      send(sock,buffer,strlen(buffer),0);
      valread = read(sock,buffer,1024);
      printf("recived::\t\t%s\n", buffer);
    } while(scanf("%126[^\n]",buffer));
  }
  close(sock);
  return 0; 
} 
