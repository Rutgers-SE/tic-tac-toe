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
  /* struct winsize w; */
  /* ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); */

  /* printf("lines %d\n", w.ws_row); */

  char command[CMDLEN] = "join;";      /* define the message to be sent */
  int len = strlen(command);    /* get the length of the message */

  struct ConPair cp = create_udp_socket(port); /* create the udp socket */

  if (sendto(cp.descriptor, command, len, 0, (SA *)&cp.info, sizeof(cp.info)) < 0) {
    perror("Error Sending");
    exit(1);
  }

  printf("%i\n", cp.descriptor);

  SAI client_in;
  socklen_t client_len = sizeof(client_in);
  if (recvfrom(cp.descriptor, command, CMDLEN, 0, (SA *)&client_in,
               &client_len) < 0) {
    perror("Error");
    exit(1);
  }
  printf("command: %s\n", command);

  return 0;
}
