#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int spawn(const char * program, char ** arg_list) 
{
    pid_t child_pid = fork();
    if (child_pid != 0) return child_pid;
    else 
    {
        execvp (program, arg_list);
        perror("exec failed");
        return 1;
    }
}

int main() 
{
    //executing command_console and inspection_console processes from installation path using env variable ARPPATH
    char* my_env_var = getenv("ARPPATH");   
    char buffer1[100],buffer2[100];
    sprintf(buffer1,"%s/inspection_console/inspection_console",my_env_var); 
    char * arg_list_1[] = { "/usr/bin/konsole",  "-e", buffer1, "10", (char*)NULL };
    sprintf(buffer2,"%s/command_console/command_console",my_env_var); 
    char * arg_list_2[] = { "/usr/bin/konsole",  "-e", buffer2, "5", (char*)NULL };
    int pid;
    pid = spawn("/usr/bin/konsole", arg_list_1);
    printf("display konsole pid = %d\n", pid);
    pid = spawn("/usr/bin/konsole", arg_list_2);
    printf("keypad konsole pid = %d\n", pid);
    printf ("Main program exiting...\n");
    return 0;
}

