#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>     // for read/write/close
#include <sys/types.h>  // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket interface functions
#include <netdb.h>      // host to IP resolution
#include <sys/select.h>
#include "lib.h"

#define LINE_SIZE 4096
int w_socket;
queue *q = NULL;
clients_list *clients = NULL;

int checkArg(int argc, char *argv[]);

void startSelect(int w_socket);

void ctrl_c_handler(int signum);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    signal(SIGINT, ctrl_c_handler);

    if (checkArg(argc, argv) == -1)
    {
        printf("Usage: server [port]\n");
        exit(0);
    }

    int port = -1;
    port = atoi(argv[1]);

    w_socket = 0;
    struct sockaddr_in serv_addr = {0};

    // initiate welcome socket
    w_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (w_socket < 0)
    {
        perror("ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // bind
    if (bind(w_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding\n");
        close(w_socket);
        exit(EXIT_FAILURE);
    }

    // listen
    if (listen(w_socket, 5) < 0)
    {
        perror("ERROR on binding\n");
        close(w_socket);
        exit(EXIT_FAILURE);
    }

    startSelect(w_socket);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int checkArg(int argc, char *argv[])
{
    if (argc != 2)
        return -1;

    int port = atoi(argv[1]);
    if (port <= 0)
        return -1;

    return 0;
}

void startSelect(int w_socket)
{
    q = create_queue();
    clients = create_list();

    int new_accept;
    fd_set rfd;
    fd_set c_rfd;
    fd_set wfd;

    FD_ZERO(&rfd);
    FD_ZERO(&c_rfd);
    FD_ZERO(&wfd);
    FD_SET(w_socket, &rfd);

    while (1)
    {
        c_rfd = rfd;
        if (q->head == NULL) // if queue is empty
            FD_ZERO(&wfd);
        else
            wfd = rfd;

        // if select failed
        if (select(getdtablesize(), &c_rfd, &wfd, NULL, NULL) < 0)
        {
            if (q != NULL)
                free_queue(q);
            if (clients != NULL)
                free_list(clients);

            close(w_socket);
            perror("select\n");
            exit(EXIT_FAILURE);
        }

        //  new chat participent
        if (FD_ISSET(w_socket, &c_rfd))
        {
            new_accept = accept(w_socket, NULL, NULL);
            if (new_accept > 0)
            {
                add_to_list(clients, new_accept);
                FD_SET(new_accept, &rfd);
            }
            else
            {
                if (q != NULL)
                    free_queue(q);
                if (clients != NULL)
                    free_list(clients);

                close(w_socket);
                perror("accept\n");
                exit(EXIT_FAILURE);
            }
        }

        // check for read
        read_messages(q, clients, &rfd, &c_rfd);

        // check for write
        send_messages(q, &wfd);
    }
}

void ctrl_c_handler(int signum)
{
    if (q != NULL)
        free_queue(q);
    if (clients != NULL)
        free_list(clients);
    close(w_socket);

    exit(EXIT_SUCCESS);
}