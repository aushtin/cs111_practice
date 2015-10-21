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