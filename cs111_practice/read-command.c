// UCLA CS 111 Lab 1 command reading

#include "command-internals.h"
#include "command.h"
#include "alloc.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
/* FIXME: You may need to add #include directives, macro definitions,
 static function definitions, etc.  */

//boolean
typedef enum { false, true } bool;

bool isValidChar(char character){
    if (isalpha(character) || isdigit(character) || character == '!' ||
        character == '%' || character == '+' || character == ',' ||
        character == '-' || character == '.' || character == '/' ||
        character == ':' || character == '@' || character == '^' ||
        character == '_' ) {
        return true;
    } else {
        return false;
    }
}

enum char_type
{
    REGULAR_CHAR,           //0
    TOKEN_CHAR,             //1
    NEWLINE_CHAR,           //2
    HASHTAG_CHAR,           //3
    WHITESPACE_CHAR,        //4
    INVALID_CHAR,           //5
};

enum char_type identify_char_type(char character) {
    
    //REGULAR CHARACTER
    if (isalpha(character) || isdigit(character) || character == '!' ||
        character == '%' || character == '+' || character == ',' ||
        character == '-' || character == '.' || character == '/' ||
        character == ':' || character == '@' || character == '^' ||
        character == '_' ) {
        return REGULAR_CHAR;
    }
    //TOKEN CHARACTER
    else if (character == '|' || character == '&' ||
             character == ';' || character == '(' || character == ')' ||
             character == '<' || character == '>'  )
        return TOKEN_CHAR;
    
    //NEWLINE CHARACTER
    else if (character == '\n')
        return NEWLINE_CHAR;
    
    //HASHTAG CHAR
    else if (character=='#')
        return HASHTAG_CHAR;
    
    //WHITESPACE
    else if (character==' ')
        return WHITESPACE_CHAR;
    
    //if none of the above, it is invalid
    else return INVALID_CHAR;
    
}


//make word**
char** make_word(char *simple_command, int num_words){
    
    char **words = (char**)(checked_malloc((num_words+1) * sizeof(char*)));
    memset (words, '\0', (num_words+1) * sizeof(char*));
    
    //counters
    int pointer_array_pos = 0;
    int word_array_pos = 0;
    int word_character_count = 0;
    int i = 0;
    
    //traversing through simple_command
    while (simple_command[i] != '\0' && simple_command[i] != '<' && simple_command[i] != '>'){
        if (simple_command[i] == ' '){
            
            //ake a new string in memory
            char *new_word = (char*) checked_malloc((i+1)*sizeof(char));
            memset(new_word, '\0', (i+1)*sizeof(char));
            
            //add word to second array
            int j;
            for (j=0; j<word_character_count;j++){
                new_word[j] =simple_command[word_array_pos];
                word_array_pos++;
            }
            
            //make our wordlist point to a new string
            words[pointer_array_pos] = new_word;
            
            //increment counters
            word_array_pos++;
            pointer_array_pos++;
            
            word_character_count=0;
        }
        //continute looking for words
        if (simple_command[i] != ' '){
            word_character_count++;
        }
        
        i++;
    }//end traversal through simple_command
    
    //make a new string in memory
    char *new_word = checked_malloc((i+1)*sizeof(char));
    memset(new_word, '\0', (i+1)*sizeof(char));
    
    //add word to second array
    int j;
    for (j=0; j<word_character_count;j++){
        new_word[j] =simple_command[word_array_pos];
        word_array_pos++;
    }
    
    //make our wordlist point to a new string
    words[pointer_array_pos] = new_word;
    
    return words;
}

