#ifndef COMM_H_
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// defining board state
#define MPS 512
#define _ 0
#define X 1
#define O 2
#define MOVE 4
#define WAIT 8
#define CMDLEN 256
// Board helpers
#define B(v) int v[3][3]
#define IS_VALID_PEICE(c) assert(c==X||c==O||c==_)
#define IS_VALID_MODE(m) assert(m==MOVE||m==WAIT)


// socket stuff
#define SAI struct sockaddr_in
#define SA struct sockaddr

struct ConPair
{
  int descriptor;
  SAI info;
};
struct ConPair
create_udp_socket(int port);

/////////////////////////////////////////////////////////////////
// DEFINING STRUCTS
/////////////////////////////////////////////////////////////////
struct Motion
{
  int game_id;
  int row;
  int column;
};



struct Player
{
  char name[256];
  struct sockaddr_in info;
  int desc;
};

struct Match
{
  struct Player player_one;
  struct Player player_two;
  int count;
};

struct
GameServer
{
  struct Match matches[256];
  struct sockaddr_in info;
  struct ConPair cp;
};

void init_board(B(board));
char character_representation(int c);

int com_parse_command(char*, char*);

int gs_join(SAI client, char* response);
int gs_leave(SAI client,  int game_id, char* response);
int gs_move(SAI client, int game_id, char* response);


int determine_winner(int board[3][3]);
void print_board(int board[3][3]);
void print_repl(int mode);
void readstr(char* buf);
int parse_coords(char* buf, int* row, int* col);
int parse_motion_command(char* cmd, int* gid, int* pid, int* row, int* col);
int board_place_piece(int board[3][3], int row, int col, int value);


#endif
