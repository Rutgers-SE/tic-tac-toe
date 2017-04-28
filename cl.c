#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "comm.h"

int main(int argc, char **argv) {
  int port = 10001;
  if (argc >= 2)
    port = atoi(argv[1]);
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  printf("lines %d\n", w.ws_row);
  char *command = "join;";
  int len = strlen(command);

  struct ConPair cp = create_udp_socket(port);

  sendto(cp.descriptor, command, len, 0, (SA *)&cp.info, sizeof(cp.info));

  SAI client_in;
  socklen_t client_len;
  ssize_t n = recvfrom(cp.descriptor, command, CMDLEN, 0, (SA *)&client_in,
                       &client_len);
  printf("%i\n", (int)n);

  return 0;
}
