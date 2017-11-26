#include <stdio.h>
#include <signal.h>

int alarmflag = 0;
void alarmhandler(int signo);

void main(void){
  printf("Begining...\n");
  signal(SIGALRM, alarmhandler);
  alarm(5);
  printf("Looping...\n");
  while (!alarmflag)
     pause();
  printf("Ending...\n");
}
void alarmhandler(int signo){
  printf("Alarm received...\n");
  alarmflag = 1;
}