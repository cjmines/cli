#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <ncurses.h>
#include <unordered_map>

// Define Direction enum class
enum class Direction { up, down, left, right };

struct Cell {
  bool is_mine = false;     ///< Indicates if the cell contains a mine.
  bool is_revealed = false; ///< Indicates if the cell has been revealed.
  bool is_flagged = false;  ///< Indicates if the cell is flagged.
  int adjacent_mines = 0;   ///< Number of adjacent mines.
};

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
      // TODO this whole if statement is bad
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
          if (is_cursor) {
            attron(COLOR_PAIR(color_pair));
            printw("F ");
            attroff(COLOR_PAIR(color_pair));
          } else {
            attron(COLOR_PAIR(11));
            printw("F ");
            attroff(COLOR_PAIR(11));
          }
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

void toggle_flag_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row >= 0 && row < board.size() && col >= 0 && col < board[0].size() &&
      !board[row][col].is_revealed) {
    board[row][col].is_flagged = !board[row][col].is_flagged;
  }
}

void flag_cell(std::vector<std::vector<Cell>> &board, int row, int col) {
  if (row >= 0 && row < board.size() && col >= 0 && col < board[0].size() &&
      !board[row][col].is_revealed) {
    board[row][col].is_flagged = true;
  }
}

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

bool field_clear(std::vector<std::vector<Cell>> &board) {
  int remaining_mines = 0;
  for (const auto &row : board) {
    for (const auto &cell : row) {
      bool mine_is_flagged = cell.is_mine && cell.is_flagged;
      bool non_mine_is_revealed = !cell.is_mine && cell.is_revealed;
      bool cell_correct = mine_is_flagged or non_mine_is_revealed;
      if (not cell_correct) {
        return false;
      }
    }
  }
  // all cells are correct
  return true;
}

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

using KeyMap = std::unordered_map<Direction, int>;

// Vim-style map for direction keys
KeyMap vim_map = {{Direction::up, static_cast<int>('k')},
                  {Direction::down, static_cast<int>('j')},
                  {Direction::left, static_cast<int>('h')},
                  {Direction::right, static_cast<int>('l')}};

KeyMap regular_map = {{Direction::up, KEY_UP},
                      {Direction::down, KEY_DOWN},
                      {Direction::left, KEY_LEFT},
                      {Direction::right, KEY_RIGHT}};

KeyMap selected_map = regular_map;

// Function to create action map with captured variables
auto create_action_map(int &cursor_row, int &cursor_col,
                       std::vector<std::vector<Cell>> &board, bool &game_over,
                       int height, int width, int mines_count) {
  return std::unordered_map<int, std::function<void()>>{
      {selected_map[Direction::up],
       [&]() { cursor_row = std::max(0, cursor_row - 1); }},
      {selected_map[Direction::down],
       [&]() {
         cursor_row =
             std::min(static_cast<int>(board.size()) - 1, cursor_row + 1);
       }},
      {selected_map[Direction::left],
       [&]() { cursor_col = std::max(0, cursor_col - 1); }},
      {selected_map[Direction::right],
       [&]() {
         cursor_col =
             std::min(static_cast<int>(board[0].size()) - 1, cursor_col + 1);
       }},
      {'d',
       [&]() {
         if (!reveal_cell(board, cursor_row, cursor_col)) {
           game_over = true;
           mvprintw(board.size() + 2, 0,
                    "Game over! Press 'r' to restart or 'q' to quit.");
         }
       }},
      {'D',
       [&]() {
         if (!reveal_adjacent_cells(board, cursor_row, cursor_col)) {
           game_over = true;
           mvprintw(board.size() + 2, 0,
                    "Game over! Press 'r' to restart or 'q' to quit.");
         }
       }},
      {'f', [&]() { toggle_flag_cell(board, cursor_row, cursor_col); }},
      {'F', [&]() { flag_adjacent_cells(board, cursor_row, cursor_col); }},
      {'q', [&]() { game_over = true; }},
      {'r', [&]() {
         board =
             std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
         initialize_board(board, mines_count);
         cursor_row = 0;
         cursor_col = 0;
         game_over = false;
       }}};
}

int start_game(int argc, char *argv[]) {
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
  bool user_requested_quit;

  auto action_map = create_action_map(cursor_row, cursor_col, board, game_over,
                                      height, width, mines_count);

  while (!game_over) {
    display_board(board, cursor_row, cursor_col);
    int ch = getch();

    if (action_map.find(ch) != action_map.end()) {
      action_map[ch]();
    }

    if (field_clear(board)) {
      game_over = true;
      mvprintw(board.size() + 2, 0, "You won, well done");
    }
  }

  endwin();
  return 0;
}