command_t createCommand(enum command_type new_cmd, char *command_string) {
    
    
    command_t x = (command_t) checked_malloc(sizeof(*x));
    x->type = new_cmd;
    x->status = -1;
    x->input = 0;
    x->output = 0;
    
    
    switch (new_cmd) {
        case SIMPLE_COMMAND:
        {
            int i=0;
            int white_space_counter = 0;
            while (command_string[i] != '\0' && command_string[i] != '<' && command_string[i] != '>'){
                if (command_string[i] == ' '){
                    white_space_counter++;
                }
                i++;
            }
            
            int j=0;
            /*
            //search for redirects
            while (command_string[j] != '\0') {
                if (command_string[j] == '<') {
                    //get the filename
                    int k = j+1;
                    int filename_startpos = k;
                    while (command_string[k] != '\0' && command_string[k] != '>') {
                        k++;
                    }
                    
                    int input_filename_size = k-filename_startpos;
                    char* input_filename = (char*) checked_malloc((input_filename_size+1) * sizeof(char));
                    memset(input_filename, '\0', (input_filename_size+1) * sizeof(char));
                    int filename_pos =0;
                    
                    for (int l = filename_startpos; l<k; l++) {
                        input_filename[filename_pos] = command_string[l];
                        filename_pos++;
                    }
                    
                    //if we're here we've read in the filename
                    x->input = input_filename;
                }
                
                if (command_string[j] == '>') {
                    
                    int k = j+1;
                    int filename_startpos = k;
                    while (command_string[k] != '\0') {
                        k++;
                    }
                    
                    int input_filename_size = k-filename_startpos;
                    char* output_filename = (char*) checked_malloc((input_filename_size+1) * sizeof(char));
                    memset(output_filename, '\0', (input_filename_size+1) * sizeof(char));
                    int filename_pos =0;
                    
                    for (int l = filename_startpos; l<k; l++) {
                        output_filename[filename_pos] = command_string[l];
                        filename_pos++;
                    }
                    
                    //if we're here, we've read in the full filename.
                    x->output = output_filename;
                    
                }
                
                j++;
                
            }*/
            
            //number of words in simple command = white space + 1
            //malloc appropriate memory
            x->u.word = checked_malloc((white_space_counter+1) * sizeof(char*));
            memset(x->u.word, '\0', (white_space_counter+1)*sizeof(char*));
            
            //WARNING: How do you make a union return a certain item?
            x->u.word = make_word(command_string, white_space_counter+1);
            break;
        }
            
        case AND_COMMAND:
            //set both pointers to null to initialize
            x->u.command[0] = '\0';
            x->u.command[1] = '\0';
            break;
        case SEQUENCE_COMMAND:
            x->u.command[0] = '\0';
            x->u.command[1] = '\0';
            break;
        case OR_COMMAND:
            x->u.command[0] = '\0';
            x->u.command[1] = '\0';
            break;
        case PIPE_COMMAND:
            x->u.command[0] = '\0';
            x->u.command[1] = '\0';
            break;
        case SUBSHELL_COMMAND:
        {
            x->u.subshell_command = NULL;
            break;
        }
        default:
            break;
    }
    
    return x;
}


///////////////////////COMMAND NODE///////////////////////////////
//  command node is used in both the stack and the linked list. //

typedef struct commandNode *commandNode_t;

struct commandNode{
    command_t cmd;
    commandNode_t next;
    commandNode_t prev;
};

commandNode_t createNode(enum command_type new_cmd){
    commandNode_t x = (commandNode_t) checked_malloc(sizeof(*x));
    //x->cmd = createCommand(new_cmd);
    x->next = NULL;
    x->prev = NULL;
    return x;
}

commandNode_t createNodeFromCommand(command_t new_command){
    commandNode_t x = (commandNode_t) checked_malloc(sizeof(*x));
    x->cmd = new_command;
    x->next = NULL;
    x->prev = NULL;
    return x;
}


enum command_type getNodeType(commandNode_t node)
{
    return node->cmd->type;
}


/////////////////////////COMMAND STACK///////////////////////
//  implemented using a linked list of commandNodes        //


typedef struct commandStack *commandStack_t;

struct commandStack{
    commandNode_t bottom;
    commandNode_t top;
};


commandStack_t createStack(){
    commandStack_t x = (commandStack_t) checked_malloc(sizeof(*x));
    x->bottom = NULL;
    x->top = NULL;
    return x;
}



void stackPush(commandStack_t stack, commandNode_t newNode){
    if (stack->bottom == NULL){
        stack->bottom = newNode;
        stack->top = newNode;
        stack->bottom->prev = NULL;
        stack->top->prev = NULL;
    } else {
        newNode->prev = stack->top;
        stack->top->next = newNode;
        stack->top = newNode;
    }
}

commandNode_t stackPop(commandStack_t stack){
    if (stack->bottom == NULL){
        return NULL;
    }
    
    commandNode_t poppedNode = stack->top;
    
    if (stack->top->prev != NULL) {
        stack->top = stack->top->prev;
        stack->top->next = NULL;
    }
    else {
        stack->top = NULL;
        stack->bottom = NULL;
    }
    
    poppedNode->next = NULL;
    poppedNode->prev = NULL;
    return poppedNode;
}

commandNode_t getTop(commandStack_t stack){
    return stack->top;
}

//checks if given string is a valid word
int isWord(char *word){
    int counter = 0;
    while (word[counter] != '\0')
    {
        if (isalpha(word[counter]) || isdigit(word[counter]) || word[counter] == '!' ||
            word[counter] == '%' || word[counter] == '+' || word[counter] == ',' ||
            word[counter] == '-' || word[counter] == '.' || word[counter] == '/' ||
            word[counter] == ':' || word[counter] == '@' || word[counter] == '^' ||
            word[counter] == '_')
        {
            counter++;
        } else {
            return 0;
        }
    }
    return 1;
}


