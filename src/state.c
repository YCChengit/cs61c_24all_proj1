#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  // TODO: Implement this function.
  game_state_t *state = malloc(sizeof(game_state_t));
  if (state == NULL) {
    return NULL;
  }
  state->num_rows = 18;
  state->board = malloc(state->num_rows * sizeof(char *));
  if (state->board == NULL) {
    free(state);
    return NULL;
  }
  for (unsigned int i = 0; i < state->num_rows; i++) {
    state->board[i] = malloc(21 * sizeof(char));
    if (state->board[i] == NULL) {
      for (unsigned int j = 0; j < i; j++) {
        free(state->board[j]);
      }
      free(state->board);
      free(state);
      return NULL;
    }
    state->board[i][20] = '\0';
    if (i == 0 || i == state->num_rows - 1) {
      for (unsigned int j = 0; j < 20; j++) {
        set_board_at(state, i, j, '#');
      }
    } else {
      set_board_at(state, i, 0, '#');
      set_board_at(state, i, 19, '#');
      for (unsigned int j = 1; j < 19; j++) {
        set_board_at(state, i, j, ' ');
      }
    }
  }
  state->num_snakes = 1;
  state->snakes = malloc(sizeof(snake_t));
  if (state->snakes == NULL) {
    for (unsigned int i = 0; i < state->num_rows; i++) {
      free(state->board[i]);
    }
    free(state->board);
    free(state);
    return NULL;
  }
  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;
  set_board_at(state, 2, 2, 'd');
  set_board_at(state, 2, 3, '>');
  set_board_at(state, 2, 4, 'D');
  set_board_at(state, 2, 9, '*');
  return state;
}

