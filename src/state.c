#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

// Definiciones de funciones de ayuda.
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Tarea 1 */
game_state_t* create_default_state() {
  game_state_t* state = malloc(sizeof(game_state_t));
  if (!state) return NULL;

  state->num_rows = 18;
  state->board = malloc(state->num_rows * sizeof(char*));
  if (!state->board) {
    free(state);
    return NULL;
  }

  for (int i = 0; i < state->num_rows; i++) {
    state->board[i] = malloc(20 * sizeof(char));
    if (!state->board[i]) {
      for (int j = 0; j < i; j++) free(state->board[j]);
      free(state->board);
      free(state);
      return NULL;
    }
    
    if (i == 0 || i == state->num_rows - 1) {
      memset(state->board[i], '#', 20);
    } else {
      memset(state->board[i], ' ', 20);
      state->board[i][0] = '#';
      state->board[i][19] = '#';
    }
  }

  // Add snake and fruit
  state->board[2][2] = 'd';
  state->board[2][3] = '>';
  state->board[2][4] = 'D';
  state->board[2][9] = '*';

  state->num_snakes = 1;
  state->snakes = malloc(sizeof(snake_t));
  if (!state->snakes) {
    for (int i = 0; i < state->num_rows; i++) free(state->board[i]);
    free(state->board);
    free(state);
    return NULL;
  }

  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;

  return state;
}


/* Tarea 2 */
void free_state(game_state_t* state) {
  if (!state) return;
  
  if (state->board) {
    for (int i = 0; i < state->num_rows; i++) {
      if (state->board[i]) free(state->board[i]);
    }
    free(state->board);
  }
  
  if (state->snakes) free(state->snakes);
  free(state);
}


/* Tarea 3 */
void print_board(game_state_t* state, FILE* fp) {
  if (!state || !fp) return;
  
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s\n", state->board[i]);
  }
}


/**
 * Guarda el estado actual a un archivo. No modifica el objeto/struct state.
 * (ya implementada para que la utilicen)
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Tarea 4.1 */


/**
 * Funcion de ayuda que obtiene un caracter del tablero dado una fila y columna
 * (ya implementado para ustedes).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}


/**
 * Funcion de ayuda que actualiza un caracter del tablero dado una fila, columna y
 * un caracter.
 * (ya implementado para ustedes).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}


/**
 * Retorna true si la variable c es parte de la cola de una snake.
 * La cola de una snake consiste de los caracteres: "wasd"
 * Retorna false de lo contrario.
*/
static bool is_tail(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
  return true;
}


/**
 * Retorna true si la variable c es parte de la cabeza de una snake.
 * La cabeza de una snake consiste de los caracteres: "WASDx"
 * Retorna false de lo contrario.
*/
static bool is_head(char c) {
  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x';
  return true;
}


