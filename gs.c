#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

int main(int argc, char **argv) {
  int port = 10001;
  if (argc >= 2)
    port = atoi(argv[1]);

  struct GameServer gs;
  bzero(&gs, sizeof(gs));
  gs.cp = create_udp_socket(10001);
  bind(gs.cp.descriptor, (SA *)&gs.cp.info, sizeof(gs.cp.info));

  // gameloop
  while (1) {
    char request[CMDLEN];
    SAI client_in;
    socklen_t client_len;
    int bytes_recv = recvfrom(gs.cp.descriptor, request, CMDLEN, 0,
                              (SA *)&client_in, &client_len);
    request[bytes_recv] = '\0';
    printf("Received %i\n", bytes_recv);
    char command[12];
    com_parse_command(command, request);
    if (strcmp(command, "join") == 0) {
      int match_index;
      gs_join(&gs, client_in, &match_index);
      notify_players(&gs, match_index);
    }

    else if (strcmp(command, "move") == 0) {
      char response[256];
    }
  }

  return 0;
}
