#include "comm.h"
#include <arpa/inet.h>
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

    /* se */
    char request[CMDLEN];
    SAI client_in;
    socklen_t client_len = sizeof(client_in);
    printf("Waiting...\n");
    int bytes_recv = recvfrom(gs.cp.descriptor, request, CMDLEN, 0,
                              (SA *)&client_in, &client_len);
    request[bytes_recv] = '\0';

    printf("Received: %s, port: %i, FD: %i\n", request, (client_in.sin_port),
           gs.cp.descriptor);


    /* parse the client's command */
    char command[12];
    com_parse_command(command, request);
    if (strcmp(command, "join") == 0) {
      /* handle players trying to join a match */
      int match_index;
      gs_join(&gs, client_in, &match_index);
      if (notify_players(&gs, match_index) < 0) {
        printf("Error notifying Players\n");
      }
    }

    else if (strcmp(command, "leave") == 0) {
      /* NOTE: handle client leaving operation */
    }

    else if (!strcmp(command, "move")) {
      /* handle client motion command */

      /* parse the incoming response */
      char response[CMDLEN];
      int match_index = com_parse_match_index(request, CMDLEN); /* get the match id */
      struct Motion motion;
      com_parse_motion(request, &motion); /* get the clients attempted motion */
      struct Match *match = gs.matches + match_index; /* get the match structure from the the game server */
      board_place_piece(match->board, motion.row, motion.column, X); /* place the piece on the board */
      mch_toggle_turn(match);


      /* start packing the response */

      /* convert the board to a string */
      char board[17];
      board_to_string(board, match_index, match->board); /* convert the board to a string */
      sprintf(response, "ok %s t1", board);              /* put the board in the response */

      printf("Response: %s\n", response);

      if (sendto(gs.cp.descriptor, response, strlen(response), 0, (SA *)&client_in, sizeof(client_in)) < 0) {
        perror("There was an error sending the information to the client");
        continue;
      }
    }

    else {
      printf("Sending Error Message\n");
      if (sendto(gs.cp.descriptor, "bad ", 4, 0, (SA *)&client_in, client_len) <
          0) {
        perror("Error sending error message");
      }
    }
  }

  return 0;
}
