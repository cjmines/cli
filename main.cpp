#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <vector>

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
 * @param mines_count Number of mines to place on the board.
 */
void initialize_board(std::vector<std::vector<Cell>> &board, int mines_count) {
  std::srand(std::time(0));
  int mines_placed = 0;
  while (mines_placed < mines_count) {
    int row = std::rand() % board.size();
    int col = std::rand() % board[0].size();
    if (!board[row][col].is_mine) {
      board[row][col].is_mine = true;
      mines_placed++;
    }
  }

  for (int row = 0; row < board.size(); row++) {
    for (int col = 0; col < board[0].size(); col++) {
      if (!board[row][col].is_mine) {
        int count = 0;
        for (int i = -1; i <= 1; i++) {
          for (int j = -1; j <= 1; j++) {
            int r = row + i;
            int c = col + j;
            if (r >= 0 && r < board.size() && c >= 0 && c < board[0].size() &&
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
  init_pair(11, COLOR_BLACK, COLOR_RED);     // Flag
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
  int remaining_mines = 0;
  for (const auto &row : board) {
    for (const auto &cell : row) {
      if (cell.is_mine && !cell.is_flagged) {
        remaining_mines++;
      }
    }
  }

  mvprintw(0, 0, "Remaining mines: %d", remaining_mines);
  move(1, 0);

  for (int row = 0; row < board.size(); row++) {
    for (int col = 0; col < board[0].size(); col++) {
      Cell curr_cell = board[row][col];
      bool is_cursor = (row == cursor_row && col == cursor_col);
      int color_pair = get_color_pair(curr_cell.adjacent_mines, is_cursor);

      // Draw unopened cells, note all flags are unopened.
      bool is_untouched = !curr_cell.is_revealed && !curr_cell.is_flagged;
      if (is_untouched) {
        if (is_cursor) {
          attron(COLOR_PAIR(10));
          printw("* ");
          attroff(COLOR_PAIR(10));
        } else {
          attron(COLOR_PAIR(1));
          printw("* ");
          attroff(COLOR_PAIR(1));
        }
      } else {
        if (curr_cell.is_flagged) {
          attron(COLOR_PAIR(11));
          printw("F ");
          attroff(COLOR_PAIR(11));
        } else if (curr_cell.is_mine) {
          printw("M ");
        } else {
          attron(COLOR_PAIR(color_pair));
          printw("%d ", curr_cell.adjacent_mines);
          attroff(COLOR_PAIR(color_pair));
        }
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
 * recursively reveals its neighboring cells. If the number of adjacent mines
 * equals the number of adjacent flags, it reveals all non-flagged squares
 * around it.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the cell to reveal.
 * @param col Column index of the cell to reveal.
 *
 * @return True if the cell was revealed successfully, false if a mine was hit.
 */
bool reveal_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row < 0 || row >= board.size() || col < 0 || col >= board[0].size() ||
      board[row][col].is_revealed || board[row][col].is_flagged) {
    return true;
  }

  board[row][col].is_revealed = true;

  if (board[row][col].is_mine) {
    return false;
  }

  int flagged_neighbors = 0;
  int adjacent_count = board[row][col].adjacent_mines;

  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int r = row + i;
      int c = col + j;
      if (r >= 0 && r < board.size() && c >= 0 && c < board[0].size() &&
          board[r][c].is_flagged) {
        flagged_neighbors++;
      }
    }
  }

  if (flagged_neighbors == adjacent_count) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        int r = row + i;
        int c = col + j;
        if (r >= 0 && r < board.size() && c >= 0 && c < board[0].size() &&
            !board[r][c].is_flagged) {
          reveal_cell(board, r, c);
        }
      }
    }
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
 * @brief Reveals all adjacent cells of a specified cell on the Minesweeper
 * board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the center cell.
 * @param col Column index of the center cell.
 */
bool reveal_adjacent_cells(std::vector<std::vector<Cell>> &board, int row,
                           int col) {
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int r = row + i;
      int c = col + j;
      if (!reveal_cell(board, r, c)) {
        return false;
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
void toggle_flag_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row >= 0 && row < board.size() && col >= 0 && col < board[0].size() &&
      !board[row][col].is_revealed) {
    board[row][col].is_flagged = !board[row][col].is_flagged;
  }
}

/**
 * @brief Flags a cell on the Minesweeper board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the cell to flag
 * @param col Column index of the cell to flag
 */
void flag_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row >= 0 && row < board.size() && col >= 0 && col < board[0].size() &&
      !board[row][col].is_revealed) {
    board[row][col].is_flagged = true;
  }
}

/**
 * @brief Flags all adjacent cells of a specified cell on the Minesweeper board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the center cell.
 * @param col Column index of the center cell.
 */
void flag_adjacent_cells(std::vector<std::vector<Cell>> &board, int row,
                         int col) {
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int r = row + i;
      int c = col + j;
      flag_cell(board, r, c);
    }
  }
}

/**
 * @brief Displays the help text for the command-line Minesweeper game.
 */
void display_help() {
  std::cout
      << "Minesweeper Game - Command Line Version\n"
      << "Usage: minesweeper [OPTIONS]\n"
      << "Options:\n"
      << "  --width <value>    Set the width of the board (default: 10)\n"
      << "  --height <value>   Set the height of the board (default: 10)\n"
      << "  --mines <value>    Set the number of mines (default: 10)\n"
      << "  --help             Display this help message\n";
}

/**
 * @brief Main function for the Minesweeper game.
 *
 * Handles command-line arguments and manages the game loop.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 *
 * @return Exit status code.
 */
int main(int argc, char *argv[]) {
  int width = 10;
  int height = 10;
  int mines_count = 10;

  // Handle command-line arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--width" && i + 1 < argc) {
      width = std::stoi(argv[++i]);
    } else if (arg == "--height" && i + 1 < argc) {
      height = std::stoi(argv[++i]);
    } else if (arg == "--mines" && i + 1 < argc) {
      mines_count = std::stoi(argv[++i]);
    } else if (arg == "--help") {
      display_help();
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      display_help();
      return 1;
    }
  }

  std::vector<std::vector<Cell>> board(height, std::vector<Cell>(width));
  initialize_board(board, mines_count);
  initialize_ncurses();

  int cursor_row = 0;
  int cursor_col = 0;
  bool game_over = false;

  while (!game_over) {
    display_board(board, cursor_row, cursor_col);
    int ch = getch();
    switch (ch) {
    case KEY_UP:
      cursor_row = std::max(0, cursor_row - 1);
      break;
    case KEY_DOWN:
      cursor_row = std::min(static_cast<int>(board.size()) - 1, cursor_row + 1);
      break;
    case KEY_LEFT:
      cursor_col = std::max(0, cursor_col - 1);
      break;
    case KEY_RIGHT:
      cursor_col =
          std::min(static_cast<int>(board[0].size()) - 1, cursor_col + 1);
      break;
    case 'd':
      if (!reveal_cell(board, cursor_row, cursor_col)) {
        game_over = true;
        mvprintw(board.size() + 2, 0,
                 "Game over! Press 'r' to restart or 'q' to quit.");
      }
      break;
    case 'D':
      if (!reveal_adjacent_cells(board, cursor_row, cursor_col)) {
        game_over = true;
        mvprintw(board.size() + 2, 0,
                 "Game over! Press 'r' to restart or 'q' to quit.");
      }
      break;
    case 'f':
      toggle_flag_cell(board, cursor_row, cursor_col);
      break;
    case 'F': // Shift-F to flag all adjacent cells
      flag_adjacent_cells(board, cursor_row, cursor_col);
      break;
    case 'q':
      game_over = true;
      break;
    case 'r':
      board = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
      initialize_board(board, mines_count);
      cursor_row = 0;
      cursor_col = 0;
      game_over = false;
      break;
    default:
      break;
    }

    // Check for win condition
    bool all_revealed = true;
    for (const auto &row : board) {
      for (const auto &cell : row) {
        if (!cell.is_mine && !cell.is_revealed) {
          all_revealed = false;
          break;
        }
      }
    }
    if (all_revealed) {
      mvprintw(board.size() + 2, 0,
               "Congratulations! You cleared the board! Press 'r' to restart "
               "or 'q' to quit.");
      game_over = true;
    }
  }

  endwin();
  return 0;
}
