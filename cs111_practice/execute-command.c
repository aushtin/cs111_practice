#include "command-internals.h"
#include "command.h"
#include "alloc.h"
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int
command_status (command_t c)
{
    return c->status;
}

typedef struct wnode *wnode_t;

struct wnode {
    
    char* file_name;
    wnode_t next, prev;
    
    
};

wnode_t create_wnode(char* file_name) {
    
    wnode_t x = (wnode_t) checked_malloc(sizeof(*x));
    x->file_name = file_name;
    x->next = NULL;
    x->prev = NULL;
    return x;
    
}

typedef struct write_list *write_list_t;

struct write_list {
    //head and tail pointers
    wnode_t head, tail;
    
    //just in case we need to look in the middle of the list
    wnode_t current;
    
    int tree_number;
};

write_list_t init_write_list(int tree_number){
    write_list_t new_write_list = (write_list_t) checked_malloc(sizeof(write_list_t));
    new_write_list->head = NULL;
    new_write_list->tail = NULL;
    new_write_list->current = NULL;
    new_write_list->tree_number=tree_number;
    return new_write_list;
}

void add_wnode_to_list(write_list_t w_list, wnode_t new_node) {
    
    if (w_list->head == NULL) {
        w_list->head = new_node;
        w_list->tail = new_node;
    }
    
    else {
        w_list->tail->next = new_node;
        new_node->next=NULL;
        new_node->prev=w_list->tail;
        
        w_list->tail = new_node;
        
        
    }
    
}

write_list_t make_write_list(write_list_t w_list, command_t c, int tree_number) {

    
    if (c == NULL)
        return NULL;
    
    //if c->output is not NULL, there is a WRITE here
    if (c->output != NULL) {
        
        wnode_t new_write = create_wnode(c->output);
        add_wnode_to_list(w_list, new_write);
        
    }
    
    switch (c->type) {
            
        case AND_COMMAND:
        case OR_COMMAND:
        case PIPE_COMMAND:
        case SEQUENCE_COMMAND:
            make_write_list(w_list, c->u.command[0], tree_number);
            make_write_list(w_list, c->u.command[1], tree_number);
            break;
        case SIMPLE_COMMAND:
            break;
        case SUBSHELL_COMMAND:
            make_write_list(w_list, c->u.subshell_command, tree_number);
            break;
        default:
            break;
            
    }
    
    return w_list;
    
}

typedef struct rnode *rnode_t;

struct rnode {
    
    char* file_name;
    rnode_t next, prev;
    
    
};

rnode_t create_rnode(char* file_name) {
    
    rnode_t x = (rnode_t) checked_malloc(sizeof(*x));
    x->file_name = file_name;
    x->next = NULL;
    x->prev = NULL;
    return x;
    
}

typedef struct read_list *read_list_t;

struct read_list {
    //head and tail pointers
    rnode_t head, tail;
    
    //just in case we need to look in the middle of the list
    rnode_t current;
    
    int tree_number;
};

read_list_t init_read_list(int tree_number){
    read_list_t new_read_list = (read_list_t) checked_malloc(sizeof(read_list_t));
    new_read_list->head = NULL;
    new_read_list->tail = NULL;
    new_read_list->current = NULL;
    new_read_list->tree_number=tree_number;
    return new_read_list;
}

void add_rnode_to_list(read_list_t r_list, rnode_t new_node) {
    
    if (r_list->head == NULL) {
        r_list->head = new_node;
        r_list->tail = new_node;
    }
    
    else {
        r_list->tail->next = new_node;
        new_node->next=NULL;
        new_node->prev=r_list->tail;
        
        r_list->tail = new_node;
        
        
    }
    
}


