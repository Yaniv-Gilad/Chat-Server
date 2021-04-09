#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for read/write/close
#include <sys/types.h>  // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket interface functions
#include <netdb.h>      // host to IP resolution
#include <sys/select.h>
#include "lib.h"

// ***** list implement ***** //
clients_list *create_list()
{
    clients_list *list = (clients_list *)malloc(sizeof(clients_list));
    if (list == NULL)
    {
        perror("malloc\n");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;

    return list;
}

void add_to_list(clients_list *list, int client_number)
{
    struct client *temp = (struct client *)malloc(sizeof(struct client));
    if (temp == NULL)
    {
        perror("malloc\n");
        exit(EXIT_FAILURE);
    }
    temp->client_number = client_number;
    temp->next = NULL;

    if (list->head == NULL)
    {
        list->head = temp;
        return;
    }

    struct client *c = list->head;
    while (c->next != NULL)
        c = c->next;

    c->next = temp;
}

void remove_from_list(clients_list *list, int client_number)
{
    struct client *c = list->head;
    struct client *prev = NULL;

    // remove the head
    if (c->client_number == client_number)
    {
        list->head = list->head->next;
        close(client_number);
        free(c);
        return;
    }

    while (c != NULL && c->client_number != client_number)
    {
        prev = c;
        c = c->next;
    }

    if (c != NULL)
    {
        prev->next = c->next;
        close(client_number);
        free(c);
    }
}

void add_message(queue *q, clients_list *list, char *to_insert, int from_who)
{
    // biuld the message
    char mess[LINE_SIZE + 20];
    char client_number[10];
    mess[0] = '\0';
    client_number[0] = '\0';
    sprintf(client_number, "%d", from_who);
    strcat(mess, "guest");
    strcat(mess, client_number);
    strcat(mess, ": ");
    strcat(mess, to_insert);

    // insert message to the queue
    struct client *c = list->head;
    while (c != NULL)
    {
        if (c->client_number != from_who)
            insert_message(q, mess, c->client_number);
        c = c->next;
    }
}

void free_list(clients_list *list)
{
    struct client *c = list->head;
    struct client *next = list->head;

    while (c != NULL)
    {
        next = next->next;
        close(c->client_number);
        free(c);
        c = next;
    }
    free(list);
}

void print_list(clients_list *list)
{
    struct client *c = list->head;
    while (c != NULL)
    {
        printf("%d\n", c->client_number);
        c = c->next;
    }
}
// ********************* //

// ***** queue implement ***** //
queue *create_queue()
{
    queue *q = (queue *)malloc(sizeof(queue));
    if (q == NULL)
    {
        perror("malloc\n");
        exit(EXIT_FAILURE);
    }

    q->head = NULL;
    q->tail = NULL;

    return q;
}

void insert_message(queue *q, char *to_insert, int to_who)
{
    struct message_list *m = (struct message_list *)malloc(sizeof(struct message_list));
    if (m == NULL)
    {
        perror("malloc\n");
        free_queue(q);
        exit(EXIT_FAILURE);
    }

    m->message[0] = '\0';
    strcpy(m->message, to_insert);
    m->to_who = to_who;
    m->next = NULL;

    // if queue is empty
    if (q->head == NULL)
    {
        q->head = m;
        q->tail = m;
        return;
    }

    q->tail->next = m;
    q->tail = q->tail->next;
}

void read_messages(queue *q, clients_list *list, fd_set *rfd, fd_set *c_rfd)
{
    int rc = 0;
    char buf[LINE_SIZE + 1];
    buf[0] = '\0';
    struct client *cur = list->head;
    struct client *next = NULL;

    while (cur != NULL)
    {
        next = cur->next;
        rc = 0;
        if (FD_ISSET(cur->client_number, c_rfd))
        {
            printf("server is ready to read from socket %d\n", cur->client_number);
            rc = read(cur->client_number, buf, LINE_SIZE);
            if (rc < 0) // read failed
            {
                perror("read\n");
                exit(EXIT_FAILURE);
            }
            if (rc == 0) // client left
            {
                FD_CLR(cur->client_number, rfd);
                remove_from_list(list, cur->client_number);
            }
            if (rc > 0) // reading message from client
            {
                buf[rc] = '\0';
                add_message(q, list, buf, cur->client_number);
                buf[0] = '\0';
            }
        }
        cur = next;
    }
}

void send_messages(queue *q, fd_set *write_fds)
{
    int curr_fd;
    struct message_list *curr = q->head;
    struct message_list *prev = NULL;

    while (curr != NULL)
    {
        curr_fd = curr->to_who;

        // if need to write
        if (FD_ISSET(curr_fd, write_fds))
        {
            printf("Server is ready to write to socket %d\n", curr_fd);
            // if its the head
            if (prev == NULL)
            {
                write(curr_fd, curr->message, strlen(curr->message) + 1);
                q->head = curr->next;

                if (q->tail == curr)
                    q->tail = q->head;

                free(curr);
                curr = q->head;
            }

            else
            {
                write(curr_fd, curr->message, strlen(curr->message) + 1);
                prev->next = curr->next;
                if (q->tail == curr)
                    q->tail = prev;
                free(curr);
                curr = prev->next;
            }

            continue;
        }

        prev = curr;
        curr = curr->next;
    }
}

void free_queue(queue *q)
{
    struct message_list *curr = q->head;
    struct message_list *temp = q->head;

    while (curr != NULL)
    {
        temp = curr;
        curr = curr->next;
        free(temp);
    }

    free(q);
}

void print_queue(queue *q)
{
    struct message_list *cur = q->head;
    while (cur != NULL)
    {
        printf("%s , to: %d\n", cur->message, cur->to_who);
        cur = cur->next;
    }
}
// ********************* //
