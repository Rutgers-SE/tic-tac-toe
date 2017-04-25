#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "comm.h"


// comm
struct ConPair
create_udp_socket(int port)
{
  struct ConPair cp;
  if ((cp.descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      printf("Error creating the socket.");
      exit(1);
    }

  bzero(&cp.info, sizeof(cp.info));
  cp.info.sin_family = AF_INET;
  cp.info.sin_addr.s_addr = htonl(INADDR_ANY);
  cp.info.sin_port = htons(port);

  return cp;
}



int
gs_join(SAI client, char* response)
{
  // get empty pending match

  // insert client

  return 0;
}

int
gs_leave(SAI client, int game_id, char* response)
{
  return 0;
}





void
init_board(B(board))
{
  int x, y;
  for (x=0;x<3;x++)
    for (y=0;y<3;y++)
      board[x][y]=_;
}

char
character_representation(int c)
{
  IS_VALID_PEICE(c);
  switch (c)
    {
    case 0: return ' ';
    case 1: return 'X';
    case 2: return 'O';
    }
  return ' ';
}

int
determine_winner(int board[3][3])
{
  // check rows
  // checking columns
  int row;
  for (row=0; row<3; row++)
    {
      if (((board[row][0] == board[row][1] &&
            board[row][1] == board[row][2] &&
            board[row][2] == board[row][0]) ||
           (board[0][row] == board[1][row] &&
            board[1][row] == board[2][row] &&
            board[2][row] == board[0][row])) &&
          board[row][0] != _)
        {
          return board[row][0];
        }
    }

  // checking diagonals

  return _;
}


void
print_board(int board[3][3])
{
  printf("    0   1   2  \n");
  printf("  +---+---+---+\n");
  int row, col;
  for (row=0; row < 3; row++)
    {
      for (col=0; col < 3; col++)
        {
          if (col == 0) printf("%i ", row);
          printf("| %c ",
                 character_representation(board[row][col]));
        }
      printf("|\n  +---+---+---+\n");
    }
}


void
print_repl(int mode)
{
  IS_VALID_MODE(mode);
  switch (mode)
    {
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
int
board_place_piece(int board[3][3], int row, int col, int value)
{
  if (!(value==X || value==O) ||
      !(0<=col && col<=2 && 0<=row && row<=2))
    {
      return -1;
    }
  if (board[row][col] != _)
    return 1;
  board[row][col]=value;
  return 0;
}

// PARSE AND PACKERS --------------------------------------------------

int
parse_motion_command(char* cmd, int* gid, int* pid, int* row, int* col)
{
  int outcome = 0;
  return outcome;
}

int pack_motion_command(char* cmd, int gid, int pid, int row, int col);

// END PARSE AND PACKERS ----------------------------------------------

void
readstr(char* buf)
{
  int idx = 0;
  char c;
  while ((c = getchar()) != '\n')
    {
      buf[idx] = c;
      idx++;
    }
  buf[idx] = '\0';
}

int
parse_coords(char* buf, int* row, int* col)
{
  if (strlen(buf) > 2 || strlen(buf) < 2)
    {
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


// tele
int
com_parse_command(char* command, char* request)
{
  char req[CMDLEN];
  memcpy(req, request, CMDLEN);
  command = strtok(req, " ;");
  return 0;
}