/* Task 2 */
void free_state(game_state_t *state) {
  // TODO: Implement this function.
  if (state->snakes != NULL) {
    free(state->snakes);
  }
  for (unsigned int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s\n", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
    return true;
  } else {
    return false;
  }
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
    return true;
  } else {
    return false;
  }
  }

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  if (c == 'w' || c == 'a' || c == 's' || c == 'd' || c == '^' || c == '<' || c == 'v' || c == '>' || c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
    return true;
  } else {
    return false;
  }
  }

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  if (c == '^') {
    return 'w';
  } else if (c == '<') {
    return 'a';
  } else if (c == 'v') {
    return 's';
  } else if (c == '>') {
    return 'd';
  } else {
    return '?';
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  if (c == 'W') {
    return '^';
  } else if (c == 'A') {
    return '<';
  } else if (c == 'S') {
    return 'v';
  } else if (c == 'D') {
    return '>';
  } else {
    return '?';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  if (c == 'v' || c == 's' || c == 'S') {
    return cur_row + 1;
  } else if (c == '^' || c == 'w' || c == 'W') {
    return cur_row - 1;
  } else {
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if (c == '>' || c == 'd' || c == 'D') {
    return cur_col + 1;
  } else if (c == '<' || c == 'a' || c == 'A') {
    return cur_col - 1;
  } else {
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cur_row = state->snakes[snum].head_row;
  unsigned int cur_col = state->snakes[snum].head_col;
  unsigned int next_row = get_next_row(cur_row, state->board[cur_row][cur_col]);
  unsigned int next_col = get_next_col(cur_col, state->board[cur_row][cur_col]);
  char next_char = state->board[next_row][next_col];
  return next_char;
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cur_row = state->snakes[snum].head_row;
  unsigned int cur_col = state->snakes[snum].head_col;
  char cur_char = state->board[cur_row][cur_col];
  unsigned int next_row = get_next_row(cur_row, cur_char);
  unsigned int next_col = get_next_col(cur_col, cur_char);
  set_board_at(state, next_row, next_col, cur_char);
  state->snakes[snum].head_row = next_row;
  state->snakes[snum].head_col = next_col;
  state->board[cur_row][cur_col] = head_to_body(cur_char);
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cur_row = state->snakes[snum].tail_row;
  unsigned int cur_col = state->snakes[snum].tail_col;
  char cur_char = state->board[cur_row][cur_col];
  unsigned int next_row = get_next_row(cur_row, cur_char);
  unsigned int next_col = get_next_col(cur_col, cur_char);
  char next_char = state->board[next_row][next_col];
  set_board_at(state, cur_row, cur_col, ' ');
  state->snakes[snum].tail_row = next_row;
  state->snakes[snum].tail_col = next_col;
  state->board[next_row][next_col] = body_to_tail(next_char);
  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    if (state->snakes[i].live) {
      char next_char = next_square(state, i);
      if (next_char == ' ') {
        update_head(state, i);
        update_tail(state, i);
      } else if (next_char == '*') {
        update_head(state, i);
        add_food(state);
      } else {
        state->snakes[i].live = false;
        unsigned int cur_row = state->snakes[i].head_row;
        unsigned int cur_col = state->snakes[i].head_col;
        state->board[cur_row][cur_col] = 'x';
      }
    }
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  size_t buffer_size = 1024;
  size_t current_length = 0;
  char *line = malloc(buffer_size);
  if (line == NULL) {
    return NULL;
  }

  while (fgets(line + current_length, buffer_size - current_length, fp)) {
    current_length += strlen(line + current_length);

    // Check if we have read the entire line
    if (line[current_length - 1] == '\n' || feof(fp)) {
      break;
    }

    // Extend the buffer if necessary
    buffer_size *= 2;
    char *new_line = realloc(line, buffer_size);
    if (new_line == NULL) {
      free(line);
      return NULL;
    }
    line = new_line;
  }

  if (current_length == 0 && feof(fp)) {
    free(line);
    return NULL;
  }

  return line;
}


/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  game_state_t *state = malloc(sizeof(game_state_t));
  state->snakes = NULL;
  state->num_snakes = 0;
  if (state == NULL) {
    return NULL;
  }
  state->num_rows = 0;
  state->board = NULL;
  char *line = read_line(fp);
  while (line != NULL) {
    state->board = realloc(state->board, (state->num_rows + 1) * sizeof(char *));
    if (state->board == NULL) {
      free(state);
      return NULL;
    }
    state->board[state->num_rows] = malloc(strlen(line) + 1);
    if (state->board[state->num_rows] == NULL) {
      for (unsigned int i = 0; i < state->num_rows; i++) {
        free(state->board[i]);
      }
      free(state->board);
      free(state);
      return NULL;
    }
    for (unsigned int i = 0; i < strlen(line); i++) {
      if (line[i] == '\n') {
        state->board[state->num_rows][i] = '\0';
      } else {
        state->board[state->num_rows][i] = line[i];
      }
    }
    state->num_rows++;
    line = read_line(fp);
  }
  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cur_row = state->snakes[snum].tail_row;
  unsigned int cur_col = state->snakes[snum].tail_col;
  unsigned int next_row = 0; 
  unsigned int next_col = 0;
  while (!is_head(state->board[cur_row][cur_col])) {
    next_row = get_next_row(cur_row, state->board[cur_row][cur_col]);
    next_col = get_next_col(cur_col, state->board[cur_row][cur_col]);
    cur_row = next_row;
    cur_col = next_col;
  }
  state->snakes[snum].head_row = cur_row;
  state->snakes[snum].head_col = cur_col;
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_rows; i++) {
    for (unsigned int j = 0; j < strlen(state->board[i]); j++) {
      if (is_tail(state->board[i][j])) {
        state->num_snakes++;
        state->snakes = realloc(state->snakes, state->num_snakes * sizeof(snake_t));
        if (state->snakes == NULL) {
          return NULL;
        }
        state->snakes[state->num_snakes - 1].tail_row = i;
        state->snakes[state->num_snakes - 1].tail_col = j;
        state->snakes[state->num_snakes - 1].live = true;
        find_head(state, state->num_snakes - 1);
      }
    }
  }
  return state;
}
