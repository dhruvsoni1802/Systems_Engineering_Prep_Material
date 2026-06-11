#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <errno.h> 
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_WAITING_CONNECTIONS 100
#define MAX_EVENTS 100
#define BUFFER_SIZE 128

// Register an fd with epoll
int epoll_add(int epoll_fd, int client_fd) {
    struct epoll_event ev;
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
}

//Function to handle a new client connection
void handle_client_connection(int epoll_fd,int listener_fd)
{

  //Accept this connection of the server socket
  int client_fd = accept(listener_fd,NULL,NULL);
  if(client_fd == -1) return;

  //Now we register the new client to the epoll mechanism
  epoll_add(epoll_fd,client_fd);
  printf("This is a new connection from client with fd: %d\n",client_fd);
}

//Function to handle the data received from a client connection
void handle_client_data(int epoll_fd,int client_fd)
{
  char buffer[BUFFER_SIZE];
  while (1) {
      int n = read(client_fd, buffer, sizeof(buffer));

      if (n > 0) {
        //Echo the reply back
        write(client_fd, buffer, n);         
      } 
      else if (n == 0) {
        //Client disconnected cleanly
        printf("Disconnected: fd=%d\n", client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL,client_fd, NULL);
        close(client_fd);
        break;
      } 
      else {
        //no more data right now
        if (errno == EAGAIN) break; 

        //Actual error
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL,client_fd, NULL);
        close(client_fd);
        break;
      }
    }
}

//Function to create a new server side socket
int create_new_socket(void)
{
  //1. We create a new socket - IPv4 and TCP
  int socket_fd = socket(AF_INET,SOCK_STREAM,0);

  //2. When we close a server, we cannot reuse the same port for sometime and we get address already in use error
  //Setting this socket option avoids it
  int opt = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  //Now we bind this socket to an address (IP + port) struct. For that we create a new address struct
  struct sockaddr_in addr = {
        .sin_family      = AF_INET, //IPV4
        .sin_port        = htons(PORT), //Host to network byte order
        .sin_addr.s_addr = INADDR_ANY //Accept any incoming IP Address
  };

  bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));

  //Now the socket is ready to accept incoming connections ( the second parameter is the max waiting connections in the queue)
  listen(socket_fd, MAX_WAITING_CONNECTIONS);
  return socket_fd;
}

int main()
{
  int listener_fd = create_new_socket();

  //Creating a new epoll instance
  int epoll_fd = epoll_create1(0);

  //Adding the listener socket to epoll
  epoll_add(epoll_fd, listener_fd);

  printf("Server running on port %d\n", PORT);

  //An array of structs we will be registering with the epoll
  struct epoll_event events[MAX_EVENTS];

  //Event loop
  while (1) {
      int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
      for (int i = 0; i < n; i++) {
        int fd = events[i].data.fd;
        if (fd == listener_fd) {
          handle_client_connection(epoll_fd, listener_fd);
        }
        else {
          handle_client_data(epoll_fd, fd);
        }
      }
  }

  //Cleanup
  close(listener_fd);
  close(epoll_fd);
  return 0;
}