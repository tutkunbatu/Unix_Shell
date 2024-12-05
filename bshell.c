#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int bsh_execute(char **args);
void bsh_loop();
char *bsh_read_line(void);
char **bsh_split_line(char *line);
int bsh_launch(char **args);
int bsh_cd(char **args);
int bsh_help(char **args);
int bsh_exit(char **args);
int lsh_num_builtins();

#define BSH_RL_BUFSIZE 1024
#define BSH_TOK_BUFSIZE 64
#define BSH_TOK_DELIM " \t\r\n\a"

// List of built-in commands
char *builtin_str[] = {
    "cd",
    "help", 
    "exit"
};

int(*builtin_func[]) (char **) = {
    &bsh_cd,
    &bsh_help,
    &bsh_exit
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

char *bsh_read_line(void){
    int bufsize = BSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer){
        fprintf(stderr, "bsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        // Reading the character
        c = getchar();
        // If it`s EOF, replace it with null
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            break;
        }
        else{
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate
        if(position >= bufsize){
            bufsize += BSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "bsh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return buffer;
}

char **bsh_split_line(char *line){
    int bufsize = BSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "bsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, BSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            bufsize += BSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, "bsh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, BSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int bsh_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0){
        // Child process
        if(execvp(args[0], args) == -1){
            perror("bsh");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0){
            // Error forking
            perror("bsh");
    } else{
        // Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

// Built-in command functions
int bsh_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, "bsh expected argument to \"cd\"\n");
    } else{
        if(chdir(args[1]) != 0){
            perror("bsh");
        }
    }
    return 1;
}

int bsh_help(char **args){
    printf("Batu Ada Tutkun's Shell\n");
    printf("Type command names and arguments and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < lsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("Use the man command for information on other programs.\n");
    return 1;
}

int bsh_exit(char **args){
    return 0;
}

int bsh_execute(char **args){
    if(args[0] == NULL){
        return 1;
    }

    for(int i = 0; i < lsh_num_builtins(); i++){
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return bsh_launch(args);
}

void bsh_loop(){
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = bsh_read_line();
        args = bsh_split_line(line);
        status = bsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv){
    // Run the command loop
    bsh_loop();
    
    return 0;
}