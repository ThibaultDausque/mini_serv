#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct  serv_s
{
  int   sockfd;
  fd_set  master;
  fd_set  rfds;
} serv_t;

void  ft_output(int fd, char *buff)
{
  int   i = 0;
  while (buff[i])
    write(fd, &buff[i++], 1);
  if (fd == 2)
    write(1, "\n", 1);
}

int init_serv(int port, serv_t *serv)
{
  struct sockaddr_in servaddr;

  // socket create and verification
  serv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serv->sockfd == -1)
  {
    ft_output(2, "Fatal error");
    exit(1);
  }
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
  servaddr.sin_port = htons(port);

  // Binding newly created socket to given IP and verification
  if ((bind(serv->sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
  {
    ft_output(2, "Fatal error");
    exit(1);
  }
  if (listen(serv->sockfd, 10) != 0)
  {
    ft_output(2, "Fatal error");
    exit(1);
  }
  FD_SET(serv->sockfd, &serv->master);
  return serv->sockfd;
}

void send_to_client(char *buff, serv_t *serv, int *cli)
{
  int   i = 0;
  while (i < 1024)
  {
    if (FD_ISSET(cli[i], &serv->master) && !FD_ISSET(cli[i], &serv->rfds))
      send(cli[i], buff, strlen(buff), 0);
    i++;
  }
}

int accept_cli(int sockfd, serv_t *serv, int *clifd)
{
  socklen_t len;
  struct sockaddr_in  cli;
  int   connfd;

  len = sizeof(cli);
  connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
  if (connfd < 0)
  {
    ft_output(2, "Fatal error");
    exit(1);
  }
  char  tab[1024];
  sprintf(tab, "server: client %d just arrived\n", connfd - 4);
  ft_output(1, tab);
  send_to_client(tab, serv, clifd);
  return connfd;
}
int extract_message(char **buf, char **msg)
{
  char *newbuf;
  int i;

  *msg = 0;
  if (*buf == 0)
    return (0);
  i = 0;
  while ((*buf)[i])
  {
    if ((*buf)[i] == '\n')
    {
      newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
      if (newbuf == 0)
        return (-1);
      strcpy(newbuf, *buf + i + 1);
      *msg = *buf;
      (*msg)[i + 1] = 0;
      *buf = newbuf;
      return (1);
    }
    i++;
  }
  return (0);
}

char *str_join(char *buf, char *add)
{
  char *newbuf;
  int len;

  if (buf == 0)
    len = 0;
  else
    len = strlen(buf);
  newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
  if (newbuf == 0)
    return (0);
  newbuf[0] = 0;
  if (buf != 0)
    strcat(newbuf, buf);
  free(buf);
  strcat(newbuf, add);
  return (newbuf);
}

int main(int ac, char **av)
{
  if (ac != 2)
  {
    ft_output(2, "Wrong number of arguments");
    exit(1);
  }
  serv_t  serv;
  int   cli[1024];
  int   i = 0;
  while (i < 1024)
    cli[i++] = 0;
  int   port = atoi(av[1]);
  int   sockfd = init_serv(port, &serv);
  int   max = sockfd;
  while (1)
  {
    serv.rfds = serv.master;
    if (!select(max + 1, &serv.rfds, 0, 0, 0))
    {
      ft_output(2, "Fatal error");
      exit(1);
    }
    if (FD_ISSET(sockfd, &serv.rfds))
    {
      int   fd = accept_cli(sockfd, &serv, cli);
      if (fd >= max)
        max = fd;
      i = 0;
      while (cli[i] != 0 && i < 1024)
        i++;
      cli[i] = fd;
      FD_SET(fd, &serv.master);
    }
    else
    {
      int   j = 0;
      while (j < 1024)
      {
        if (FD_ISSET(cli[j], &serv.rfds))
        {
          char  *buff = NULL;
          int   bytes = recv(cli[j], buff, sizeof(char), 0);
          buff[bytes] = '\0';
          if (bytes <= 0)
          {
            char  tab[1024];
            sprintf(tab, "server: client %d just left\n", cli[j] - 4);
            ft_output(1, tab);
            send_to_client(tab, &serv, cli);
            FD_CLR(cli[j], &serv.master);
            close(cli[j]);
            cli[j] = 0;
          }
          else
          {
            char    *buf = NULL;
            while (extract_message(&buf, &buff))
            {
                char    *join = NULL;
                join = str_join(join, buf);
                ft_output(1, join);
                send_to_client(join, &serv, cli);
            }
          }
        }
        j++;
      }
    }
  }
}