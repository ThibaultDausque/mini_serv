#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
# include <stdlib.h>
# include <stdio.h>

# define ERROR "Fatal Error"

typedef struct s_cli {
    int     fd;
    int     id;
}   t_cli;

typedef struct s_serv {
    fd_set  master;
    fd_set  rfds;
    fd_set  wfds;
    int     nfds;
    int     sockfd;
}   t_serv;

int init_serv(t_serv *serv, int port)
{
    struct sockaddr_in  servaddr;

    serv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv->sockfd == -1)
    {
        printf("tutu");
        write(0, ERROR, strlen(ERROR));
        exit(0);
    }
    if (serv->sockfd > serv->nfds)
        serv->nfds = serv->sockfd;
    FD_SET(serv->sockfd, &serv->master);
    FD_SET(serv->sockfd, &serv->rfds);
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(port);

    // Binding newly created socket to given IP and verification
    if ((bind(serv->sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("toto");
        write(0, ERROR, strlen(ERROR));
        exit(0);
    }
    if (listen(serv->sockfd, 1024) != 0)
    {
        write(0, ERROR, strlen(ERROR));
        exit(0);
    }
    return 1;
}

int accept_cli(t_serv *serv, t_cli *cli)
{
    struct sockaddr_in  client;
    socklen_t   len = sizeof(client);
    int         clifd;

    clifd = accept(serv->sockfd, (struct sockaddr *)&client, &len);
    cli->fd = clifd;
    if (clifd < 0)
    {
        write(0, ERROR, strlen(ERROR));
        exit(0);
    }
    else
    {
        if (clifd > serv->nfds)
            serv->nfds = cli->fd;
        cli->id = clifd - 4;
        char    tab[1024];
        sprintf(tab, "server: client %d arrived.\n", cli->fd);
        // int     i = 0;
        FD_SET(clifd, &serv->master);
        FD_SET(clifd, &serv->rfds);
        // while (i < cli->id)
        // {
        //     send(i + 4, &tab, sizeof(tab), 0);
        //     i++;
        // }
    }
    return clifd;
}

int main(int ac, char **av)
{
    t_serv  serv;
    t_cli   cli[1024];

    if (ac != 2)
        exit(0);
    int     port = atoi(av[1]);
    int     j = 0;

    serv.nfds = 0;
    init_serv(&serv, port);
    while (1)
    {
        serv.rfds = serv.master;
        select(serv.nfds, &serv.rfds, 0, 0, 0);
        printf("serv socket: %d\n", serv.sockfd);
        printf("nfds: %d\n", serv.nfds);
        if (FD_ISSET(serv.sockfd, &serv.rfds))
            accept_cli(&serv, &cli[j++]);
        printf("a client joined !\n");
    }
    return 0;
}