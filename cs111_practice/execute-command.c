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

//check for inputs and outputs
//if they exist, deal with them somehow
void handle_IO(command_t c) {
    
    if (c->input != NULL) { //we have an input
        
        int input_fd;
        input_fd = open(c->input, O_RDONLY, 0666);
        if (input_fd < 0) {
            fprintf(stderr, "Error in opening output file!");
            exit(1);
        }
        
        int dup_result = dup2(input_fd, 0);
        if (dup_result < 0) {
            fprintf(stderr, "Error in dup2() for input!");
            exit(1);
        }
        
        close(input_fd);
    }
    
    if (c->output != NULL) { //we have an input
        
        int output_fd;
        output_fd = open(c->output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        
        if (output_fd < 0) {
            fprintf(stderr, "Error in opening output file!");
            exit(1);
        }
        
        int dup_result = dup2(output_fd, 1);
        if (dup_result < 0) {
            fprintf(stderr, "Error in dup2() for output!");
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
                
            }
            
            
            else {  //this is the parent
                int status;
                //wait for child to exit
                while (-1 == waitpid(pid, &status, 0)){
                    
                }
                // printf("Child has not exited yet! WIFEXITED returns %d\n", WIFEXITED(status));
                //printf("WIFEXITED returns %d\n", WIFEXITED(status));
                //if (WIFEXITED(status)) {
                // printf("first child exited with %u\n", WEXITSTATUS(status));
                if (WIFEXITED(status))
                    c->status = WEXITSTATUS(status);
                
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
                //error(1, 0, "Cannot create pipe.");
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
                    //error(1, 0, "Cannot write to pipe.");
                }
                
                execute_command(c->u.command[0], time_travel);
                c->status = c->u.command[0]->status;
                
                close(fildes[1]);
            } else if (pid > 0) { //parent
                
                int status;
                
                /*
                WARNING: Do we do the same thing as in SIMPLE COMMAND, or can we use
                waitpid like we did here?
                */
                
                //wait for child to ext
                waitpid(pid, &status, 0);
                
                
                //close the WRITE portion
                close(fildes[1]);
                
                //check to see if we can use fildes[0] as input
                if (dup2(fildes[0],0) == -1){
                    //error(1, 0, "dup2() for parent failed.");
                }
                
                execute_command(c->u.command[1], time_travel);
                c->status = c->u.command[1]->status;
                
                close(fildes[0]);
            } else {    //error
                //error(1, 0, "Couldn't create child process");
            }
            
            break;
            
        case SUBSHELL_COMMAND:
            
            execute_command(c->u.subshell_command, time_travel);
            
            break;
            
        default:
            fprintf(stderr, "command is somehow invalid");
            exit(1);
            break;
            
            
    }
    
    
    
}