bool isOperator(char character) {
    
    if (character == '|' || character == '&' || character == ';') {
        return true;
    }
    else return false;
}

int getPrecedence(commandNode_t operator) {
    
    if (operator->cmd->type == SEQUENCE_COMMAND) {
        return 1;
    }
    else if (operator->cmd->type == OR_COMMAND || operator->cmd->type == AND_COMMAND) {
        return 2;
    }
    else if (operator->cmd->type == PIPE_COMMAND) {
        return 3;
    }
    
    return -1;
    
}

commandNode_t combine_commands(commandNode_t operator, commandNode_t top_operand, commandNode_t bot_operand) {
    
    command_t c1 = bot_operand->cmd;
    command_t c2 = top_operand->cmd;
    
    operator->cmd->u.command[0] = c1;
    operator->cmd->u.command[1] = c2;
    
    //assume that this doesnt kill the commands
    free(top_operand);
    free(bot_operand);
    
    return operator;
    
}

//////////////////////COMMAND STREAM/////////////////////
// command_stream is a linked list of commandNodes     //

typedef struct command_stream *command_stream_t;

//plant a tree. soon it will become part of a forest
command_t make_command_tree(char *complete_command){
    commandStack_t command_stack = createStack();
    commandStack_t operator_stack = createStack();
    
    int buff_pos = 0;
    char buff_char = '\0';
    
    //char array to store simple commands as they come
    
    int simple_command_pos = 0;
    
    /*
     REGULAR_CHAR,           //0
     TOKEN_CHAR,             //1
     WHITESPACE_CHAR,        //4
     */
    while ((buff_char = complete_command[buff_pos]) != '\0'){
        
        //If a simple command, push it onto a command stack
        if (identify_char_type(buff_char) == REGULAR_CHAR){
            
            char *simple_command = checked_malloc(1024 * sizeof(char));
            memset(simple_command, '\0', 1024 * sizeof(char));
            
            //add first valid character, and then add rest of the characters and whitespace in the simple command
            while (identify_char_type(buff_char) == REGULAR_CHAR || identify_char_type(buff_char) == WHITESPACE_CHAR /*||buff_char == '<' || buff_char == '>'*/){
                
                simple_command[simple_command_pos] = buff_char;
                simple_command_pos++;
                
                buff_pos++;
                buff_char = complete_command[buff_pos];
                
            }
            
            command_t new_cmd = createCommand(SIMPLE_COMMAND, simple_command);
            commandNode_t new_node = createNodeFromCommand(new_cmd);
            stackPush(command_stack, new_node);
            
            simple_command_pos =0;
            
            continue;
        }
        
        if (buff_char == '(') {
            command_t new_cmd = createCommand(SUBSHELL_OPEN, NULL);
            commandNode_t new_node = createNodeFromCommand(new_cmd);
            stackPush(operator_stack, new_node);
            buff_pos++;
            continue;
        }
        
        if (buff_char == '<') {
            
            //get the filename
            int k = buff_pos+1;
            int filename_startpos = k;
            while (complete_command[k] != '\0' && complete_command[k] != '>' && isOperator(complete_command[k]) != true && complete_command[k]!= ')') {
                k++;
            }
            
            int input_filename_size = k-filename_startpos;
            char* input_filename = (char*) checked_malloc((input_filename_size+1) * sizeof(char));
            memset(input_filename, '\0', (input_filename_size+1) * sizeof(char));
            int filename_pos =0;
            int l;
            for (l = filename_startpos; l<k; l++) {
                input_filename[filename_pos] = complete_command[l];
                filename_pos++;
            }
            
            //if we're here we've read in the filename
        
            getTop(command_stack)->cmd->input = input_filename;
            buff_pos=k;
            continue;
        }
        
        if (buff_char=='>') {
            
            int k = buff_pos+1;
            int filename_startpos = k;
            while (complete_command[k] != '\0' && isOperator(complete_command[k]) != true && complete_command[k]!= ')') {
                k++;
            }
            
            int input_filename_size = k-filename_startpos;
            char* output_filename = (char*) checked_malloc((input_filename_size+1) * sizeof(char));
            memset(output_filename, '\0', (input_filename_size+1) * sizeof(char));
            int filename_pos =0;
            int l;
            for (l = filename_startpos; l<k; l++) {
                output_filename[filename_pos] = complete_command[l];
                filename_pos++;
            }
            
            //if we're here, we've read in the full filename.
            getTop(command_stack)->cmd->output = output_filename;
            buff_pos=k;
            continue;
            
        }
        
        if (isOperator(buff_char) && operator_stack->top == NULL) {
            if (buff_char == '&') {
                command_t new_cmd = createCommand(AND_COMMAND, NULL);
                commandNode_t new_node = createNodeFromCommand(new_cmd);
                stackPush(operator_stack, new_node);
                buff_pos+=2;
                continue;
            }
            else if (buff_char == '|') {
                if (complete_command[buff_pos+1] == '|') {
                    command_t new_cmd = createCommand(OR_COMMAND, NULL);
                    commandNode_t new_node = createNodeFromCommand(new_cmd);
                    stackPush(operator_stack, new_node);
                    buff_pos+=2;
                    continue;
                }
                else {
                    command_t new_cmd = createCommand(PIPE_COMMAND, NULL);
                    commandNode_t new_node = createNodeFromCommand(new_cmd);
                    stackPush(operator_stack, new_node);
                    buff_pos++;
                    continue;
                }
            }
            else if (buff_char == ';') {
                command_t new_cmd = createCommand(SEQUENCE_COMMAND, NULL);
                commandNode_t new_node = createNodeFromCommand(new_cmd);
                stackPush(operator_stack, new_node);
                buff_pos++;
                continue;
            }
        }
        
        if (isOperator(buff_char) && operator_stack->top !=NULL) {
            
            command_t new_cmd;
            commandNode_t new_node;
            
            if (buff_char == '&') {
                new_cmd = createCommand(AND_COMMAND, NULL);
                new_node = createNodeFromCommand(new_cmd);
                buff_pos+=2;
            }
            else if (buff_char == '|') {
                if (complete_command[buff_pos+1] == '|') {
                    new_cmd = createCommand(OR_COMMAND, NULL);
                    new_node = createNodeFromCommand(new_cmd);
                    buff_pos+=2;
                }
                else {
                    new_cmd = createCommand(PIPE_COMMAND, NULL);
                    new_node = createNodeFromCommand(new_cmd);
                    buff_pos++;
                }
            }
            else { //(buff_char == ';') {
                new_cmd = createCommand(SEQUENCE_COMMAND, NULL);
                new_node = createNodeFromCommand(new_cmd);
                buff_pos++;
            }
            
            while ( getTop(operator_stack) != NULL && (getPrecedence(new_node) <= getPrecedence(getTop(operator_stack))) && getTop(operator_stack)->cmd->type != SUBSHELL_OPEN ) {
                commandNode_t popped = stackPop(operator_stack);
                commandNode_t operand1 = stackPop(command_stack);
                commandNode_t operand2 = stackPop(command_stack);
                
                //combine and then push onto stack
                commandNode_t combined_command = combine_commands(popped, operand1, operand2);
                stackPush(command_stack, combined_command);
                
            }
            
            stackPush(operator_stack, new_node);
            continue;
            
        }
        
        if (buff_char == ')') {
            
            commandNode_t poppedOperator;
            while ( ((poppedOperator = stackPop(operator_stack))->cmd->type) != SUBSHELL_OPEN  ) {
                
                commandNode_t operand1 = stackPop(command_stack);
                commandNode_t operand2 = stackPop(command_stack);
                
                //combine and then push onto stack
                commandNode_t combined_command = combine_commands(poppedOperator, operand1, operand2);
                stackPush(command_stack, combined_command);
                
            }
            
            //at this point the popped operator should be a SUBSHELL_OPEN; get rid of it
            free(poppedOperator);
            
            command_t new_subshell = createCommand(SUBSHELL_COMMAND, NULL);
            new_subshell->u.subshell_command = stackPop(command_stack)->cmd;
            commandNode_t new_subshell_node = createNodeFromCommand(new_subshell);
            stackPush(command_stack, new_subshell_node);
            
            buff_pos++;
            continue;
        }
        
        
        
        //buff_pos++;
    } //end of buff_char while loop
    
    while (operator_stack -> top != NULL) {
        commandNode_t popped = stackPop(operator_stack);
        commandNode_t operand1 = stackPop(command_stack);
        commandNode_t operand2 = stackPop(command_stack);
        
        //combine and then push onto stack
        commandNode_t combined_command = combine_commands(popped, operand1, operand2);
        stackPush(command_stack, combined_command);
        
    }
    
    return getTop(command_stack)->cmd;
}

