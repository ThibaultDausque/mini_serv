#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>

typedef struct  cli_s
{
    int     fd;
    int     id;
    char    *buf;
}   cli_t;

typedef struct serv_s
{
    int     sockfd;
    fd_set  master;
    fd_set  rfds;
}   serv_t;

char    send_msg[1000000];

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

void    ft_message(int fd, char *buf)
{
    int     i = 0;

    while (buf[i])
        write(fd, &buf[i++], 1);
    if (fd == 2)
        write(1, "\n", 1);
}

int init_serv(int port, serv_t  *serv)
{
    struct sockaddr_in servaddr;

    // socket create and verification
    serv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv->sockfd == -1)
    {
        ft_message(2, "Fatal error");
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
        ft_message(2, "Fatal error");
        exit(1);
    }
    if (listen(serv->sockfd, 10) != 0)
    {
        ft_message(2, "Fatal error");
        exit(1);
    }
    FD_SET(serv->sockfd, &serv->master);
    return serv->sockfd;
}

int accept_cli(int sockfd)
{
    struct sockaddr_in  cli;
    socklen_t           len;
    int                 connfd;

    len = sizeof(cli);
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        ft_message(2, "Fatal error");
        exit(1);
    }
    return connfd;
}

void    send_to_cli(char *buf, int fd, cli_t *c)
{
    int     i = 0;

    while (i < 1024)
    {
        if (c[i].fd > 0 && c[i].fd != fd)
            send(c[i].fd, buf, strlen(buf), 0);
        i++;
    }
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        ft_message(2, "Wrong number of arguments");
        exit(1);
    }
    serv_t  serv;
    cli_t   c[1024] = {{0}};
    int     i = 0;
    int     port = atoi(av[1]);
    int     sockfd = init_serv(port, &serv);
    int     max = sockfd;
    int     id = 0;
    while (1)
    {
        serv.rfds = serv.master;
        if (!select(max + 1, &serv.rfds, 0, 0, 0))
        {
            ft_message(2, "Fatal error");
            exit(1);
        }
        if (FD_ISSET(sockfd, &serv.rfds))
        {
            int     fd = accept_cli(sockfd);
            if (fd >= max)
                max = fd;
            i = 0;
            while (i < 1024 && c[i].fd != 0) i++;
            c[i].fd = fd;
            c[i].id = id;
            c[i].buf = NULL;
            FD_SET(fd, &serv.master);
            char    out[1024];
            sprintf(out, "server: client %d just arrived\n", id++);
            send_to_cli(out, fd, c);
        }
        else
        {
            int     j = 0;
            while (j < 1024)
            {
                if (c[j].fd > 0 && FD_ISSET(c[j].fd, &serv.rfds))
                {
                    char    msg[1024];
                    int     r = recv(c[j].fd, msg, sizeof(msg) - 1, 0);
                    if (r <= 0)
                    {
                        char    *buf = NULL;
                        c[j].buf = str_join(c[j].buf, msg);
                        while (extract_message(&c[j].buf, &buf))
                        {
                            sprintf(send_msg, "client %d: %s", c[j].id, buf);
                            send_to_cli(send_msg, c[j].fd, c);
                            free(buf);
                            buf = NULL;
                        }
                        free(c[j].buf);
                        c[j].buf = NULL;

                        char    out[1024];
                        c[j].fd = 0;
                        sprintf(out, "server: client %d just left\n", c[j].id);
                        send_to_cli(out, c[j].fd, c);
                        FD_CLR(c[j].fd, &serv.master);
                        close(c[j].fd);
                    }
                    else
                    {
                        msg[r] = '\0';
                        char    *buf = NULL;
        
                        c[j].buf = str_join(c[j].buf, msg);
                        while (extract_message(&c[j].buf, &buf))
                        {
                            sprintf(send_msg, "client %d: %s", c[j].id, buf);
                            send_to_cli(send_msg, c[j].fd, c);
                            free(buf);
                            buf = NULL;
                        }
                        if (!c[j].buf[0])
                        {
                            free(c[j].buf);
                            c[j].buf = NULL;
                        }
                    }
                }
                j++;
            }
        }
    }
}