/**
 * Retorna true si la variable c es parte de una snake.
 * Una snake consiste de los siguientes caracteres: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return is_tail(c) || is_head(c) || c == '^' || c == '<' || c == 'v' || c == '>';
  return true;
}


/**
 * Convierte un caracter del cuerpo de una snake ("^<v>")
 * al caracter que correspondiente de la cola de una
 * snake ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
    case '^': return 'w';
    case '<': return 'a';
    case 'v': return 's';
    case '>': return 'd';
    default: return '?';
  }
}


/**
 * Convierte un caracter de la cabeza de una snake ("WASD")
 * al caracter correspondiente del cuerpo de una snake
 * ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
    case 'W': return '^';
    case 'A': return '<';
    case 'S': return 'v';
    case 'D': return '>';
    default: return '?';
  }
}


/**
 * Retorna cur_row + 1 si la variable c es 'v', 's' o 'S'.
 * Retorna cur_row - 1 si la variable c es '^', 'w' o 'W'.
 * Retorna cur_row de lo contrario
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  switch (c) {
    case 'v': case 's': case 'S': return cur_row + 1;
    case '^': case 'w': case 'W': return cur_row - 1;
    default: return cur_row;
  }
  return cur_row; // Retorno por defecto
}



/**
 * Retorna cur_col + 1 si la variable c es '>' or 'd' or 'D'.
 * Retorna cur_col - 1 si la variable c es '<' or 'a' or 'A'.
 * Retorna cur_col de lo contrario
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  switch (c) {
    case '>': case 'd': case 'D': return cur_col + 1;
    case '<': case 'a': case 'A': return cur_col - 1;
    default: return cur_col;
  }
  return cur_col; // Retorno por defecto
}


/**
 * Tarea 4.2
 *
 * Funcion de ayuda para update_state. Retorna el caracter de la celda
 * en donde la snake se va a mover (en el siguiente paso).
 *
 * Esta funcion no deberia modificar nada de state.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  if (!state || snum >= state->num_snakes) return '?';
  
  snake_t snake = state->snakes[snum];
  char head = get_board_at(state, snake.head_row, snake.head_col);
  
  unsigned int next_row = get_next_row(snake.head_row, head);
  unsigned int next_col = get_next_col(snake.head_col, head);
  
  return get_board_at(state, next_row, next_col);
}


/**
 * Tarea 4.3
 *
 * Funcion de ayuda para update_state. Actualiza la cabeza de la snake...
 *
 * ... en el tablero: agregar un caracter donde la snake se va a mover (¿que caracter?)
 *
 * ... en la estructura del snake: actualizar el row y col de la cabeza
 *
 * Nota: esta funcion ignora la comida, paredes, y cuerpos de otras snakes
 * cuando se mueve la cabeza.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  if (!state || snum >= state->num_snakes) return;
  
  snake_t* snake = &state->snakes[snum];
  char head = get_board_at(state, snake->head_row, snake->head_col);
  
  unsigned int new_row = get_next_row(snake->head_row, head);
  unsigned int new_col = get_next_col(snake->head_col, head);
  
  // Convert old head to body
  set_board_at(state, snake->head_row, snake->head_col, head_to_body(head));
  
  // Update head position
  snake->head_row = new_row;
  snake->head_col = new_col;
  set_board_at(state, new_row, new_col, head);
}


/**
 * Tarea 4.4
 *
 * Funcion de ayuda para update_state. Actualiza la cola de la snake...
 *
 * ... en el tablero: colocar un caracter blanco (spacio) donde se encuentra
 * la cola actualmente, y cambiar la nueva cola de un caracter de cuerpo (^<v>)
 * a un caracter de cola (wasd)
 *
 * ...en la estructura snake: actualizar el row y col de la cola
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  if (!state || snum >= state->num_snakes) return;
  
  snake_t* snake = &state->snakes[snum];
  char tail = get_board_at(state, snake->tail_row, snake->tail_col);
  
  unsigned int new_row = get_next_row(snake->tail_row, tail);
  unsigned int new_col = get_next_col(snake->tail_col, tail);
  char next = get_board_at(state, new_row, new_col);
  
  // Clear current tail
  set_board_at(state, snake->tail_row, snake->tail_col, ' ');
  
  // Update tail position and convert next body to tail
  snake->tail_row = new_row;
  snake->tail_col = new_col;
  set_board_at(state, new_row, new_col, body_to_tail(next));
}

/* Tarea 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  if (!state) return;
  
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    if (!state->snakes[i].live) continue;
    
    char next = next_square(state, i);
    if (next == '#' || is_snake(next)) {
      // Snake dies
      state->snakes[i].live = false;
      set_board_at(state, state->snakes[i].head_row, 
                  state->snakes[i].head_col, 'x');
    } 
    else if (next == '*') {
      // Snake eats food - grow by not updating tail
      update_head(state, i);
      add_food(state);
    } 
    else {
      // Normal move
      update_head(state, i);
      update_tail(state, i);
    }
  }
}

/* Tarea 5 */
game_state_t* load_board(char* filename) {
  if (filename == NULL) return NULL;

  FILE* fp = fopen(filename, "r");
  if (fp == NULL) return NULL;

  // Contar filas y columnas máximas
  unsigned int num_rows = 0;
  size_t max_cols = 0;
  char buffer[1024];

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
      num_rows++;
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len-1] == '\n') len--;
      if (len > max_cols) max_cols = len;
  }

  rewind(fp);

  
  // Allocate state
  game_state_t* state = malloc(sizeof(game_state_t));
  if (state == NULL) {
      fclose(fp);
      return NULL;
  }

  state->num_rows = num_rows;
  state->num_snakes = 0;
  state->snakes = NULL;
  state->board = malloc(num_rows * sizeof(char*));
  if (state->board == NULL) {
      fclose(fp);
      free(state);
      return NULL;
  }
  
  // Read each row
  for (unsigned int i = 0; i < num_rows; i++) {
    state->board[i] = NULL;
}

   // Inicializar todo a NULL
   for (unsigned int i = 0; i < num_rows; i++) {
    state->board[i] = NULL;
}

// Leer filas
for (unsigned int i = 0; i < num_rows; i++) {
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        // Limpieza en caso de error
        for (unsigned int j = 0; j < i; j++) free(state->board[j]);
        free(state->board);
        free(state);
        fclose(fp);
        return NULL;
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') buffer[--len] = '\0';

    state->board[i] = malloc(len + 1);
    if (state->board[i] == NULL) {
        // Limpieza en caso de error
        for (unsigned int j = 0; j < i; j++) free(state->board[j]);
        free(state->board);
        free(state);
        fclose(fp);
        return NULL;
    }

    strcpy(state->board[i], buffer);
}

fclose(fp);
return state;
}



/**
 * Tarea 6.1
 *
 * Funcion de ayuda para initialize_snakes.
 * Dada una structura de snake con los datos de cola row y col ya colocados,
 * atravezar el tablero para encontrar el row y col de la cabeza de la snake,
 * y colocar esta informacion en la estructura de la snake correspondiente
 * dada por la variable (snum)
*/
static void find_head(game_state_t* state, unsigned int snum) {
  if (state == NULL || snum >= state->num_snakes) return;
  
  snake_t* snake = &state->snakes[snum];
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;
  char c = get_board_at(state, row, col);
  
  while (!is_head(c)) {
    row = get_next_row(row, c);
    col = get_next_col(col, c);
    c = get_board_at(state, row, col);
  }
  
  snake->head_row = row;
  snake->head_col = col;
}


/* Tarea 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  if (state == NULL || state->board == NULL) return NULL;
  
  // Count number of tails (snakes)
  unsigned int num_snakes = 0;
  for (unsigned int i = 0; i < state->num_rows; i++) {
    for (unsigned int j = 0; j < strlen(state->board[i]); j++) {
      if (is_tail(state->board[i][j])) num_snakes++;
    }
  }
  
  state->num_snakes = num_snakes;
  state->snakes = malloc(num_snakes * sizeof(snake_t));
  if (state->snakes == NULL) return NULL;
  
  // Initialize each snake
  unsigned int idx = 0;
  for (unsigned int i = 0; i < state->num_rows; i++) {
    for (unsigned int j = 0; j < strlen(state->board[i]); j++) {
      if (is_tail(state->board[i][j])) {
        state->snakes[idx].tail_row = i;
        state->snakes[idx].tail_col = j;
        state->snakes[idx].live = true;
        find_head(state, idx);
        idx++;
      }
    }
  }
  
  return state;
}
