#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <unistd.h>

#define MAXCLIENTS 6;

int main(int argc, char *argv[])
{

  //Ta emot ip, port and nickname in the following format:
  //ip:port nickname
  if (argc != 2)
  {
    //We dont have the correct input. Exit the program
    printf("Invalid input.\n");
    exit(1);
  }
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  //Desthost should now have the ip in Desthost and port in Destport
  //Check if one of them might be NULL
  if (Desthost == NULL || Destport == NULL)
  {
    //One of the variables are null. Exit the program
    printf("Invalid input.\n");
    exit(1);
  }

  //Variables
  int listener;
  int newfd;
  int rv;
  int nrOfClient = 0;

  struct addrinfo hint, *servinfo, *p;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;     //IPv4 or IPv6
  hint.ai_socktype = SOCK_STREAM; //TCP

  rv = getaddrinfo(Desthost, Destport, &hint, &servinfo);
  if (rv != 0)
  {
    //Getaddrinfo faild
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  //Loop trough all the information and try to make a sockt
  for (p = servinfo; p != NULL; p->ai_next)
  {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0)
    {
      //Could not make socket try again
      continue;
    }
    //setsockoptions
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      printf("setsockopt.\n");
      exit(1);
    }
    //bind
    if (bind(listener, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(listener);
      printf("Could not bind.\n");
      continue;
    }
    //If we made it here we succesfully created a socket. Exit loop
    break;
  }
  freeaddrinfo(servinfo);
  //Check if the for loop did not make a socket
  if (p == NULL)
  {
    printf("Could not bind.\n");
    exit(1);
  }
  //Litser for connections
  if(listen(listener, MAXCLIENTS) < 0){
    printf("Litse.\n");
    close(listener);
    exit(1);
  }
  
  fd_set master;
  fd_set read_fds;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0, &master);
  FD_SET(sock, &master);
  int nfds = litsener;

  while(true){
    read_fds = master;

    if(select(nfds+1, read_fds, NULL, NULL, NULL) == -1){
      printf("Select.\n");
      close(listener);
      exit(1);
    }

    for(int i = 0; i < nfds; i++){
      if(FD_ISSET(i, &read_fds)){
        if(i == listener){
          
        }
      }
    }

  }
  return 0;
}
