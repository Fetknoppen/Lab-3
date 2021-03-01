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
#include <iostream>
#include <sys/select.h>
#include <sys/time.h>

using namespace std;

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

  if (strlen(Nickname) > 12)
  {
    printf("Nickname is too long.\n");
    exit(1);
  }
  //Variables
  int rv;
  int sock;
  char s[INET6_ADDRSTRLEN];
  char buf[256];
  int numbytes;
  string userInput;

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
  //First we need to recive a message from the server with ist protocol. Ex "Hello 1.0\n"
  memset(buf, 0, sizeof(buf));
  numbytes = recv(sock, buf, sizeof(buf), 0);
  if (numbytes < 0)
  {
    //Did not recive
    printf("Could not recive.\n");
    close(sock);
    exit(1);
  }
  printf("Server: %s\n", buf);
  if (strcmp("HELLO 1\n", buf) == 0)
  {
    //We support this protocol
    printf("Protocol supported.\n");
  }
  else
  {
    //we dont support this protocol
    printf("Protocol NOT supported.\n");
    close(sock);
    exit(1);
  }
  //Send back my nickname

  printf("Sending nickname: %s\n", Nickname);
  string nick = "NICK " + string(Nickname) + "\n";

  numbytes = send(sock, nick.c_str(), nick.length(), 0);
  if (numbytes < 0)
  {
    //Did not send
    printf("Could not send.\n");
    close(sock);
    exit(1);
  }
  //Recive an OK or ERR
  memset(buf, 0, sizeof(buf));
  numbytes = recv(sock, buf, sizeof(buf), 0);
  if (numbytes < 0)
  {
    //Did not recive
    printf("Could not recive.\n");
    close(sock);
    exit(1);
  }
  printf("Server: %s\n", buf);
  if (strcmp("ERR\n", buf) == 0)
  {
    //Cant use that nickname
    printf("Cant use that nickname.\n");
    close(sock);
    exit(1);
  }
  printf("Nickname ok.\n");

  fd_set master;
  fd_set read_fds;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0, &master);
  FD_SET(sock, &master);
  int nfds = sock;
  int r;

  //In this stage we can send and recive messages to/from the server
  while (true)
  {
    read_fds = master;

    if (select(nfds + 1, &read_fds, NULL, NULL, NULL) == -1)
    {
      printf("select.\n");
      close(sock);
      exit(1);
    }
    if (FD_ISSET(nfds, &read_fds))
    {
      //Got something from server
      memset(buf, 0, sizeof(buf));
      numbytes = recv(sock, buf, sizeof(buf), 0);
      if (numbytes < 0)
      {
        printf("Could not recive.\n");
        close(sock);
        exit(1);
      }
      //Was this msg from me?
      int ok = 0;
      for(int i = 0; i < strlen(Nickname); i++){
        if(buf[i] == Nickname[i]){
          ok++;
        }
      }
      if(ok != strlen(Nickname)){
        printf("%s.\n", buf);
      }
      
    }
    if (FD_ISSET(0, &read_fds))
    {
      //Client is writing something
      string temp;
      getline(cin, temp);
      userInput = "MSG " + temp;
      if (strlen(userInput.c_str()) > 2255)
      {
        printf("Message is too long.\n");
      }
      else
      {
        //printf("Sending: %s\n", userInput.c_str());
        numbytes = send(sock, userInput.c_str(), userInput.length(), 0);
        if (numbytes < 0)
        {
          printf("Could not send.\n");
          close(sock);
          exit(1);
        }
      }
    }
  }

  return 0;
}
