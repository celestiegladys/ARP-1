#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>

// defining global variables
int z = 0;
int vel_z = 0;

// pre-deifnition of singal handler function
void sig_handler (int signo);

int main() 
{
    // defining variables for named pipes 
    int fd1, fd2, fd3;
    char * myfifo2 = "/tmp/myfifo2"; 
    mkfifo(myfifo2, 0666); 
    char * myfifo4 = "/tmp/myfifo4"; 
    mkfifo(myfifo4, 0666); 
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
    
    fd3 = open("logfiles/z_data.log",O_RDWR | O_CREAT | O_TRUNC, 0644);

    while(1) 
    { 
        // initialising IPC between motor_z and command console
        fd1 = open(myfifo2,O_RDWR);
        FD_ZERO(&fds);
        FD_SET(fd1, &fds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        retval = select(fd1+1, &fds, NULL, NULL, &tv);

        if (retval == -1) perror("select()");
        else if (retval == 1)   // read input velocity from command console
        {
            read(fd1, buffer1, 80); 
            if(buffer1[0] == 'w') vel_z--;
            if(buffer1[0] == 'x') vel_z++;
            if(buffer1[0] == 's') vel_z = 0;
        } 
        else                   // send  position to the inspection console
        {
            // initialising IPC between motor_z and inspection console
            fd2 = open(myfifo4,O_WRONLY);
            sprintf(buffer2,"%d",z);
            write(fd2, buffer2, sizeof(buffer2));
            close(fd2);

            // writing motor position data to log file
            fd3 = open("logfiles/z_data.log",O_RDWR | O_APPEND);
            sprintf(buffer2,"%d\n",z);
            if(z<10) write(fd3, buffer2, 2);
            else write(fd3, buffer2, 3);
            close(fd3);
        }
        close(fd1);

        // applying the position limits
        if(z>9) 
        {
            z=10;
            if(vel_z > 0) vel_z = 0;
        }
        if(z<1) 
        {
            z=0;
            if(vel_z < 0) vel_z = 0;
        }

        // moving the hoist along the z axis
        z = z + vel_z;

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
        z = 0;
        vel_z = 0;
    }

    // emergency stop handler
    else if (signo == SIGUSR1) vel_z = 0;
}
