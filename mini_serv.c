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
fd_set  master;
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
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    if (listen(sockfd, 1024) != 0)
    {
        printf("cannot listen\n");
        exit(0);
    }
    if (sockfd > maxfd)
        maxfd = sockfd;
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
        printf("client %d just arrived.\n", i++);
        FD_SET(cli->fd, &master);
    }
    return cli->fd;
}

int start_server(int sockfd)
{
    int     bytes;
    t_cli   cli[1024];
    int     i = 0;
    int     j = 0;
    int     retval;

    while (1)
    {
		FD_SET(sockfd, &master);
		rfds = master;
		retval = select(1024, &rfds, NULL, NULL, NULL);
		if (!retval)
		{
			printf("select() error\n");
			exit(0);
		}
        i = 0;
        if (FD_ISSET(sockfd, &rfds))
        {
            if (accept_cli(sockfd, &cli[j]))
            {
                int     k = 0;
                while (k != j)
                {
                    char    buff[1024];
                    sprintf(buff, "client %d just arrived.\n", cli[j].id);
                    send(cli[k].fd, buff, sizeof(buff), 0);
                    k++;
                }
            }
            j++;
        }
        while (i <= j)
        {
            if (FD_ISSET(cli[i].fd, &rfds))
            {
                char    buff[1024];
                bytes = recv(cli[i].fd, buff, sizeof(buff), 0);
                int     k = 0;
                while (k != j)
                {
                    char    mess[1024];
                    if (k != i)
                    {
                        sprintf(mess, "client %d: %s", cli[i].id, buff);
                        send(cli[k].fd, mess, sizeof(mess), 0);
                    }
                    k++;
                }
                if (bytes <= 0)
                {
                    printf("client %d disconnected.", cli[i].id);
                    close(cli[i].fd);
                }
                else
                    printf("client %d: %s\n", i, buff);
            }
            i++;
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