struct command_stream {
    //head and tail pointers
    commandNode_t head, tail;
    
    //just in case we need to look in the middle of the list
    commandNode_t current;
};

command_stream_t initStream(){
    command_stream_t new_stream = (command_stream_t) checked_malloc(sizeof(command_stream_t));
    new_stream->head = NULL;
    new_stream->tail = NULL;
    new_stream->current = NULL;
    return new_stream;
}

void addNodeToStream(command_stream_t cs_stream, commandNode_t new_node) {
    
    if (cs_stream->head == NULL) {
        cs_stream->head = new_node;
        cs_stream->tail = new_node;
    }
    
    else {
        cs_stream->tail->next = new_node;
        new_node->next=NULL;
        new_node->prev=cs_stream->tail;
        
        cs_stream->tail = new_node;
        
        
    }
    
}

bool isTokenChar(char character) {
    
    if (character == '|' || character == '&' ||
        character == ';' || character == '(' || character == ')' ||
        character == '<' || character == '>'  )
        return true;
    else return false;
    
}


bool isValidToken(char first, char second) {
    
    
    switch (first) {
        case ';':
            if (!isTokenChar(second))
                return true;
            break;
        case '|':
            if (isTokenChar(second) && first == second)
                return true;
            break;
        case  '&':
            if (isTokenChar(second) && first == second)
                return true;
            break;
        case '(':
            if (!isTokenChar(second))
                return true;
            break;
    }
    return false;
}


