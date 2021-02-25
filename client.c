#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

  //Ta emot ip, port and nickname in the following format:
  //ip:port nickname
  if (argc != 3)
  {
    //We dont have the correct input. Exit the program
    printf("Invalid input.\n");
    exit(1);
  }
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  char *Nickname = argv[2];
  //Desthost should now have the ip in Desthost, port in Destport and the nickname in Nickname
  //Check if one of them might be NULL
  if (Desthost == NULL || Destport == NULL || Nickname == NULL)
  {
    //One of the variables are null. Exit the program
    printf("Invalid input.\n");
    exit(1);
  }
  //Variables
  int rv;
  int sock;
  char s[INET6_ADDRSTRLEN];
  char buf[256];

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
    sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock < 0)
    {
      //Could not make socket try again
      continue;
    }
    //If we made it here we succesfully created a socket. Exit loop
    break;
  }
  freeaddrinfo(servinfo);
  //Check if the for loop did not make a socket
  if (p == NULL)
  {
    printf("Faild to create socket.\n");
    exit(1);
  }
  //We now have a socket so we try to connect to the server
  if (connect(sock, p->ai_addr, p->ai_addrlen) < 0)
  {
    //Faild to connect
    printf("Could not connect.\n");
    close(sock);
    exit(1);
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("client: connecting to %s\n", s);
  //We are now connected to the server
  //First we need to recive a message from the server Ex: Hello 1
  //Send back my nickname
  //Recive an OK or ERR
  
  //In this stage we can send and recive messages to/from the server
  //we need to check the input from the terminal and send it
  //At the same time we need to recive messages and print them.
  //To be able to boath read from STDIN and the socket
  return 0;
}
