#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <vector>

const int BOARD_ROWS = 10;
const int BOARD_COLS = 10;
const int MINES_COUNT = 10;

/**
 * @brief Represents the state of a cell in the Minesweeper board.
 */
struct Cell {
  bool is_mine = false;     ///< Indicates if the cell contains a mine.
  bool is_revealed = false; ///< Indicates if the cell has been revealed.
  bool is_flagged = false;  ///< Indicates if the cell is flagged.
  int adjacent_mines = 0;   ///< Number of adjacent mines.
};

/**
 * @brief Initializes the Minesweeper board with mines and adjacent mine counts.
 *
 * @param board 2D vector representing the Minesweeper board.
 */
void initialize_board(std::vector<std::vector<Cell>> &board) {
  board.resize(BOARD_ROWS, std::vector<Cell>(BOARD_COLS));

  std::srand(std::time(0));
  int mines_placed = 0;
  while (mines_placed < MINES_COUNT) {
    int row = std::rand() % BOARD_ROWS;
    int col = std::rand() % BOARD_COLS;
    if (!board[row][col].is_mine) {
      board[row][col].is_mine = true;
      mines_placed++;
    }
  }

  for (int row = 0; row < BOARD_ROWS; row++) {
    for (int col = 0; col < BOARD_COLS; col++) {
      if (!board[row][col].is_mine) {
        int count = 0;
        for (int i = -1; i <= 1; i++) {
          for (int j = -1; j <= 1; j++) {
            int r = row + i;
            int c = col + j;
            if (r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS &&
                board[r][c].is_mine) {
              count++;
            }
          }
        }
        board[row][col].adjacent_mines = count;
      }
    }
  }
}

/**
 * @brief Initializes the ncurses environment and color pairs.
 */
void initialize_ncurses() {
  initscr();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);

  start_color();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);   // Default background
  init_pair(2, COLOR_BLUE, COLOR_BLACK);    // Number 1
  init_pair(3, COLOR_GREEN, COLOR_BLACK);   // Number 2
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);  // Number 3
  init_pair(5, COLOR_RED, COLOR_BLACK);     // Number 4
  init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Number 5
  init_pair(7, COLOR_CYAN, COLOR_BLACK);    // Number 6
  init_pair(8, COLOR_MAGENTA, COLOR_BLACK); // Number 7
  init_pair(9, COLOR_WHITE,
            COLOR_BLACK); // Number 0 (black background, white text)
  init_pair(10, COLOR_BLACK, COLOR_MAGENTA); // Cursor (golden with black text)
}

/**
 * @brief Gets the color pair for a specific number and cursor state.
 *
 * @param number The number on the cell (0-8) or special value for mine or flag.
 * @param is_cursor Boolean flag indicating if the cell is under the cursor.
 *
 * @return The color pair number.
 */
int get_color_pair(int number, bool is_cursor) {
  if (is_cursor) {
    return 10; // Cursor color: gold on black
  }
  switch (number) {
  case 0:
    return 9; // Black background with white text
  case 1:
    return 2; // Blue
  case 2:
    return 3; // Green
  case 3:
    return 4; // Yellow
  case 4:
    return 5; // Red
  case 5:
    return 6; // Pink
  case 6:
    return 7; // Lime green
  case 7:
    return 8; // Purple
  default:
    return 1; // Default color for mines or flags
  }
}

/**
 * @brief Displays the Minesweeper board with colored cells and cursor.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param cursor_row Current row of the cursor.
 * @param cursor_col Current column of the cursor.
 */