void eatWhiteSpaces(char *buffer, int bufferSize, char *newArray){
    bool foundWord = false;
    int newArrayPos = 0;
    //memset(newArray, '\0', bufferSize * sizeof(char));
    int i;
    for (i = 0; i < bufferSize; i++){
        
        if (buffer[i] == ' '){
            //if we have a valid word before the space
            if (foundWord == false){
                continue;
            } else { //if we do have a word on the left of the white space
                
                //eat whitespaces, when we break out, we will have a character
                while (buffer[i] == ' '){
                    i++;
                }
                
                /*
                 REGULAR_CHAR,           //0
                 TOKEN_CHAR,             //1
                 NEWLINE_CHAR,           //2
                 HASHTAG_CHAR,           //3
                 WHITESPACE_CHAR,        //4
                 INVALID_CHAR,           //5
                 */
                if (identify_char_type(buffer[i])==REGULAR_CHAR ||
                    identify_char_type(buffer[i])==INVALID_CHAR){
                    //add space, and then add character
                    newArray[newArrayPos] = ' ';
                    newArrayPos++;
                    newArray[newArrayPos] = buffer[i];
                    newArrayPos++;
                } else if (identify_char_type(buffer[i])==TOKEN_CHAR){
                    newArray[newArrayPos] = buffer[i];
                    newArrayPos++;
                    foundWord=false;
                } else if (identify_char_type(buffer[i])==NEWLINE_CHAR){
                    
                } else if (identify_char_type(buffer[i])==HASHTAG_CHAR){
                    i++;
                }
            }
            
        } else if(identify_char_type(buffer[i])==REGULAR_CHAR ||
                  identify_char_type(buffer[i])==INVALID_CHAR){
            foundWord = true;
            newArray[newArrayPos] = buffer[i];
            newArrayPos++;
        } else if(identify_char_type(buffer[i])==TOKEN_CHAR){
            foundWord = false;
            newArray[newArrayPos] = buffer[i];
            newArrayPos++;
        } else if (identify_char_type(buffer[i])==HASHTAG_CHAR){
            i++;
            foundWord = false;
        } else if (identify_char_type(buffer[i])==NEWLINE_CHAR){
            foundWord = false;
        }
    }
    return;
}

//make sure the '&' character comes in pairs
void checkAndSyntax(char* complete_command) {
    
    int pos=0;
    while (complete_command[pos] != '\0') {
        
        //check for valid syntax
        //make sure & comes in pairs
        
        if (complete_command[pos] == '&') {
            pos++;
            if (complete_command[pos] != '&') {
                fprintf(stderr, "Invalid syntax: checkAndSyntax!");
                exit(1);
            }
        }
        pos++;
        
    }
    
}

//there can be no consecutive token characters
//the exceptions: a '&' character must be followed immediately by another '&'
//                a '|' character may be followed by a '|'
//                tokens may be followed by an open paren '('
void checkForConsecutiveTokens(char* complete_command) {
    
    int pos=0;
    while(complete_command[pos] != '\0') {
        
        //there can be no consecutive token characters
        if ( isTokenChar(complete_command[pos]) ) {
            switch (complete_command[pos]) {
                    
                case '<': case '>': case ';':
                    if ( complete_command[pos+1] == '<' || complete_command[pos+1] == '>' || complete_command[pos+1] == '|' || complete_command[pos+1] == '&' || complete_command[pos+1] == ';' || complete_command[pos+1] == ')' ) {
                        fprintf(stderr, "Invalid syntax : checkForConsecutiveTokens!");
                        exit(1);
                    }
                    break;
                case '&':
                    pos++;
                    if ( complete_command[pos+1] == '<' || complete_command[pos+1] == '>' || complete_command[pos+1] == '|' || complete_command[pos+1] == '&' || complete_command[pos+1] == ';' || complete_command[pos+1] == ')' ) {
                        fprintf(stderr, "Invalid syntax : checkForConsecutiveTokens!");
                        exit(1);
                    }
                    break;
                case '|':
                    if ( complete_command[pos+1] == '<' || complete_command[pos+1] == '>' || complete_command[pos+1] == '&' || complete_command[pos+1] == ';' || complete_command[pos+1] == ')' ) {
                        fprintf(stderr, "Invalid syntax : checkForConsecutiveTokens!");
                        exit(1);
                    }
                    
                    if ( complete_command[pos+1] == '|' ) {
                        pos++;
                        if ( complete_command[pos+1] == '<' || complete_command[pos+1] == '>' || complete_command[pos+1] == '|' || complete_command[pos+1] == '&' || complete_command[pos+1] == ';' || complete_command[pos+1] == ')' ) {
                            fprintf(stderr, "Invalid syntax : checkForConsecutiveTokens!");
                            exit(1);
                        }
                    }
                    break;
                case '(':
                    if ( complete_command[pos+1] == '<' || complete_command[pos+1] == '>' || complete_command[pos+1] == '|' || complete_command[pos+1] == '&' || complete_command[pos+1] == ';' ) {
                        fprintf(stderr, "Invalid syntax : checkForConsecutiveTokens!");
                        exit(1);
                    }
                    break;
                default: // ')'
                    break;
            }
        }
        pos++;
    }
}