//check for inputs and outputs
//if they exist, deal with them somehow
void handle_IO(command_t c) {
    
    if (c->input != NULL) { //we have an input
        
        int input_fd;
        input_fd = open(c->input, O_RDONLY, 0666);
        if (input_fd < 0) {
            fprintf(stderr, "%s: error opening input file\n", c->input);
            exit(1);
        }
        
        int dup_result = dup2(input_fd, 0);
        if (dup_result < 0) {
            fprintf(stderr, "Error in dup2() for input %s!\n", c->input);
            exit(1);
        }
        
        close(input_fd);
    }
    
    if (c->output != NULL) { //we have an input
        
        int output_fd;
        output_fd = open(c->output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        
        if (output_fd < 0) {
            fprintf(stderr, "%s: error opening output file", c->output);
            exit(1);
        }
        
        int dup_result = dup2(output_fd, 1);
        if (dup_result < 0) {
            fprintf(stderr, "Error in dup2() for output %s\n", c->output);
            exit(1);
        }
        
        close(output_fd);
    }
    
}




//what is time_travel?
void
execute_command (command_t c, int time_travel)
{
    
    
    pid_t pid;
    int fildes[2];    
    switch (c->type) {
            
        case SIMPLE_COMMAND:
            
            pid = fork();
            
            if (pid == -1) { //error in fork()
                fprintf(stderr, "Error in fork()!");
                exit(1);
            }
            
            else if (pid == 0) { //we are in the child process; execute simple command here
                
                
                handle_IO(c);
                
                execvp(c->u.word[0], c->u.word);
                
                //error in finding file
 
                fprintf(stderr, "%s: command not found\n", c->u.word[0]);
                exit(1);
                
            }
            
            
            else {  //this is the parent
                int status;
                //wait for child to exit
                
                while (-1 == waitpid(pid, &status, 0)){
                //    printf("Child has not exited yet! WIFEXITED returns %d\n", WIFEXITED(status));
                }
                /*
                printf("WIFEXITED returns %d\n", WIFEXITED(status));
                if (WIFEXITED(status)) {
                 printf("first child exited with %u\n", status);*/
                if (WIFEXITED(status)) {
                    c->status = WEXITSTATUS(status);
                    //printf("Exit status for %s command: %d\n", c->u.word[0], c->status);
                }
                
            }
            
            break;
        case AND_COMMAND:
            
            //execute first command in array
            execute_command(c->u.command[0], time_travel);
            c->status = c->u.command[0]->status;
            
            //execute second command in array if first one exits 0 (i.e. true)
            if (c->status == 0){
                execute_command(c->u.command[1], time_travel);
                c->status = c->u.command[1]->status;
                
            }
            break;
            
        case OR_COMMAND:
            //execute first command in array
            execute_command(c->u.command[0], time_travel);
            c->status = c->u.command[0]->status;
            
            //if first command isn't true, check second one
            if (c->status != 0){
                execute_command(c->u.command[1], time_travel);
                c->status = c->u.command[1]->status;
            }
            
            break;
        case SEQUENCE_COMMAND:
            //recursively call both commands
            execute_command(c->u.command[0], time_travel);
            //c->status = c->u.command[0]->status;
            
            execute_command(c->u.command[1], time_travel);
            c->status = c->u.command[1]->status;
            
            break;
        case PIPE_COMMAND:
            /*
             int pipe(int fildes[2]);
             
             The pipe() function shall create a pipe and place two file descriptors, one each into the arguments fildes[0] and fildes[1], that refer to the open file descriptions for the read and write ends of the pipe.
             
             Upon successful completion, 0 shall be returned; otherwise, -1 shall be returned and errno set to indicate the error.
             */
            
            //make a pipe, check for successful creation
            if (pipe(fildes) == -1){
                fprintf(stderr, "Cannot create pipe.");
                exit(1);
            }
            
            pid = fork();
            
            if (pid == -1) { //error in fork()
                fprintf(stderr, "Error in fork() for PIPE_COMMAND!");
                exit(1);
            } else if (pid == 0) { //child
                
                //close the READ portion (first element), then go on to check WRITE element
                close(fildes[0]);
                
                /*
                dup2 to check to see if we can write to pipe
                remember: file descriptors have the following integer values:
                    0: for standard input
                    1: for standard output
                    2: for standard error
                */
                
                if (dup2(fildes[1],1) == -1){

                    fprintf(stderr, "Cannot write to pipe");
                    exit(1);
                }
                
                execute_command(c->u.command[0], time_travel);
                c->status = c->u.command[0]->status;
                
                close(fildes[1]);
                exit(0);
                
            } else if (pid > 0) { //parent
                
                int status;
                
                //wait for child to ext
                while (-1 == waitpid(pid, &status, 0)){}
                
                //close the WRITE portion
                close(fildes[1]);
                
                //check to see if we can use fildes[0] as input
                if (dup2(fildes[0],0) == -1){
                    fprintf(stderr, "dup2() for parent failed");
                    exit(1);
                }
                
                execute_command(c->u.command[1], time_travel);
                c->status = c->u.command[1]->status;
                
                close(fildes[0]);
                
            } else {    //error
                fprintf(stderr, "Couldn't create child process (PIPE).");
                exit(1);
            }
            
            break;
            
        case SUBSHELL_COMMAND:
            
            c->u.subshell_command->input=c->input;
            c->u.subshell_command->output= c->output;
            execute_command(c->u.subshell_command, time_travel);
            
            break;
            
        default:
            fprintf(stderr, "command is somehow invalid");
            exit(1);
            break;
            
            
    }
}