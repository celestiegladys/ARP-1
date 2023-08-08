#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>

// definig position limits for display
#define max_col 50
#define max_row 10

// defining global variables
pid_t child1_pid, child2_pid;

// pre-deifnition of singal handler function
void sig_handler (int signo);

int main() 
{
    child1_pid = fork();
    if (child1_pid == 0)                                       // first child (motor_x)
    {
        //executing motor_x process from installation path using env variable ARPPATH
        char* my_env_var = getenv("ARPPATH");   
        char buffer1[100];
        sprintf(buffer1,"%s/motor_x/motor_x",my_env_var); 
        char * arg_list1[] = { buffer1, NULL};
        execvp(buffer1, arg_list1);
        perror("exec failed");
        return 1;
    } 
    else                                                      // second child (motor_z)      
    {
        child2_pid = fork();
        if (child2_pid == 0)
        {
            //executing motor_z process from installation path using env variable ARPPATH
            char* my_env_var = getenv("ARPPATH");   
            char buffer2[100];
            sprintf(buffer2,"%s/motor_z/motor_z",my_env_var); 
            char * arg_list2[] = { buffer2, NULL};
            execvp(buffer2, arg_list2);
            perror("exec failed");
            return 1;
        } 
        else 
        {                                                     //parent process (inspection console)
            // defining private variables
            int n1, n2;
            int i, j;  
            char buffer1[80],buffer2[80]; 
            char format_string[80]="%d";

            // defining variables for named pipes  
            int fd1, fd2;
            char * myfifo3 = "/tmp/myfifo3"; 
            mkfifo(myfifo3, 0666); 
            char * myfifo4 = "/tmp/myfifo4"; 
            mkfifo(myfifo4, 0666); 

            // registering signal handler
            struct sigaction sig;
            memset(&sig, 0, sizeof(sig));
            sig.sa_handler = &sig_handler;
            sigaction(SIGTSTP, &sig, NULL);
            sigaction(SIGINT, &sig, NULL);
            sigaction(SIGUSR1, &sig, NULL);
            sigaction(SIGUSR2, &sig, NULL);      

            while (1) 
            { 
                // initialising IPC between motor_x and inspection console
                fd1 = open(myfifo3,O_RDONLY); 
                read(fd1, buffer1, 80);
                sscanf(buffer1, format_string, &n1);
                close(fd1);

                 // initialising IPC between motor_z and inspection console
                fd2 = open(myfifo4,O_RDONLY); 
                read(fd2, buffer2, 80);
                sscanf(buffer2, format_string, &n2);
                close(fd2);

                // displaying the initial message
                system("clear"); 
                printf("**************************************************\n" 
                "* Here you can monitor the poisiton of hoist!    *\n" 
                "*                                                *\n"
                "* Press Ctrl+C for emergency stop.               *\n"         
                "* Press Ctrl+Z for resetting the position.       *\n"
                "*                                                *\n"
                "**************************************************\n"
                "\n");

                // displaying the position
                printf("x = %d, z = %d\n",n1,-n2);
                printf("\e[?25l"); // to hide the cursor 

                // displaying the hoist
                for(i=0;i<n1;i++) printf("="); printf("o");
                for(i=n1+1;i<max_col;i++) printf("=");
                printf("\n");
                for(i=0;i<n2;i++)
                {
                    for(j=0;j<n1;j++) printf(" ");
                    printf("o\n");
                }

                // send signal to watchdog (we are not inactive!)
                char line[32];
                FILE *cmd = popen("pidof watchdog", "r");
                fgets(line, 32, cmd);
                pid_t pid_watchdog = strtoul(line, NULL, 10);
                kill(pid_watchdog, SIGUSR1);
                pclose(cmd);
            } 
        }
    } 
    return 0; 
} 

// signal handler function
void sig_handler (int signo) 
{
    // position reset handler
    if (signo == SIGTSTP) 
    {
        kill(child1_pid, SIGUSR2);
        kill(child2_pid, SIGUSR2);
    }

    // emergency stop handler
    else if (signo == SIGINT) 
    {
        kill(child1_pid, SIGUSR1);
        kill(child2_pid, SIGUSR1);
    }  
}
