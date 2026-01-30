#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct cli_s
{
    int     fd;
    int     id;
    char    *buff;
}   cli_t;

typedef struct serv_s
{
    int     sockfd;
    fd_set  master;
    fd_set  rfds;
}   serv_t;

char    send_buff[1000000];

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

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
            free(*buf);
            *buf = NULL;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

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

void    ft_message(int fd, char *buff)
{
    int     i = 0;
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
	if (serv->sockfd == -1) { 
        ft_message(2, "Fatal error");
        exit(1);
	} 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(serv->sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        ft_message(2, "Fatal error");
        exit(1);
	} 
	if (listen(serv->sockfd, 10) != 0) {
        ft_message(2, "Fatal error");
        exit(1);
	}
    FD_SET(serv->sockfd, &serv->master);
    return serv->sockfd;
}

int accept_cli(int sockfd)
{
    struct sockaddr_in  cli;
    socklen_t   len;
    int         connfd;

	len = sizeof(cli);
	connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
	if (connfd < 0) { 
        ft_message(2, "Fatal error");
        exit(1);
    } 
    return connfd;
}

int send_to_c(char *buff, int fd, cli_t *c)
{
    int     i = 0;

    while (i < 1024)
    {
        if (c[i].fd > 0 && c[i].fd != fd)
            send(c[i].fd, buff, strlen(buff), 0);
        i++;
    }
    return 1;
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        ft_message(2, "Fatal error");
        exit(1);
    }
    serv_t  serv;
    cli_t   c[1024];
    int     port = atoi(av[1]);
    int     sockfd = init_serv(port, &serv);
    int     max = sockfd;
    int     i = 0;
    while (i < 1024) c[i++].fd = 0;
    int     id = 0;
    while (1)
    {
        serv.rfds = serv.master;
        select(max + 1, &serv.rfds, 0, 0, 0);
        if (FD_ISSET(sockfd, &serv.rfds))
        {
            int     fd = accept_cli(sockfd);
            if (fd >= max)
                max = fd;
            i = 0;
            while (i < 1024 && c[i].fd != 0) i++;
            c[i].fd = fd;
            c[i].id = id;
            c[i].buff = NULL;
            char    output[1024];
            sprintf(output, "server: client %d just arrived\n", id);
            send_to_c(output, c[i].fd, c);
            FD_SET(fd, &serv.master);
            id++;
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
                        if (c[j].buff)
                        {
                            char    *last_msg = NULL;
                            while (extract_message(&c[j].buff, &last_msg))
                            {
                                sprintf(send_buff, "client %d: %s", c[j].id, last_msg);
                                send_to_c(send_buff, c[j].fd, c);
                                free(last_msg);
                                last_msg = NULL;
                            }
                        }
                        free(c[j].buff);
                        c[j].buff = NULL;

                        char    output[1024];
                        sprintf(output, "server: client %d just left\n", c[j].id);
                        send_to_c(output, c[j].fd, c);
                        FD_CLR(c[j].fd, &serv.master);
                        close(c[j].fd);
                        c[j].fd = 0;
                    }
                    else
                    {
                        msg[r] = '\0';
                        char    *toto = NULL;
                        c[j].buff = str_join(c[j].buff, msg);
                        while (extract_message(&c[j].buff, &toto))
                        {
                            sprintf(send_buff, "client %d: %s", c[j].id, toto);
                            send_to_c(send_buff, c[j].fd, c);
                            free(toto);
                            toto = NULL;
                        }
                    }
                }
                j++;
            }
        }
    }
}