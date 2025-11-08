#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR "Fatal error\n"

int			servfd;

typedef struct s_client {
	int		fd;
	int		pos;
	char	msg[1024];
}	t_client;

t_client*	mini_serv(char* port)
{
	struct sockaddr_in	servaddr, cliaddr;
	int			clifd;
	socklen_t	len = sizeof(servaddr);
	char		tab[1024];
	t_client	cli[1024];
	static int	i = 0;
	static int	k = 0;


	servfd = socket(AF_INET, SOCK_STREAM, 0);
	if (servfd == -1)
	{
		write(0, ERROR, sizeof(ERROR));
		exit(1);
	}
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(servfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		write(0, ERROR, strlen(ERROR));
		close(servfd);
		printf("toto");
		exit(1);
	}
	if (listen(servfd, 50) == -1)
	{
		write(0, ERROR, strlen(ERROR));
		close(servfd);
		exit(1);
	}
	cli[i].pos = k++;
	cli[i].fd = accept(servfd, (struct sockaddr *)&cliaddr, &len);
	i++;
	if (clifd == -1)
	{
		write(0, ERROR, strlen(ERROR));
		close(servfd);
		exit(1);
	}
	else
	{
		sprintf(tab, "server: client %d just arrived\n", clifd);
		write(1, tab, strlen(tab));
	}
	return ;
}

int	run_server(char* port)
{
	int		clifd = -1;
	char	buff[1024];
	int		bytes = 0;
	int		pos = 1;
	t_client	*cli;

	printf("--- mini_serv ---\n");
	printf("\n");
	int		i;
	while (1)
	{
		if (clifd == -1)
		{
			cli = mini_serv(port);
		}
		i = 0;
		while (i < pos)
		{
			bytes = recv(clifd, buff, sizeof(buff) - 1, 0);
			if (bytes <= 0 && clifd != -1)
			{
				char	err[30];
				sprintf(err, "server: client %d just left\n", clifd);
				close(clifd);
				return 1;
			}
			else
			{
				char	tab[1024];
				sprintf(tab, "client %d: %s\n", clifd, buff);
				write(1, tab, strlen(tab));
			}
			i++;
		}
	}
	close(clifd);
	close(servfd);
	return 1;
}

int	main(int ac, char **av)
{
	(void) ac;
	char*	port = av[1];
	run_server(port);
	return 0;
}