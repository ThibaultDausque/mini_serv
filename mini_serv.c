#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>

typedef struct	s_cli {
	int		fd;
}	t_cli;

typedef struct s_serv {
	int		sockfd;
	fd_set	master;
	fd_set	rfds;
}	t_serv;

int	write_err(char* err)
{
	int		i = 0;
	while (err[i])
		write(2, &err[i++], 1);
	write(1, "\n", 1);
	return 1;
}

int	init_serv(int port, t_serv* serv)
{
	struct sockaddr_in servaddr;

	// socket create and verification
	serv->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serv->sockfd == -1)
	{
		write_err("Fatal error");
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
		write_err("Fatal error");
		exit(1);
	}
	if (listen(serv->sockfd, 10) != 0)
	{
		write_err("Fatal error");
		exit(1);
	}
	FD_SET(serv->sockfd, &serv->master);
	return serv->sockfd;
}

int	accept_cli(int sockfd)
{
	int		fd;
	struct sockaddr_in c;
	socklen_t	len = sizeof(c);
	fd = accept(sockfd, (struct sockaddr *)&c, &len);
	if (fd < 0)
	{
		write_err("Fatal error");
		exit(1);
	}
	return fd;
}

void	write_mess(char *buff)
{
	int		i = 0;
	while(buff[i])
		write(1, &buff[i++], 1);
}

char*	ft_substr(char *buff, int start, int end)
{
	int		len;
	char*	new;

	len = end - start;
	new = (char*)malloc((len + 1) * sizeof(char));
	if (!new)
		return NULL;
	int		i = 0;
	while (start < end)
		new[i++] = buff[start++];
	new[i] = '\0';
	return new;
}

int	extract_mess(char *buff, int max, fd_set master)
{
	int		i = 0;
	int		j = 0;
	int		flag = 0;

	while (buff[i])
	{
		if (buff[i] == '\n')
		{
			flag = 1;
			j = i;
			char	*new = ft_substr(buff, j, i);
			int		j = 4;
			while (j <= max)
			{
				if (!FD_ISSET(j, &master) || j == max)
				{
					j++;
					continue ;
				}
				char	buff[1024];
				sprintf(buff, "client %d: %s", j - 4, new);
				write_mess(buff);
				write(1, "\n", 1);
				send(j, new, strlen(new), 0);
				j++;
			}
		}
		i++;
	}
	if (!flag)
	{
		int		k = 4;
		while (k <= max)
		{
			if (!FD_ISSET(k, &master) || k == max)
			{
				k++;
				continue ;
			}
			char	buff[1024];
			sprintf(buff, "client %d: %s", k - 4, buff);
			write_mess(buff);
			write(1, "\n", 1);
			send(k++, buff, strlen(buff), 0);
		}
	}
	return 1;
}

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		write_err("Wrong number of arguments");
		exit(1);
	}
	t_serv	serv;
	t_cli	cli[1024];
	int		port = atoi(av[1]);
	int		sockfd = init_serv(port, &serv);
	int		i = 0;
	int		max = sockfd;
	while (i < 1024)
		cli[i++].fd = 0;
	i = 0;
	while (1)
	{
		serv.rfds = serv.master;
		if (!select(1024, &serv.rfds, 0, 0, 0))
		{
			write_err("Fatal error");
			exit(1);
		}
		if (FD_ISSET(sockfd, &serv.rfds))
		{
			int		fd = accept_cli(sockfd);
			int		l = 0;
			while (cli[l].fd != 0 && l < 1024)
				l++;
			cli[l].fd = fd;
			if (fd >= max)
				max = fd;
			FD_SET(cli[l].fd, &serv.master);
			char	buff[1024];
			sprintf(buff, "server: client %d just arrived\n", fd - 4);
			write_mess(buff);
			int		j = 0;
			while(j < 1024)
			{
				if (!FD_ISSET(cli[j].fd, &serv.master) || cli[j].fd == fd)
				{
					j++;
					continue ;
				}
				send(cli[j].fd, buff, strlen(buff), 0);
				j++;
			}
		}
		else
		{
			int		k = 0;
			while (k < 1024)
			{
				if (!FD_ISSET(cli[k].fd, &serv.rfds))
				{
					k++;
					continue ;
				}
				char	buff[1024];
				int		bytes = recv(cli[k].fd, buff, sizeof(buff), 0);
				if (bytes <= 0)
				{
					char	tab[1024];
					sprintf(tab, "server: client %d just left\n", cli[k].fd - 4);
					write_mess(tab);
					FD_CLR(cli[k].fd, &serv.master);
					close(cli[k].fd);
					cli[k].fd = 0;
					int		l = 0;
					while (l < 1024)
					{
						if (!FD_ISSET(cli[l].fd, &serv.master) || FD_ISSET(cli[l].fd, &serv.rfds))
						{
							l++;
							continue ;
						}
						send(cli[l++].fd, tab, strlen(tab), 0);
					}
				}
				// else
				// {
				// 	buff[bytes] = '\0';
				// 	extract_mess(buff, max, serv.master);
				// }
				k++;
			}
		}
	}
}
