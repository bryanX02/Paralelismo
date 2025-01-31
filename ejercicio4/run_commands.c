#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include<sys/types.h>
#include<fcntl.h>

pid_t launch_command(char** argv){
    pid_t pid = fork();

    if (pid == 0){
        execvp(argv[0], argv);
        exit(1);
    }
    else{
        return pid;
    }
}



char **parse_command(const char *cmd, int* argc) {
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));

    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc)=arg_count; // Return argc

    return argv;
}


int main(int argc, char *argv[]) {
    char **cmd_argv;
    int cmd_argc;
    int i;
    
    /*
    if (argc != 3) {
        fprintf(stderr, "Usage: %s \"command\"\n", argv[0]);
        return EXIT_FAILURE;
    }
    */

	int  opt;
    int status;
    while((opt = getopt(argc, argv, "x:s:")) != -1) {
		switch(opt) {
		case 'x':
			cmd_argv=parse_command(optarg,&cmd_argc);
            
            pid_t pid_m = launch_command(cmd_argv);
            
            //printf("pid -- %d\n", pid_m);

            if (pid_m>0){
                wait(&status);
            }

            for (i = 0; cmd_argv[i] != NULL; i++) {
                free(cmd_argv[i]);  // Free individual argument
            }
            free(cmd_argv);  // Free the cmd_argv array
			break;
		case 's':
	        FILE *stream = fopen(optarg,"r");
            if (stream< 0){
		        perror("open"); exit(1);
 	        }
            char line[1024];
            int j = 1;
            while(fgets(line, sizeof(line), stream) > 0){
               cmd_argv=parse_command(line, &cmd_argc);

                printf("@@ Running command #%d: %s", j, line);
                pid_t pid_m = launch_command(cmd_argv);
                
                //printf("pid -- %d\n", pid_m);

                if (pid_m>0){
                    wait(&status);
                }
                printf("@@ Command #%d terminated (pid: %d, status: %d)\n\n", j, pid_m, status);
                for (i = 0; cmd_argv[i] != NULL; i++) {
                    free(cmd_argv[i]);  // Free individual argument
                }
                free(cmd_argv);  // Free the cmd_argv array
                
                j++;
            }

			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

}