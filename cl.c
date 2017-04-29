#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "comm.h"

struct client_state_t {
  int match_index;
};

void client_state_to_str(struct client_state_t cs, char *buf) {
  sprintf(buf, "M=%i", cs.match_index);
}

void readline(char *buffer) {
  char c;
  int idx = 0;
  while ((c = getchar()) != '\n') {
    buffer[idx] = c;
    idx++;
  }
}

int main(int argc, char **argv) {
  int port = 10001;
  if (argc >= 2)
    port = atoi(argv[1]);
  /* struct winsize w; */
  /* ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); */

  /* printf("lines %d\n", w.ws_row); */

  /* connect to the udp socket */
  struct ConPair cp = create_udp_socket(port);
  struct client_state_t client_state;
  /* the default index will be -1. this is because the client is not connected to a match */
  client_state.match_index = -1;
  char client_state_string[CMDLEN];

  while (1) {

    client_state_to_str(client_state, client_state_string);
    printf("TTT:[%s]> ", client_state_string); /* repl prompt */

    char command[CMDLEN];
    bzero(command, CMDLEN); /* reset command to 0s */
    readline(command);      /* read user line */
    int command_len = strlen(command);
    if (command_len == 0) {
      /* there was no command supplied, continue */
      printf("Please supply a demand.\n");
      continue;
    }

    char operator[CMDLEN];
    com_parse_command(operator, command);
    if (strcmp(operator, "join") == 0) {
      // handle what happens after joining a match

      /* attempt to join match */
      if (sendto(cp.descriptor, command, command_len, 0, (SA *)&cp.info,
                 sizeof(cp.info)) < 0) {
        perror("Error Sending Command");
        exit(1);
      }

      /* receive response from match server */
      SAI client_in;
      socklen_t client_len = sizeof(client_in);
      char response[CMDLEN];
      bzero(response, CMDLEN);
      if (recvfrom(cp.descriptor, response, CMDLEN, 0, (SA *)&client_in,
                   &client_len) < 0) {
        perror("Error");
        exit(1);
      }

      if (com_response_ok(response, CMDLEN)) {
        /* handle the case when accepted into the match */
        printf("%s\n", response);

        client_state.match_index = com_parse_match_index(response, CMDLEN);
        printf("match index: %i\n", client_state.match_index);

        client_state_to_str(client_state, client_state_string);
        printf("TTT:[%s]< The command was successful\n", client_state_string);

        char board_string[CMDLEN];
        com_parse_board_string(response, board_string);
        printf("Parsed board: %s\n", board_string);
        board_print_from_string(board_string);

      } else {
        /* Handle the case when you could not join a match */
        printf("%s\n", response);
        client_state_to_str(client_state, client_state_string);
        printf("TTT:[]< Could not connect to match\n");
      }
    }
    else if (strcmp(operator, "move") == 0) {

      pack_match_id(command, client_state.match_index); /* append the match index to the command string */

      /* attempt to send move command */
      if (sendto(cp.descriptor, command, strlen(command), 0, (SA *)&cp.info,
                 sizeof(cp.info)) < 0) {
        perror("Error Sending Command");
        exit(1);
      }

      /* this is the response from the server */
      char response[CMDLEN];
      bzero(response, CMDLEN);

      SAI client_in;
      socklen_t client_len = sizeof(client_in);

      if (recvfrom(cp.descriptor, response, CMDLEN, 0, (SA *)&client_in,
                   &client_len) < 0) {
        perror("Error");
        exit(1);
      }

      if (com_response_ok(response, CMDLEN)) {
        /* handle the case when accepted into the match */
        printf("%s\n", response);

        client_state.match_index = com_parse_match_index(response, CMDLEN);
        printf("match index: %i\n", client_state.match_index);

        client_state_to_str(client_state, client_state_string);
        printf("TTT:[%s]< The command was successful\n", client_state_string);

        char board_string[CMDLEN];
        com_parse_board_string(response, board_string);
        printf("Parsed board: %s\n", board_string);
        board_print_from_string(board_string);

      } else {
        printf("Not your turn\n");
      }
    }
    else {
      printf("ERR: >> unsupported command <<\n");
    }
  }

  close(cp.descriptor);



  return 0;
}