//check that operators have operands (a valid word on the left and right side)
void checkIfOperatorsHaveOperands(char* complete_command) {
    
    int pos=0;
    
    char first_character = complete_command[0];
    if ( first_character == '<' || first_character == '>' || first_character == '|' || first_character == '&' || first_character == ';' || first_character == ')' ) {
        fprintf(stderr, "Invalid syntax : checkIfOperatorsHaveOperands!");
        exit(1);
    }
    
    while(complete_command[pos] != '\0') {
        pos++;
    }
    
    char last_character = complete_command[pos-1];
    if ( last_character == '<' || last_character == '>' || last_character == '|' || last_character == '&' || last_character == ';' || last_character == '(' ) {
        fprintf(stderr, "Invalid syntax : checkIfOperatorsHaveOperands!");
        exit(1);
    }
    
}


//check if parantheses are valid in a complete command
void validParentheses(char *complete_command){
    int command_counter = 0;
    int resolved_parantheses = 0;
    
    //can't start with closing parantheses
    if (complete_command[0] == ')'){
        fprintf(stderr, "Invalid syntax : Starting with closing parantheses!");
        exit(1);
    }
    
    //check that there are equal amounts of opening and closing parantheses
    while(complete_command[command_counter] != '\0'){
        if (complete_command[command_counter] == '('){
            resolved_parantheses++;
            
        } else if (complete_command[command_counter] == ')'){
            
            //checks if we have more closing braces proceeding opening braces
            resolved_parantheses--;
            if (resolved_parantheses<0){
                fprintf(stderr, "Invalid syntax : Too many closing parantheses!");
                exit(1);
            }
        }
        command_counter++;
    }
    
    if (resolved_parantheses != 0){
        fprintf(stderr, "Invalid syntax : Number of closing parantheses != number of opening parantheses");
        exit(1);
    }
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
                     void *get_next_byte_argument)
{
    
    //current character
    char curr;
    
    char prev_char_stored = '\0';
    int consecutive_newlines = 0;
    
    
    //buffer size = 1KB
    int BUFFER_SIZE = 1024;
    //buffer to read characters into; buffer can hold 1024 chars at once
    char *buffer = (char *) checked_malloc(BUFFER_SIZE * sizeof(char));
    
    //int to count how many chars are in buffer
    int numChars = 0;
    
    //int to count which line we are on
    //int syntax_line_counter = 0;
    int invalid_char_line_counter = 1;
    
    //if this is true, we are searching for the right operand of an operator
    bool found_AND_OR_PIPE_SEQUENCE = false; //looking for operand
    
    //declare command stream and allocate space
    //maybe dont need to allocate space because initStream() already does?
    command_stream_t theStream;    // = (command_stream_t) checked_malloc(sizeof(struct command_stream));
    
    //initialize command_stream
    theStream = initStream();
    
    //start reading the chars from the input
    while ((curr = get_next_byte(get_next_byte_argument)) != EOF ) {
        
        if (numChars > 0){
            //only stores previous meaningful character, i.e. not whitespace
            if (buffer[numChars-1] != ' ' && buffer[numChars-1] != '#'){
                prev_char_stored = buffer[numChars-1];
            }
        }
        
        if (prev_char_stored == '&' || prev_char_stored == '|' || prev_char_stored == ';'){
            found_AND_OR_PIPE_SEQUENCE = true;
        }
        
        
        
        ////////////////////////////////////////////////////////////
        //////////////////      NEWLINE CHAR    ////////////////////
        ////////////////////////////////////////////////////////////
        
        /*
         New variables introduced:
         char prev_char_stored;
         int consecutive_newlines;
         bool command_has_ended; //haven't needed to use yet
         
         */
        if (identify_char_type(curr) == NEWLINE_CHAR){
            
            //found a newline, increment counter
            consecutive_newlines++;
            
            //hit second (or more) newline
            if (consecutive_newlines > 1){
                //not looking for an operator
                if (!found_AND_OR_PIPE_SEQUENCE){
                    //add second newline to buffer
                    buffer[numChars] = '\n';
                    numChars++;
                    curr = get_next_byte(get_next_byte_argument);
                    
                    /*
                     check for newlines, whitespaces, and hashtags after second newline
                     Store newlines on the buffer, ignore white spaces, and for hashtags:
                     put hashtag and newline on the buffer
                     */
                    while (identify_char_type(curr) == NEWLINE_CHAR || identify_char_type(curr) == WHITESPACE_CHAR || identify_char_type(curr) == HASHTAG_CHAR){
                        if (curr == '\n'){
                            buffer[numChars] = '\n';
                            numChars++;
                        } else if (curr == '#'){
                            //add hashtag to buffer
                            buffer[numChars] = '#';
                            numChars++;
                            
                            //go to end of comment
                            while (identify_char_type(curr) != NEWLINE_CHAR){
                                if ((curr = get_next_byte(get_next_byte_argument)) == EOF) {
                                    break;
                                }
                            }
                            //broke out of loop, curr is now a newline char; add to buffer
                            buffer[numChars] = '\n';
                            numChars++;
                        }
                        
                        if ((curr = get_next_byte(get_next_byte_argument)) == EOF) {
                            break;
                        }
                    }
                    
                    consecutive_newlines = 0;
                    
                    //create temporary array with size of numChars;
                    char* buffer_no_whitespaces = checked_malloc(BUFFER_SIZE * (sizeof(char)));  //the correct syntaxed command
                    memset(buffer_no_whitespaces, '\0', BUFFER_SIZE * sizeof(char));
                    
                    //run validation, then parse if correct
                    
                    eatWhiteSpaces(buffer, numChars, buffer_no_whitespaces);
                    
                    
                    int pos = 0;
                    
                    while (buffer_no_whitespaces[pos] != '\0'){
                        if (identify_char_type(buffer_no_whitespaces[pos]) == NEWLINE_CHAR){
                            invalid_char_line_counter++;
                        }
                        if (identify_char_type(buffer_no_whitespaces[pos]) == INVALID_CHAR){
                            identify_char_type(buffer_no_whitespaces[pos]);
                            fprintf(stderr, "%d: Invalid character: %c <---", invalid_char_line_counter, buffer_no_whitespaces[pos]);
                            exit(1);
                        }
                        pos++;
                    }
                    
                    checkAndSyntax(buffer_no_whitespaces);
                    checkForConsecutiveTokens(buffer_no_whitespaces);
                    checkIfOperatorsHaveOperands(buffer_no_whitespaces);
                    validParentheses(buffer_no_whitespaces);
                    
                    /*
                    int i;
                     for (i= 0; i<numChars; i++){
                     printf("%c", buffer_no_whitespaces[i]);
                     }
                     
                     //this just separates commands
                     printf("\n");
                    */
                    
                    commandNode_t root = createNodeFromCommand(make_command_tree(buffer_no_whitespaces));
                    addNodeToStream(theStream, root);
                    
                    
                    //reset everything with memset
                    memset(buffer, '\0', BUFFER_SIZE * sizeof(char));
                    free(buffer_no_whitespaces);
                    numChars = 0;
                    consecutive_newlines = 0;
                    
                    
                    if (curr == EOF)
                        break;
                    /*
                     We are now at a new command.
                     Add first character to buffer, whether valid or invalid.
                     Will check syntax later.
                     */
                    if (identify_char_type(curr) != HASHTAG_CHAR){
                        buffer[numChars] = curr;
                        numChars++;
                    } else {
                        //add a hashtag and newline to the buffer
                        buffer[numChars] = '#';
                        numChars++;
                        
                        //get to the end of the line
                        while (identify_char_type(curr) != NEWLINE_CHAR){
                            if ((curr = get_next_byte(get_next_byte_argument)) == EOF) {
                                break;
                            }
                        }
                        
                        //now we have a newline; add newline to buffer
                        buffer[numChars]=curr;
                        numChars++;
                    }
                    continue;
                } else { //if we are looking for operand, don't end the commmand
                    
                    while (identify_char_type(curr) == NEWLINE_CHAR || identify_char_type(curr) == WHITESPACE_CHAR || identify_char_type(curr) == HASHTAG_CHAR){
                        if (curr == '\n'){
                            buffer[numChars] = '\n';
                            numChars++;
                        } else if (curr == '#'){
                            //add hashtag to buffer
                            buffer[numChars] = '#';
                            numChars++;
                            
                            while (identify_char_type(curr) != NEWLINE_CHAR){
                                if ((curr = get_next_byte(get_next_byte_argument)) == EOF) {
                                    break;
                                }
                            }
                            //broke out of loop, curr is now a newline char; add to buffer
                            buffer[numChars] = '\n';
                            numChars++;
                        }
                        
                        if ((curr = get_next_byte(get_next_byte_argument)) == EOF) {
                            break;
                        }
                    }
                    //broken out of while loop, we now have a regular character; add to buffer
                    
                    if (curr == EOF)
                        break;
                    
                    buffer[numChars] = curr;
                    numChars++;
                    found_AND_OR_PIPE_SEQUENCE = false;
                    consecutive_newlines = 0;
                    continue;
                }
            } else {
                //add newline to buffer; this is when number of newlines equals one
                buffer[numChars] = '\n';
                numChars++;
                continue;
            }
        } //end newline case
        else {
            
            
            if (!found_AND_OR_PIPE_SEQUENCE && consecutive_newlines == 1) {
                buffer[numChars] = ';';
                numChars++;
            }
            
            buffer[numChars] = curr;
            numChars++;
            consecutive_newlines = 0;
            
            //if we are here we no longer skip lines
            found_AND_OR_PIPE_SEQUENCE = false;
        }
    }
    
    char* buffer_no_whitespaces = checked_malloc(BUFFER_SIZE * (sizeof(char)));  //the correct syntaxed command
    memset(buffer_no_whitespaces, '\0', BUFFER_SIZE * sizeof(char));
    
    //run validation, then parse if correct
    //void eatWhiteSpaces(char *buffer, int bufferSize, char *newArray)
    eatWhiteSpaces(buffer, numChars, buffer_no_whitespaces);
    int pos = 0;
    
    while (buffer_no_whitespaces[pos] != '\0'){
        if (identify_char_type(buffer_no_whitespaces[pos]) == NEWLINE_CHAR){
            invalid_char_line_counter++;
        }
        if (identify_char_type(buffer_no_whitespaces[pos]) == INVALID_CHAR){
            identify_char_type(buffer_no_whitespaces[pos]);
            fprintf(stderr, "%d: Invalid character: %c <---", invalid_char_line_counter, buffer_no_whitespaces[pos]);
            exit(1);
        }
        pos++;
    }
    
    checkAndSyntax(buffer_no_whitespaces);
    checkForConsecutiveTokens(buffer_no_whitespaces);
    checkIfOperatorsHaveOperands(buffer_no_whitespaces);
    validParentheses(buffer_no_whitespaces);
    
    /*
     if (buffer_no_whitespaces[0] != '\0') {
         int i;
     for (i= 0; i<numChars; i++){
     printf("%c", buffer_no_whitespaces[i]);
     }
     printf("\n");
     }
    */
    
    //make sure buffer_no_whitespace is not empty
    if (buffer_no_whitespaces[0] != '\0') {
    commandNode_t root = createNodeFromCommand(make_command_tree(buffer_no_whitespaces));
    addNodeToStream(theStream, root);
    }
    
    free(buffer);
    free(buffer_no_whitespaces);
    
    return theStream;
}

