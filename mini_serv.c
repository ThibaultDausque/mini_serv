#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

typedef struct s_cli
{
    char    buff[1024];
    int     fd;
    int     id;
} t_cli;

fd_set  rfds;
fd_set  wfds;
int     maxfd = 0;

int init_serv(int port)
{
    int     sockfd;
    struct sockaddr_in  servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    if (listen(sockfd, 10) != 0)
    {
        printf("cannot listen\n");
        exit(0);
    }
    FD_SET(sockfd, &rfds);
    FD_SET(sockfd, &wfds);
    return sockfd;
}

int accept_cli(int sockfd, t_cli *cli)
{
    socklen_t           len;
    struct sockaddr_in  client;
    static int          i = 0;

    len = sizeof(cli);
    cli->fd = accept(sockfd, (struct sockaddr *)&client, &len);
    if (cli->fd < 0)
    {
        printf("server accept failed...\n");
        exit(0);
    }
    else
    {
        cli->id = i;
        printf("client %d connected.\n", i++);
        FD_SET(cli->fd, &rfds);
        FD_SET(cli->fd, &wfds);
        if (cli->fd > maxfd)
            maxfd = cli->fd;
    }
    return cli->fd;
}

int start_server(int sockfd)
{
    int     bytes = 0;
    t_cli   cli[1024];
    int     i = 0;
    int     retval;

    while (1)
    {
        i = 0;
        while (i < 1024)
        {
            if (FD_ISSET(i, &rfds))
            {
                if (i == sockfd)
                {
                    accept_cli(sockfd, &cli[i]);
                    retval = select(maxfd + 1, &rfds, &wfds, NULL, NULL);
                }
                else
                {
                    bytes = recv(cli[i].fd, cli[i].buff, sizeof(cli[i].buff), 0);
                    if (bytes <= 0)
                    {
                        printf("client %d: disconnected.\n", cli[i].id);
                        exit(0);
                    }
                    else
                        printf("client %d: %s\n", cli[i].id, cli[i].buff);
                }
                i++;
            }
        }
    }
    return 1;
}

int main(int ac, char **av)
{
    if (ac != 2)
        return 0;
    
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    int     port = atoi(av[1]);
    int     sockfd = init_serv(port);
    start_server(sockfd);
    return 0;
}