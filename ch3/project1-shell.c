#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* 80 chars per line, per command */


int read_in(char *a) {
    char c;
    int num  = 0;
    
    while (((c = getchar()) != '\n') && (num < MAX_LINE)) {
        a[num++] = c;
    }
    
    if (num == MAX_LINE && c != '\n') {
        printf("the command line is limited to 80 Characters\n");
        num = -1;
        while (c != '\n') c = getchar();
    } else {
        a[num] = 0;
    }
    return num;
}


int compile(char *buffer, int length, char* args[]) {
    int args_num = 0, last = -1;
    args[0] = NULL;
    int i = 0;
    for (i = 0; i <= length; ++i) {
        /* The last one must be zero so use <=*/
        if (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i]) {
            continue;
        } else {
            if (last != i-1) {
                //store the compile result of [last+1, i-1]
                args[args_num] = (char*)malloc(i-last);
                if (args[args_num] == NULL) {
                    printf("Unable to allocate memory\n");
                    return 1;
                }
                //malloc size+1
                memcpy(args[args_num], &buffer[last+1], i-last-1);
                //
                args[args_num][i-last] = 0;
                //set the last char zero
                args[++args_num] = NULL;
            }
            last = i;
        }
    }
    return args_num;
}


void print_history(char history[][MAX_LINE+1], int history_count) {
    if (history_count == 0) {
        printf("No history found\n");
        return;
    }
    int i = 0;
    int ii = 0;
    int j = 0;
    if (history_count < 10) {
        for (i = history_count, j = 10; i > 0 && j > 0; i --, j--) {
            printf("%4d\t%s\n", i, history[i % 10]);
        }
    } else {
        for (ii = history_count, j = 10; ii > 0 && j > 0; ii --, j--) {
            printf("%4d\t%s\n", j, history[ii % 10]);
        }
    }
}

int main() {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    
    char history[10][MAX_LINE + 1];
    int history_count = 0;
    
    char buffer[MAX_LINE + 1];
    memset(buffer, 0, sizeof(buffer));
    int length, args_num;
    
    while (should_run) {
        printf("osh>");
        fflush(stdout);
        
        length = read_in(buffer);
        if (length == -1) continue;
        
        /* executing the last command if the input is "!!" */
        if (strcmp(buffer, "!!") == 0) {
            if (history_count > 0) {
                memcpy(buffer, history[history_count % 10], MAX_LINE + 1);
                length = strlen(buffer);
            } else {
                printf("History doesn't have this command.\n");
                continue;
            }
        }
        
        args_num = compile(buffer, length, args);
        if (args_num == 0) continue;
        
        /*  executing the certain command if the input is ! N */
        if (strcmp(args[0], "!") == 0) {
            int temp = 0;
            int length = strlen(args[1]);
	    int i = 0;
            for (i = 0; i < length; i++) {
                if (args[1][i] > '9' || args[1][i] < '0') {
                    temp = -1;
                }
                temp = temp * 10 + (args[1][i] - '0');
            }
            
            if (temp <= 0 || temp < history_count - 9 || temp > history_count) {
                printf("History doesn't have this command.\n");
                continue;
            } else {
                memcpy(buffer, history[temp % 10], MAX_LINE + 1);
                length = strlen(buffer);
                args_num = compile(buffer, length, args);
            }
        }
        
        /* exiting the shell if the input is "exit" */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }
        
        /* list not more than 10 most recently entered commands */
        if (strcmp(args[0], "history") == 0) {
            print_history(history, history_count);
            continue;
        }
        
        history_count ++;
        memcpy(history[history_count% 10], buffer, MAX_LINE + 1);
        
        /*execute commond in a new process*/
        int run_background = 0;
        if (strcmp(args[args_num-1], "&") == 0) {
            run_background = 1;
            args[--args_num] = NULL;
        }
        
        pid_t pid = fork();
        int status;
        if (pid < 0) {
            printf("Error in Fork Process\n");
            return 1;
        } else if (pid == 0) {
            status = execvp(args[0], args);
            if (status == -1) {
                printf("Command faild\n");
            }
            return 0;
        } else {
            if (run_background) {
                printf("pid #%d running in background %s\n", pid, buffer);
            } else {
                wait(&status);
            }
        }
    }
    return 0;
}
