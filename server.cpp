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

#define MAXCLIENTS 100

using namespace std;

string removeWord(string str, string word)  
{ 
    // Check if the word is present in string 
    // If found, remove it using removeAll() 
    if (str.find(word) != string::npos) 
    { 
        size_t p = -1; 
  
        // To cover the case 
        // if the word is at the 
        // beginning of the string 
        // or anywhere in the middle 
        string tempWord = word + " "; 
        while ((p = str.find(word)) != string::npos) 
            str.replace(p, tempWord.length(), ""); 
  
        // To cover the edge case 
        // if the word is at the 
        // end of the string 
        tempWord = " " + word; 
        while ((p = str.find(word)) != string::npos) 
            str.replace(p, tempWord.length(), ""); 
    } 
  
    // Return the resultant string 
    return str; 
} 

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
  int yes = 1;
  string cmds[2] = {"NICK ", "MSG "};
  int bytesRecived;
  string nicknames[MAXCLIENTS];

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
  if (listen(listener, MAXCLIENTS) < 0)
  {
    printf("Litse.\n");
    close(listener);
    exit(1);
  }

  fd_set master;
  FD_ZERO(&master);
  FD_SET(listener, &master);
  int fdmax = listener;

  char buf[10000];
  memset(buf, 0, sizeof(buf));

  while (true)
  {
    fd_set copy = master;

    if (select(fdmax + 1, &copy, NULL, NULL, NULL) == -1)
    {
      printf("select.\n");
      close(listener);
      exit(1);
    }

    for (int i = 0; i < fdmax + 1; i++)
    {
      if (FD_ISSET(i, &copy))
      {
        if (i == listener)
        {
          //Accept a new connection
          newfd = accept(listener, p->ai_addr, &p->ai_addrlen);
          if (newfd < 0)
          {
            printf("Accept.\n");
            continue;
          }
          else
          {
            FD_SET(newfd, &master);
            nrOfClient++;
            if (newfd > fdmax)
            {
              fdmax = newfd;
            }
            if (send(newfd, "HELLO 1\n", sizeof("HELLO 1\n"), 0) < 0)
            {
              printf("Sending.\n");
            }
          }
        }
        else
        {
          //Accept a new message
          memset(buf, 0, sizeof(buf));
          bytesRecived = recv(i, buf, sizeof(buf), 0);
          if (bytesRecived <= 0)
          {
            printf("Remove client.\n");
            nicknames[i] = "";
            close(i);
            FD_CLR(i, &master);
          }
          if (string(buf).find(cmds[0]) != string::npos)
          {
            //Found "NICK "
            printf("NICK.\n");
            /*char delim2[] = " ";
            char *nick = strtok(buf, delim2);
            nick = strtok(NULL, delim2);*/

            string nick = removeWord(string(buf), "NICK");
            nick = removeWord(nick, "\n");
            printf("%s\n", nick.c_str());
            for (int s = 0; s < MAXCLIENTS; s++)
            {
              if (nick == nicknames[s])
              {
                printf("Nickname in use.\n");
                send(i, "ERR\n", sizeof("ERR\n"), 0);
                close(i);
                FD_CLR(i, &master);
              }
            }
            nicknames[i] = nick;
            if (send(i, "OK\n", sizeof("OK\n"), 0) < 0)
            {
              printf("Remove client.\n");
              close(i);
              FD_CLR(i, &master);
            }
          }
          else if (string(buf).find(cmds[1]) != string::npos)
          {
            //Found "MSG "

            printf("MSG.\n");
            string msg = removeWord(string(buf), "MSG");
            msg = nicknames[i]+": "+msg;
            printf("MSG: %s\n",msg.c_str());
            for (int k = 0; k <= fdmax; k++)
            {
              if (FD_ISSET(k, &master))
              {
                if (k != listener && k != i)
                {
                  if (send(k, msg.c_str(), msg.length(), 0) < 0)
                  {
                    printf("Send.\n");
                  }
                }
              }
            }
          }
          //Check what to do (MSG or NICK)
          //If its a nickname, check it and sed a responce.
          //If its a message, send the message to all clients, exept the one who sent it
          printf("New message.\n");
        }
      }
    }
  }
  return 0;
}