// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
//#define PORT 8888 
   
int main(int argc, char const *argv[]) 
{
  int PORT = 0;
  char *ip_serv = "";
  if(argc < 3){
    printf("usge: client ip-ser port-ser\n");
    return -1;
  } else {
    PORT = atoi(2[argv]);
    ip_serv = 1[argv];
  }
  printf("%d\n",PORT);
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
   
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
    printf("\nConnection Failed \n"); 
    return -1; 
  }

  valread = read( sock, buffer, 1024); 
  printf("%s\n",buffer);
    
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

  return 0; 
} 
