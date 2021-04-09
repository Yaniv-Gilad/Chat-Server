#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for read/write/close

#define LINE_SIZE 4096

// ***** structs ***** //
struct client
{
    int client_number;
    struct client *next;
};

typedef struct clients_list
{
    struct client *head;
} clients_list;

struct message_list
{
    char message[LINE_SIZE + 20];
    int to_who;
    struct message_list *next;
};

typedef struct
{
    struct message_list *head;
    struct message_list *tail;
} queue;
// ******************* //

// ***** list declaration ***** //
clients_list *create_list();
void add_to_list(clients_list *list, int client_number);
void remove_from_list(clients_list *list, int client_number);
void add_message(queue *q, clients_list *list, char *to_insert, int from_who);
void print_list(clients_list *list);
void free_list(clients_list *list);
// ***************************** //

// ***** queue declaration ***** //
queue *create_queue();
void insert_message(queue *q, char *to_insert, int to_who);
void read_messages(queue *q, clients_list *list, fd_set *rfd, fd_set *c_rfd);
void send_messages(queue *q, fd_set *write_fds);
void free_queue(queue *q);
void print_queue(queue *q);
// ***************************** //
