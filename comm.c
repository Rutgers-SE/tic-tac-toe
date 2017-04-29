#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "comm.h"

// comm
struct ConPair create_udp_socket(int port) {
  struct ConPair cp;
  if ((cp.descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Error creating the socket.");
    exit(1);
  }

  bzero(&cp.info, sizeof(cp.info));
  cp.info.sin_family = AF_INET;
  cp.info.sin_addr.s_addr = htonl(INADDR_ANY);
  cp.info.sin_port = htons(port);

  return cp;
}

int mch_full(struct Match *match) {
  // Return 1 if both players are ready, else return 0
  return (match->player_one.status == P_READY) &&
         (match->player_two.status == P_READY);
}

struct Match *get_pending_or_empty_match(struct GameServer *gs, int *gi) {
  int i;
  for (i = 0; i < 256; i++) {
    if (gs->matches[i].status == M_PENDING ||
        gs->matches[i].status == M_EMPTY) {
      *gi = i;
      return (gs->matches + i);
    }
  }
  return NULL;
}

int resp_ok(char *response) {
  strcpy(response, "ok ");
  return 3;
}

/**
 * Notify players will send the current state of the match to the players
 * who are participating.
 */
int notify_players(struct GameServer *gs, int match_index) {
  /* NOTE: this code is gross */
  /* Yeah, I'm not touching this */
  char response[256];
  int offset = 0;
  offset = resp_ok(response); /* make the response ok.... BAD */
  offset = offset + board_to_string(
                        response + offset, match_index,
                        gs->matches[match_index].board); /* write the board */
  int cur_off = offset;
  if (gs->matches[match_index].player_one.status == P_READY) {
    if (gs->matches[match_index].whos_turn == 1)
      strcpy(response + cur_off, "t1;");
    else if (gs->matches[match_index].whos_turn == 2)
      strcpy(response + cur_off, "t0;");
    printf("Sending: %s\n", response);
    sendto(gs->cp.descriptor, response, strlen(response), 0,
           (SA *)&(gs->matches[match_index].player_one.info),
           sizeof(gs->matches[match_index].player_one.info));
  }
  if (gs->matches[match_index].player_two.status == P_READY) {
    if (gs->matches[match_index].whos_turn == 1)
      strcpy(response + cur_off, "t0;");
    else if (gs->matches[match_index].whos_turn == 2)
      strcpy(response + cur_off, "t1;");
    printf("Sending: %s\n", response);
    sendto(gs->cp.descriptor, response, strlen(response), 0,
           (SA *)&(gs->matches[match_index].player_two.info),
           sizeof(gs->matches[match_index].player_two.info));
  }
  return 0;
}

/**
 * Send board state through the wire
 */
int board_to_string(char *output, int match_index, int (*board)[3]) {
  sprintf(output, "m%i b%d-%d-%d-%d-%d-%d-%d-%d-%d ", match_index, 
          board[0][0], board[0][1], board[0][2], 
          board[1][0], board[1][1], board[1][2],
          board[2][0], board[2][1], board[2][2]);
  return strlen(output);
}

void board_print_from_string(char *board_string) {
  int board[3][3];
  sscanf(board_string, "%d-%d-%d-%d-%d-%d-%d-%d-%d", 
         board[0] + 0, board[0] + 1, board[0] + 2, 
         board[1] + 0, board[1] + 1, board[1] + 2, 
         board[2] + 0, board[2] + 1, board[2] + 2);
  print_board(board);
}

int gs_join(struct GameServer *gs, SAI client, int *gi) {
  // get empty pending match
  struct Match *match = get_pending_or_empty_match(gs, gi);
  if (match == NULL)
    return 1;
  // insert client
  if (mch_add_player(match, client))
    return 1;
  // check match status
  if (mch_full(match)) {
    match->status = M_READY;
    match->whos_turn = 1;
  }

  return 0;
}

/**
 * Return -1 on error, 0 on success
 */
int gs_leave(struct GameServer *gs, SAI client, int gi) { return 0; }

void init_game_server(struct GameServer *gs) { bzero(gs, sizeof(*gs)); }

int mch_add_player(struct Match *match, SAI pin) {
  if (match->player_one.status == P_EMPTY &&
      match->player_two.info.sin_port != pin.sin_port) {
    match->player_one.info = pin;
    match->player_one.status = P_READY;
    return 0;
  }
  if (match->player_two.status == P_EMPTY &&
      match->player_one.info.sin_port != pin.sin_port) {
    match->player_two.info = pin;
    match->player_two.status = P_READY;
    return 0;
  }
  return 1;
}

void init_board(B(board)) {
  int x, y;
  for (x = 0; x < 3; x++)
    for (y = 0; y < 3; y++)
      board[x][y] = _;
}

char character_representation(int c) {
  IS_VALID_PEICE(c);
  switch (c) {
  case 0:
    return ' ';
  case 1:
    return 'X';
  case 2:
    return 'O';
  }
  return ' ';
}

int determine_winner(int (*board)[3]) {
  // check rows
  // checking columns
  int row;
  for (row = 0; row < 3; row++) {
    if (((board[row][0] == board[row][1] && board[row][1] == board[row][2] &&
          board[row][2] == board[row][0]) ||
         (board[0][row] == board[1][row] && board[1][row] == board[2][row] &&
          board[2][row] == board[0][row])) &&
        board[row][0] != _) {
      return board[row][0];
    }
  }

  // checking diagonals

  return _;
}

void print_board(int (*board)[3]) {
  printf("    0   1   2  \n");
  printf("  +---+---+---+\n");
  int row, col;
  for (row = 0; row < 3; row++) {
    for (col = 0; col < 3; col++) {
      if (col == 0)
        printf("%i ", row);
      printf("| %c ", character_representation(board[row][col]));
    }
    printf("|\n  +---+---+---+\n");
  }
}

void print_repl(int mode) {
  IS_VALID_MODE(mode);
  switch (mode) {
  case MOVE:
    printf("move> ");
    break;
  case WAIT:
    printf("WAIT.\n");
    break;
  }
}
/////////////////////////////////////////////////////////////////
// DEFINING PROTOCOL FUNCTIONS
/////////////////////////////////////////////////////////////////

/*
 * Returns  1 if space is already taken;
 * Returns  0 if piece placed.
 * Returns -1 if invalid arguments.
 */
int board_place_piece(int (*board)[3], int row, int col, int value) {
  if (!(value == X || value == O) ||
      !(0 <= col && col <= 2 && 0 <= row && row <= 2)) {
    return -1;
  }
  if (board[row][col] != _)
    return 1;
  board[row][col] = value;
  return 0;
}

// PARSE AND PACKERS --------------------------------------------------

int parse_motion_command(char *cmd, int *gid, int *pid, int *row, int *col) {
  int outcome = 0;
  return outcome;
}

int pack_motion_command(char *cmd, int gid, int pid, int row, int col);

void pack_match_id(char *command, int match_index) {
  sprintf(command, "%s m%i", command, match_index);
}

// END PARSE AND PACKERS ----------------------------------------------

void readstr(char *buf) {
  int idx = 0;
  char c;
  while ((c = getchar()) != '\n') {
    buf[idx] = c;
    idx++;
  }
  buf[idx] = '\0';
}

int parse_coords(char *buf, int *row, int *col) {
  if (strlen(buf) > 2 || strlen(buf) < 2) {
    return 1; // invalid string length;
  }

  char rows[2];
  rows[0] = buf[0];
  rows[1] = '\0';
  char cols[2];
  cols[0] = buf[1];
  cols[1] = '\0';

  *col = atoi(cols);
  *row = atoi(rows);

  return 0;
}

void com_parse_board_string(char *response, char *board_string) {
  /* NOTE: this can be refactored */
  int start = 0;
  for (; (response[start] != ' ' || response[start + 1] != 'b') &&
         start + 1 < CMDLEN;
       start++)
    ;
  start += 2;
  int end = start;
  for (; response[end] != ' ' && end < CMDLEN; end++)
    ;
  strncpy(board_string, response + start, end - start);
  board_string[end - start] = '\0';
}

/**
 * Return -1 on error
 * return index on success
 */
int com_parse_match_index(char *response, int len) {
  char match_index_string[CMDLEN];
  com_parse_char_command(match_index_string, response, 'm');
  /* int start = 0; */
  /* while(response[start] != ' ' && response[start + 1] != 'm' && start+1 < len) */
  /*   start++; */
  /* start += 2; */
  /* int end = start; */
  /* while (response[end] != ' ' && end < len) */
  /*   end++; */
  /* strncpy(match_index_string, response + start, end-start); */
  return atoi(match_index_string);
}

// tele
int com_parse_command(char *command, char *request) {
  char req[CMDLEN];
  char *com;
  memcpy(req, request, CMDLEN);
  com = strtok(req, " ;");
  memcpy(command, com, strlen(com));
  command[strlen(com)] = 0;
  return 0;
}

/**
 * Return 1 when the response was positive
 * Return 0 when the response was positive
 */
int com_response_ok(char *response, unsigned int len) {
  char first_token[len];
  int i = 0;
  for (; response[i] != ' ' && i < CMDLEN; i++) {
    first_token[i] = response[i];
  }
  first_token[i + 1] = 0;

  if (strcmp(first_token, "ok") == 0) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * Parses that command thooooo
 */
void com_parse_motion(char *response, struct Motion *motion) {
  char dest[3];        /* the space allocation is 3 because #-# */
  com_parse_char_command(dest, response, 'p'); /* p for play */
  sscanf(dest, "%i-%i", &(motion->row), &(motion->column));
}


void com_parse_char_command(char *dest, char *src, char tag) {
  int start = 0;
  while (!(src[start]==' ' && src[start+1]==tag) && start+1 < CMDLEN)
    start++;
  start += 2;
  int end = start;
  while (src[end] != ' ' && src[end] != '\0' && end < CMDLEN)
    end++;
  strncpy(dest, src + start, end - start);
}