void free_command(command_t to_be_freed) {
    
    enum command_type cmd_type = to_be_freed->type;
    
    free(to_be_freed->input);
    free(to_be_freed->output);
    
    if (cmd_type == AND_COMMAND || cmd_type == OR_COMMAND || cmd_type == PIPE_COMMAND || cmd_type == SEQUENCE_COMMAND) {
        
        free_command(to_be_freed->u.command[0]);
        free_command(to_be_freed->u.command[1]);
        
        //we're outta there, now free yourself
        free(to_be_freed->u.command[0]);
        free(to_be_freed->u.command[1]);
        
    }
    
    else if (cmd_type == SIMPLE_COMMAND) {
        
        char **w = to_be_freed->u.word;
        while ( *++w) {
            free(*w);
        }
        //free(w);
    }
    
    else //(cmd_type == SUBSHELL_COMMAND)
    {
        
        free_command(to_be_freed->u.subshell_command);
        free(to_be_freed->u.subshell_command);
        
    }
    
    
    
    
}

command_t
read_command_stream (command_stream_t s)
{
    if (s->head == NULL) {
        return NULL;
    }
    
    
    command_t grabbed_command = s->head->cmd;
    commandNode_t to_be_freed = s->head;
    
    s->head = s->head->next;
    
    if (s->head != NULL) {
        s->head->prev = NULL;
    }
    free(to_be_freed);
    return grabbed_command;
}


