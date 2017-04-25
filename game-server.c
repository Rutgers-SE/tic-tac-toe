#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>



int
main(int argc, char** argv)
{
  B(board);
  init_board(board);
  while (1)
    {
      print_board(board);
      int winner = determine_winner(board);
      if (winner != _)
        {
          printf("Winner is %c\n", character_representation(winner));
          break;
        }

      print_repl(MOVE);

      char buf[128];
      readstr(buf);
      int row, col;

      if (parse_coords(buf, &row, &col))
        {
          continue;
        }

      board_place_piece(board, row, col, X);
    }

  return 0;
}
