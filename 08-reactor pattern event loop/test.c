#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_QUEUE 10

//Function to create a server side socket
int create_socket(void)
{
  //IPv4 and TCP socket
  int socket_fd = socket(AF_INET,SOCK_STREAM,0);

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = PORT,
    .sin_addr.s_addr = INADDR_ANY
  };

  int bind_ret = bind(socket_fd,(struct sockaddr *)&addr,sizeof(addr));

  if(bind_ret == -1)
  {
    return -1;
  }

  int listen_ret = listen(socket_fd,MAX_QUEUE);
}

int main()
{
  return 0;
}