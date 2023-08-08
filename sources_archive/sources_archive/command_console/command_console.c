#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>


int main() 
{       
    // defining variables for named pipes
    int fd1, fd2; 
    char * myfifo = "/tmp/myfifo"; 
    mkfifo(myfifo, 0666);
    char * myfifo2 = "/tmp/myfifo2"; 
    mkfifo(myfifo2, 0666);  
    
    char buffer[80];

    while (1) 
    { 
        // displaying the initial message
        system("clear");
        printf(
        "*****************************************************\n"
        "* Control the hoist using the following keys!       *\n"
        "*                                                   *\n"            
        "* w-x : increase/decrease linear velocity in z axis *\n"
        "*  s :stop in z axis                                *\n"
        "*                                                   *\n"   
        "* f-h : increase/decrease linear velocity in x axis *\n"
        "*  g :stop in x axis                                *\n"
        "*                                                   *\n"
        "*****************************************************\n");
        printf("input = ");
        
        // initialising IPC between motor_x,z and command console
        fd1 = open(myfifo, O_WRONLY);
        fd2 = open(myfifo2, O_WRONLY);      
        fflush(stdout);
        fgets(buffer, 80 , stdin); 

        // sending the velocity values to motors
        if((buffer[0]=='w')||(buffer[0]=='s')||(buffer[0]=='x')) write(fd2, buffer, sizeof(buffer)); 
        else if ((buffer[0]=='f')||(buffer[0]=='g')||(buffer[0]=='h')) write(fd1, buffer, sizeof(buffer));

        close(fd1);
        close(fd2); 

        // send signal to watchdog (we are not inactive!)
        char line[32];
        FILE *cmd = popen("pidof watchdog", "r");
        fgets(line, 32, cmd);
        pid_t pid_watchdog = strtoul(line, NULL, 10);
        kill(pid_watchdog, SIGUSR1);
        pclose(cmd);
    } 
    return 0;
} 
