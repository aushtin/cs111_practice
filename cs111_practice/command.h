// UCLA CS 111 Lab 1 command interface

typedef struct command *command_t;
typedef struct commandNode *commandNode_t;
typedef struct command_stream *command_stream_t;

typedef struct wnode *wnode_t;
typedef struct write_list *write_list_t;
typedef struct rnode *rnode_t;
typedef struct read_list *read_list_t;

typedef enum { false, true } bool;

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
 the command stream will invoke GETBYTE (ARG) to get the next byte.
 GETBYTE will return the next input byte, or a negative number
 (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
 an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
 nonzero.  */
void execute_command (command_t, int);

/* Return the exit status of a command, which must have previously been executed.
 Wait for the command, if it is not already finished.  */
int command_status (command_t);


write_list_t init_write_list();
wnode_t create_wnode(char *file_name);
void add_wnode_to_list(wnode_t wnode, write_list_t write_list);
write_list_t make_write_list(write_list_t w_list, command_t c);

read_list_t init_read_list();
rnode_t create_rnode(char *file_name);
void add_rnode_to_list(rnode_t wnode, read_list_t read_list);
read_list_t make_read_list(read_list_t r_list, command_t c);



void free_command(command_t);

struct commandNode{
    command_t cmd;
    commandNode_t next;
    commandNode_t prev;
    write_list_t write_list;
    read_list_t read_list;
    int tree_number;
};