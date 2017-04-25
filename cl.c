#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "comm.h"


int
main(int argc, char** argv)
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  printf("lines %d\n", w.ws_row);
  char *command = "join";
  int len = strlen(command);

  struct ConPair cp = create_udp_socket(10001);
  sendto(cp.descriptor, command, len+1,
         0, (SA*)&cp.info, sizeof(cp.info));

  return 0;
}
