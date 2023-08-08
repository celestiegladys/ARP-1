#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>


// defining global variables
int x = 25;
int vel_x = 0;

// pre-deifnition of singal handler function
void sig_handler (int signo);

int main() 
{
    // defining variables for named pipes  
    int fd1, fd2, fd3;
    char * myfifo = "/tmp/myfifo"; 
    mkfifo(myfifo, 0666); 
    char * myfifo3 = "/tmp/myfifo3"; 
    mkfifo(myfifo3, 0666); 
    char buffer1[80], buffer2[80];

    // defining variables for selecting (non determinism)
    fd_set fds;
    int retval;
    struct timeval tv; 

    // registering signal handler
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = &sig_handler;
    sigaction(SIGUSR1, &sig, NULL);
    sigaction(SIGUSR2, &sig, NULL);
    sigaction(SIGTSTP, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);

    fd3 = open("logfiles/x_data.log",O_RDWR | O_CREAT | O_TRUNC, 0644);
    
    while(1) 
    { 
        // initialising IPC between motor_x and command console
        fd1 = open(myfifo,O_RDWR);
        FD_ZERO(&fds);
        FD_SET(fd1, &fds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        retval = select(fd1+1, &fds, NULL, NULL, &tv);

        if (retval == -1) perror("select()");
        else if (retval == 1)   // read input velocity from command console
        {
            read(fd1, buffer1, 80); 
            if(buffer1[0] == 'f') vel_x--;
            if(buffer1[0] == 'h') vel_x++;
            if(buffer1[0] == 'g') vel_x=0;
        } 
        else                    // send  position to the inspection console
        {
            // initialising IPC between motor_x and inspection console
            fd2 = open(myfifo3,O_WRONLY);
            sprintf(buffer2,"%d",x);
            write(fd2, buffer2, sizeof(buffer2));
            close(fd2);

            // writing motor position data to log file
            fd3 = open("logfiles/x_data.log",O_RDWR | O_APPEND);
            sprintf(buffer2,"%d\n",x);
            if(x<10) write(fd3, buffer2, 2);
            else  write(fd3, buffer2, 3);
            close(fd3);
        }
        close(fd1);

        // applying the position limits
        if(x>49)
        {
            x = 50;
            if(vel_x > 0) vel_x = 0;
        } 
        if(x<1) 
        {
            x = 0;
            if(vel_x < 0) vel_x = 0;
        }

        // moving the hoist along the x axis
        x = x + vel_x;

        // send signal to watchdog 
        char line[32];
        FILE *cmd = popen("pidof watchdog", "r");
        fgets(line, 32, cmd);
        pid_t pid_watchdog = strtoul(line, NULL, 10);
        kill(pid_watchdog, SIGUSR1);
        pclose(cmd);
    } 
    return 0; 
} 

// signal handler function
void sig_handler (int signo) 
{
    // position reset handler
    if (signo == SIGUSR2) 
    {
        x = 25;
        vel_x = 0;
    }

    // emergency stop handler
    else if (signo == SIGUSR1) vel_x = 0; 
}
