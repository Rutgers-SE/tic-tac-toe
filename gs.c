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
      if (gs_join(&gs, client_in, &match_index)) {
        /* failed to join the match: server probably full */
        char *response = "bad iserver-full";
        if (sendto(gs.cp.descriptor, response, strlen(response), 0,
                   (SA *)&client_in, sizeof(client_in)) < 0 ) {
          /* there was an error sending error message. LOL */
          printf("Error sending message to client.");
        }
        continue;
      }
      if (notify_players(&gs, match_index) < 0) {
        printf("Error notifying Players\n");
      }
    }

    else if (strcmp(command, "leave") == 0) {
      /* NOTE: handle client leaving operation */
      /* parse the incoming response */
      char response[CMDLEN];
      bzero(response, CMDLEN);

      /* get the match id */
      int match_index = com_parse_match_index(request, CMDLEN);
      int player_who_left;
      struct Match *match = gs.matches + match_index;
      if (mch_leave(match, client_in.sin_port, &player_who_left)) {
        sprintf(response, "bad iyou-are-not-even-a-player...-weirdo");
        cp_send(gs.cp.descriptor, response, (SA *)&client_in);
      } else {
        sprintf(response, "ok m-1 iNICE-you-are-a-quiter");
        cp_send(gs.cp.descriptor, response, (SA *)&client_in);

        /* notify other player */
        if (player_who_left == 1) {
          /* notify player 2 */
          if (match->player_two.status == P_READY) {
            cp_send(gs.cp.descriptor, "ok ithe-other-player-has-left", (SA *)&match->player_two.info);
          }
        } else {
          /* notify player 1 */
          if (match->player_one.status == P_READY) {
            cp_send(gs.cp.descriptor, "ok ithe-other-player-has-left", (SA *)&match->player_one.info);
          }
        }
      }
      continue;
    }

    else if (!strcmp(command, "move")) {
      /* handle client motion command */

      /* parse the incoming response */
      char response[CMDLEN];
      bzero(response, CMDLEN);

      /* get the match id */
      int match_index = com_parse_match_index(request, CMDLEN);

      /* get the match structure from the the game server */
      struct Match *match = gs.matches + match_index;

      /* determine if the player trying to make the move is authorized to do so */
      if (mch_players_turn(match, client_in.sin_port)) {
        /* the player trying to make the move is not allowed to at the time. */
        sprintf(response, "bad inot-your-turn");
        if (sendto(gs.cp.descriptor, response, strlen(response), 0,
                   (SA *)&client_in, sizeof(client_in)) < 0) {
          perror("There was an error sending the message back to the player");
        }
        continue;
      }

      /* get the clients attempted motion */
      struct Motion motion;
      com_parse_motion(request, &motion);

      /* place the piece on the board */
      board_place_piece(match->board, motion.row, motion.column, match->whos_turn);
      mch_toggle_turn(match);

      /* determine the winner */
      int winner = determine_winner(match->board);
      if (winner != _) {
        /* there is a winner! */
        /* setup the response */
        char win_response[CMDLEN];
        bzero(win_response, CMDLEN);
        char lose_response[CMDLEN];
        bzero(lose_response, CMDLEN);
        char board[17];
        bzero(board, 17);

        board_to_string(board, -1, match->board); /* convert the board to a string */
        sprintf(win_response, "ok iyou-have-won%s", board);
        sprintf(lose_response, "ok iyou-have-lost %s", board);

        /* handle the case of a winner */
        if (winner == 1) {
          /* player 1 is the winner */
          cp_send(gs.cp.descriptor, win_response, (SA*)&match->player_one.info);
          cp_send(gs.cp.descriptor, lose_response, (SA*)&match->player_two.info);
        } else {
          /* player 2 is the winner */
          cp_send(gs.cp.descriptor, win_response, (SA*)&match->player_two.info);
          cp_send(gs.cp.descriptor, lose_response, (SA*)&match->player_one.info);
        }


        bzero(match, sizeof(*match)); /* remove players from match */

        continue;
      }


      /* the case when there it no winner */
      /* convert the board to a string */
      char board[17];
      bzero(board, 17);
      board_to_string(board, match_index, match->board); /* convert the board to a string */
      sprintf(response, "ok %s ", board);              /* put the board in the response */

      printf("Response: %s\n", response);

      char m_resp[CMDLEN], nm_resp[CMDLEN];
      strcpy(m_resp, response);
      strcpy(nm_resp, response);
      sprintf(m_resp, "%s t1", m_resp);
      sprintf(nm_resp, "%s t0", nm_resp);

      /* sending the response to the players */
      if (match->whos_turn == 1) {
        /* player 1's turn */
        cp_send(gs.cp.descriptor, m_resp, (SA*)&match->player_one.info);
        cp_send(gs.cp.descriptor, nm_resp, (SA*)&match->player_two.info);
      } else {
        /* player 2's turn */
        cp_send(gs.cp.descriptor, m_resp, (SA*)&match->player_two.info);
        cp_send(gs.cp.descriptor, nm_resp, (SA*)&match->player_one.info);
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