void display_board(const std::vector<std::vector<Cell>> &board, int cursor_row,
                   int cursor_col) {
  int remaining_mines = MINES_COUNT;
  for (const auto &row : board) {
    for (const auto &cell : row) {
      if (cell.is_flagged) {
        remaining_mines--;
      }
    }
  }

  mvprintw(0, 0, "Remaining mines: %d", remaining_mines);
  move(1, 0);

  for (int row = 0; row < BOARD_ROWS; row++) {
    for (int col = 0; col < BOARD_COLS; col++) {

      Cell curr_cell = board[row][col];

      bool is_cursor = (row == cursor_row && col == cursor_col);
      int color_pair = get_color_pair(curr_cell.adjacent_mines, is_cursor);

      // draw all unopened cells gray color
      if (!curr_cell.is_revealed and !curr_cell.is_flagged) {
        if (is_cursor) {
          attron(COLOR_PAIR(10));
          printw("* ");
          attroff(COLOR_PAIR(10));
          continue;
        } else {
          attron(COLOR_PAIR(1));
          printw("* ");
          attroff(COLOR_PAIR(1));
          continue;
        }
      } else { // its either revelead or it's a flag
        attron(COLOR_PAIR(color_pair));
        if (curr_cell.is_flagged) {
          printw("F ");
        } else if (board[row][col].is_mine) {
          printw("M ");
        } else {
          printw("%d ", board[row][col].adjacent_mines);
        }
        attroff(COLOR_PAIR(color_pair));
      }
    }
    printw("\n");
  }
  refresh();
}

/**
 * @brief Reveals a cell on the Minesweeper board recursively.
 *
 * This function reveals the specified cell and, if it has no adjacent mines,
 * recursively reveals its neighboring cells.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the cell to reveal.
 * @param col Column index of the cell to reveal.
 *
 * @return True if the cell was revealed successfully, false if a mine was hit.
 */
bool reveal_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS ||
      board[row][col].is_revealed || board[row][col].is_flagged) {
    return true;
  }

  board[row][col].is_revealed = true;

  if (board[row][col].is_mine) {
    return false;
  }

  if (board[row][col].adjacent_mines == 0) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        reveal_cell(board, row + i, col + j);
      }
    }
  }

  return true;
}

/**
 * @brief Flags or unflags a cell on the Minesweeper board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the cell to flag or unflag.
 * @param col Column index of the cell to flag or unflag.
 */
void flag_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS &&
      !board[row][col].is_revealed) {
    board[row][col].is_flagged = !board[row][col].is_flagged;
  }
}

/**
 * @brief Handles the game loop, user input, and game state updates.
 */
void play_game() {
  std::vector<std::vector<Cell>> board;
  initialize_board(board);
  initialize_ncurses();

  int cursor_row = 0, cursor_col = 0;
  bool game_over = false;
  bool game_won = false;

  while (!game_over && !game_won) {
    display_board(board, cursor_row, cursor_col);

    int ch = getch();
    switch (ch) {
    case KEY_UP:
      if (cursor_row > 0)
        cursor_row--;
      break;
    case KEY_DOWN:
      if (cursor_row < BOARD_ROWS - 1)
        cursor_row++;
      break;
    case KEY_LEFT:
      if (cursor_col > 0)
        cursor_col--;
      break;
    case KEY_RIGHT:
      if (cursor_col < BOARD_COLS - 1)
        cursor_col++;
      break;
    case 'f':
      flag_cell(board, cursor_row, cursor_col);
      break;
    case ' ':
      if (!board[cursor_row][cursor_col].is_revealed) {
        game_over = !reveal_cell(board, cursor_row, cursor_col);
      }
      break;
    case 'q':
      game_over = true;
      break;
    }

    bool all_revealed = true;
    for (int row = 0; row < BOARD_ROWS; row++) {
      for (int col = 0; col < BOARD_COLS; col++) {
        if (!board[row][col].is_revealed && !board[row][col].is_mine) {
          all_revealed = false;
          break;
        }
      }
    }

    if (all_revealed) {
      game_won = true;
      mvprintw(BOARD_ROWS + 2, 0,
               "Congratulations! You've cleared all the mines!");
      refresh();
      char choice = getch();
      if (choice == 'y' || choice == 'Y') {
        endwin();
        play_game();
        return;
      }
    }

    if (game_over) {
      mvprintw(BOARD_ROWS + 2, 0,
               "Game Over! Press 'y' to restart or 'q' to quit.");
      refresh();
      char choice = getch();
      if (choice == 'y' || choice == 'Y') {
        endwin();
        play_game();
        return;
      }
    }
  }

  endwin();
}

/**
 * @brief The main function that starts the Minesweeper game.
 *
 * @return 0 on successful execution.
 */
int main() {
    play_game();
    return 0;
}
