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
        printf("client %d connected.\n", i++);
        FD_SET(cli->fd, &master);
    }
    return cli->fd;
}

int start_server(int sockfd)
{
    int     bytes;
    t_cli   cli[1024];
    int     i = 0;
    int     retval;

    while (1)
    {
		FD_SET(sockfd, &master);
		FD_SET(STDIN_FILENO, &master);
		FD_SET(STDOUT_FILENO, &master);
		FD_SET(STDERR_FILENO, &master);
		wfds = master;
		printf("sockfd: %d \n in: %d\n out: %d\n err: %d\n", sockfd, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
		retval = select(1024, &wfds, NULL, NULL, NULL);
		if (!retval)
		{
			printf("select() error\n");
			exit(0);
		}
        i = 0;
        while (i < 1024)
        {
			if (!FD_ISSET(i, &master))
				break ;
			if (i == sockfd)
			{
				accept_cli(sockfd, &cli[i]);
				printf("fd: %d\n", i);
			}
			else
			{
				bytes = recv(i, cli[i].buff, sizeof(cli[i].buff), 0);
				if (bytes <= 0)
				{
					printf("client %d disconnected.", i);
					exit(0);
				}
				else
				{
					printf("client %d: %s\n", i, cli[i].buff);
				}
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
