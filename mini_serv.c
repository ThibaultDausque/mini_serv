#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>

typedef struct s_cli {
    int     fd;
    int     id;
}   t_cli;

typedef struct s_serv {
    int     sockfd;
    t_cli   cli[1024];
    fd_set  master;
    fd_set  rfds;
}   t_serv;

int max = 0;
int init_serv(int port, t_serv *serv)
{
  int sockfd;
  struct sockaddr_in servaddr;

  // socket create and verification
  serv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serv->sockfd == -1)
  {
    printf("socket creation failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
  servaddr.sin_port = htons(port);

  // Binding newly created socket to given IP and verification
  if ((bind(serv->sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
  {
    printf("socket bind failed...\n");
    exit(0);
  }
  FD_SET(serv->sockfd, &serv->master);
  max++;
  return serv->sockfd;
}

int accept_cli(t_cli *cli, int sockfd, t_serv *serv)
{
    socklen_t     len;
    struct sockaddr_in  client;

    len = sizeof(client);
    cli->fd = accept(sockfd, (struct sockaddr *)&client, &len);
    if (cli->fd < 0)
    {
        printf("server accept failed...\n");
        exit(1);
    }
    else
    {
        printf("server accept client...\n");
        max++;
        FD_SET(cli->fd, &serv->master);
    }
    return cli->fd;
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        printf("too much args...\n");
        exit(1);
    }
    t_serv  serv;
    t_cli   cli[1024];

    int     i = 0;
    int     port = atoi(av[1]);
    int     sockfd = init_serv(port, &serv);
    while (1)
    {
        serv.rfds = serv.master;
        int nfd = select(max + 1, &serv.rfds, 0, 0, 0);
        if (FD_ISSET(serv.sockfd, &serv.rfds))
        {
            accept_cli(&cli[i], sockfd, &serv);
            cli[i].id = cli[i].fd - 4;
            i++;
        }
    }
}