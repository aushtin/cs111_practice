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

/*
Thoughts on how to implement time travel:
 
Find files that are dependent
    - look for input/output operators

 Make dependency graph
    - which commands are dependent on a previous command?
 
Pseudocode:

 Traverse each tree in order to find commands with inputs and outputs
    Store these inputs/outputs in a data structure specific to each tree
 Traverse rest of the trees, and do the same
 Compare the input/output data structures of the trees
    If we find two inputs/outputs are same in two commands, we have dependence
        Increment number of dependencies for that specific input/output
 Run non-dependent in parallel
 execute dependent commands sequentially

 
*/

int
command_status (command_t c)
{
    return c->status;
}

///////////////////////////////////////////////////////////////
//////////////////   WRITE NODE CODE    ///////////////////////
///////////////////////////////////////////////////////////////

struct wnode {
    char* file_name;
    wnode_t next, prev;
};


struct write_list {
    wnode_t head, tail;
    wnode_t current;
};

write_list_t init_write_list(){
    write_list_t new_write_list = (write_list_t) checked_malloc(sizeof(write_list_t));
    new_write_list->head = NULL;
    new_write_list->tail = NULL;
    new_write_list->current = NULL;
    return new_write_list;
}

wnode_t create_wnode(char *file_name){
    wnode_t x = (wnode_t) checked_malloc(sizeof(*x));
    x->file_name = file_name;
    x->prev = NULL;
    x->next = NULL;
    return x;
}

void add_wnode_to_list(wnode_t wnode, write_list_t write_list) {
    
    if (write_list->head == NULL) {
        write_list->head = wnode;
        write_list->tail = wnode;
    }
    
    else {
        write_list->tail->next = wnode;
        wnode->next=NULL;
        wnode->prev=write_list->tail;
        write_list->tail = wnode;
    }
}


write_list_t make_write_list(write_list_t w_list, command_t c){
    if (!c){
        return NULL;
    }
    
    //if c->output is not NULL, there is a write, add it
    if (c->output){
        wnode_t new_write = create_wnode(c->output);
        add_wnode_to_list(new_write, w_list);
    }
    
    switch (c->type) {
        case AND_COMMAND:
        case SEQUENCE_COMMAND:
        case OR_COMMAND:
        case PIPE_COMMAND: {
            make_write_list(w_list, c->u.command[0]);
            make_write_list(w_list, c->u.command[1]);
            break;
        }
        case SIMPLE_COMMAND:
            break;
        case SUBSHELL_COMMAND: {
            make_write_list(w_list, c->u.subshell_command);
            break;
        }
        default:
            break;
    }
    
    return w_list;
}

///////////////////////////////////////////////////////////////
///////////////////   READ NODE CODE    ///////////////////////
///////////////////////////////////////////////////////////////

struct rnode {
    char* file_name;
    rnode_t next, prev;
};


struct read_list {
    rnode_t head, tail;
    rnode_t current;
};

read_list_t init_read_list(){
    read_list_t new_read_list = (read_list_t) checked_malloc(sizeof(read_list_t));
    new_read_list->head = NULL;
    new_read_list->tail = NULL;
    new_read_list->current = NULL;
    return new_read_list;
}

rnode_t create_rnode(char *file_name){
    rnode_t x = (rnode_t) checked_malloc(sizeof(*x));
    x->file_name = file_name;
    x->prev = NULL;
    x->next = NULL;
    return x;
}

void add_rnode_to_list(rnode_t rnode, read_list_t read_list) {
    
    if (read_list->head == NULL) {
        read_list->head = rnode;
        read_list->tail = rnode;
    }
    
    else {
        read_list->tail->next = rnode;
        rnode->next=NULL;
        rnode->prev=read_list->tail;
        read_list->tail = rnode;
    }
}

read_list_t make_read_list(read_list_t r_list, command_t c){
    if (!c){
        return NULL;
    }
    
    //if c->output is not NULL, there is a write, add it
    if (c->input){
        rnode_t new_read = create_rnode(c->output);
        add_rnode_to_list(new_read, r_list);
    }
    
    switch (c->type) {
        case AND_COMMAND:
        case SEQUENCE_COMMAND:
        case OR_COMMAND:
        case PIPE_COMMAND: {
            make_read_list(r_list, c->u.command[0]);
            make_read_list(r_list, c->u.command[1]);
            break;
        }
        case SIMPLE_COMMAND:
            break;
        case SUBSHELL_COMMAND: {
            make_read_list(r_list, c->u.subshell_command);
            break;
        }
        default:
            break;
    }
    
    return r_list;
}

//Compares read_lists. If we find matching reads, then we return 1.
//If there are no matching reads, return 0.
bool compare_read_list (read_list_t read_list1, read_list_t read_list2){
    if (read_list1 == NULL || read_list2 == NULL)
        return false;
    
    //We use these variables to traverse through each write_list
    rnode_t list1_curr_node = read_list1->head;
    rnode_t list2_curr_node = read_list2->head;
    
    while (list1_curr_node != NULL){
        while (list2_curr_node != NULL){
            if (list1_curr_node->file_name == list2_curr_node->file_name){
                return true;
            }
            
            list2_curr_node = list2_curr_node->next;
        }
        
        list1_curr_node = list1_curr_node->next;
    }
    
    return false;
}

/////////////////////////////////////////////////////////////
////////////////   DEPENDENCY TYPES    //////////////////////
/////////////////////////////////////////////////////////////

bool RAW_dependency(read_list_t tree2_read_list, write_list_t tree1_write_list){
    if (tree2_read_list == NULL || tree1_write_list == NULL)
        return false;
    
    //temp variables for iteration through lists
    rnode_t tree2_curr_node = tree2_read_list->head;
    wnode_t tree1_curr_node = tree1_write_list->head;
    
    while (tree2_curr_node != NULL){
        while (tree1_curr_node != NULL){
            if (tree2_curr_node->file_name == tree1_curr_node->file_name){
                return true;
            }
            
            tree1_curr_node = tree1_curr_node->next;
        }
        
        tree2_curr_node = tree2_curr_node->next;
    }
    
    return false;
}

bool WAR_dependency(write_list_t tree2_write_list, read_list_t tree1_read_list){
    if (tree2_write_list == NULL || tree1_read_list == NULL)
        return false;
    
    //temp variables for iteration through lists
    wnode_t tree2_curr_node = tree2_write_list->head;
    rnode_t tree1_curr_node = tree1_read_list->head;
    
    while (tree2_curr_node != NULL){
        while (tree1_curr_node != NULL){
            if (tree2_curr_node->file_name == tree1_curr_node->file_name){
                return true;
            }
            
            tree1_curr_node = tree1_curr_node->next;
        }
        
        tree2_curr_node = tree2_curr_node->next;
    }
    
    return false;
}

bool WAW_dependency(write_list_t tree2_write_list, write_list_t tree1_write_list){
    if (tree2_write_list == NULL || tree1_write_list == NULL)
        return false;
    
    //temp variables for iteration through lists
    wnode_t tree1_curr_node = tree2_write_list->head;
    wnode_t tree2_curr_node = tree1_write_list->head;
    
    while (tree1_curr_node != NULL){
        while (tree2_curr_node != NULL){
            if (tree1_curr_node->file_name == tree2_curr_node->file_name){
                return true;
            }
            
            tree2_curr_node = tree2_curr_node->next;
        }
        
        tree1_curr_node = tree1_curr_node->next;
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////
///////////////////   EXECUTING COMMAND CODE    ///////////////////////
///////////////////////////////////////////////////////////////////////

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

void
time_travel(command_stream_t cstream) {
    

    
}