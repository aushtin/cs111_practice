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
            
            else {   //this is the parent process
                
                //do nothing
                int status;
                waitpid(pid, &status, WUNTRACED);
                c->status = status;
                printf("the command %s exited with code %d", c->u.word[0], c->status);
            }
            
            
            break;
            
            
    }
    
    
    
}
