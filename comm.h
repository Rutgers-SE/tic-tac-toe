#ifndef COMM_H_
#include <arpa/inet.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>

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
#define IS_VALID_PEICE(c) assert(c == X || c == O || c == _)
#define IS_VALID_MODE(m) assert(m == MOVE || m == WAIT)

#define M_EMPTY 0
#define M_PENDING 1
#define M_READY 2
#define M_INPROGRESS 4

// socket stuff
#define SAI struct sockaddr_in
#define SA struct sockaddr



#define NOTIFY_PLAYER(p1, num)                                \
  if (gs->matches[match_index].p1.status == P_READY) {        \
    if (gs->matches[match_index].whos_turn == num) {          \
      strcpy(response + cur_off, "t1;");                      \
    } else {                                                  \
      strcpy(response + cur_off, "t0;");                      \
    }                                                         \
    printf("Sending: %s\n", response);                        \
    sendto(gs->cp.descriptor, response, strlen(response), 0,  \
           (SA *)&(gs->matches[match_index].p1.info),         \
           sizeof(gs->matches[match_index].p1.info));         \
  }                                                           \


struct ConPair {
  int descriptor;
  SAI info;
};
struct ConPair create_udp_socket(int port);
void cp_send(int descriptor, char *response, SA *info);
int cp_recv(int fd, void *buf, SA *sinfo, socklen_t *slen);

/////////////////////////////////////////////////////////////////
// DEFINING STRUCTS
/////////////////////////////////////////////////////////////////
struct Motion {
  int game_id;
  int row;
  int column;
};

#define P_EMPTY 0
#define P_READY 1

struct Player {
  struct sockaddr_in info;
  int status;
};

struct Match {
  int board[3][3];
  struct Player player_one;
  struct Player player_two;
  int whos_turn;
  int rounds;
  int status;
};

struct GameServer {
  struct Match matches[256];
  struct sockaddr_in info;
  struct ConPair cp;
};

void init_board(B(board));
char character_representation(int c);

int com_response_ok(char *, unsigned int);
int com_parse_command(char *, char *);
int com_parse_match_index(char *, int);
int com_parse_board_string(char *, char *);
void com_parse_motion(char *response, struct Motion *motion);
int com_parse_char_command(char *dest, char *src, char tag);
int com_parse_turn(char *command, int *turn);
int com_parse_info_string(char *response, char *message);

int gs_join(struct GameServer *gs, SAI client, int *gi);
int gs_leave(struct GameServer *gs, SAI client, int gi);
int gs_move(struct GameServer *gs, SAI client, int gi);

int board_to_string(char *output, int match_index, int (*board)[3]);
void board_print_from_string(char *);

int determine_winner(int (*board)[3]);
void print_board(int (*board)[3]);
void print_repl(int mode);
void readstr(char *buf);
int parse_coords(char *buf, int *row, int *col);
int parse_motion_command(char *cmd, int *gid, int *pid, int *row, int *col);
int board_place_piece(int (*board)[3], int row, int col, int value);

int mch_add_player(struct Match *match, SAI pin);
int mch_toggle_turn(struct Match *match);
int mch_players_turn(struct Match *match, int port);
int mch_leave(struct Match *match, int port, int *player_who_left);

int notify_players(struct GameServer *gs, int match_index);

// pack set
void pack_match_id(char *command, int match_index);

#endif
