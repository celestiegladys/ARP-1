#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>


typedef enum {
  false,
  true
} bool;

int const SLEEP_TIMER = 60;
bool isKiller = true;

void sig_handler (int signo);

int main (int argc, char *argv) {
  pid_t pid_command = -1, pid_inspection = -1, pid_motor_x = -1, pid_motor_z = -1;

  // registering signal handler
  struct sigaction sig;
  memset(&sig, 0, sizeof(sig));
  sig.sa_handler = &sig_handler;
  sigaction(SIGUSR1, &sig, NULL);

  // get PID of the processes
  char line[32];
  FILE *cmd1 = popen("pidof command", "r"); // returns both PIDS ("1234 1234")
  fgets(line, 32, cmd1);
  pid_command = strtoul(line,NULL,10);
  pclose(cmd1);

  FILE *cmd2 = popen("pidof inspection", "r"); // returns both PIDS ("1234 1234")
  fgets(line, 32, cmd2);
  pid_inspection = strtoul(line,NULL,10);
  pclose(cmd2);

  FILE *cmd3 = popen("pidof motro_x", "r"); // returns both PIDS ("1234 1234")
  fgets(line, 32, cmd3);
  pid_motor_x = strtoul(line,NULL,10);
  pclose(cmd3);

  FILE *cmd4 = popen("pidof motor_z", "r"); // returns both PIDS ("1234 1234")
  fgets(line, 32, cmd4);
  pid_motor_z = strtoul(line,NULL,10);
  pclose(cmd4);

  printf("Watchdog active. Timer: 60 seconds\n");
  fflush(stdout);

  // start timer to kill processes
  while (true) {
    fflush(stdout);
    sleep(SLEEP_TIMER); // 
    pid_t wait (int *statloc);
    if (isKiller) {
      // kill processes
      printf("Watchdog: Terminating processes...\n");
      kill(pid_command, SIGKILL);
      kill(pid_inspection, SIGKILL);
      kill(pid_motor_x, SIGKILL);
      kill(pid_motor_z, SIGKILL);
      return 0;
    } else {
      // reset "killing"
      isKiller = true;
      printf("Watchdog: Timer reset\n");
    }
  }

  return 0;
}

void sig_handler (int signo) {
  if (signo == SIGUSR1) {
    isKiller = false;
  }
}
