#include "solver.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Function to read the Minesweeper board from a file
std::pair<Board, int> read_board_from_file(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file");
  }

  std::vector<std::vector<Cell>> board;
  std::string line;

  int mine_count = 0;

  while (std::getline(file, line)) {
    std::vector<Cell> row;
    std::istringstream iss(line);
    std::string value;

    while (iss >> value) {
      Cell cell;
      if (value == "M") {
        cell.is_mine = true;
        mine_count += 1;
      } else {
        cell.adjacent_mines = std::stoi(value);
      }
      row.push_back(cell);
    }
    board.push_back(row);
  }

  file.close();
  return {board, mine_count};
}

Board generate_board(int mines_count, int height, int width) {
  std::vector<std::vector<Cell>> board(height, std::vector<Cell>(width));
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
  return board;
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
        } else if (curr_cell.safe_start) {
          attron(COLOR_PAIR(1));
          printw("X ");
          attroff(COLOR_PAIR(1));
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
  if (row < 0 || row >= board.size() || col < 0 || col >= board.at(0).size() ||
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
      << "  --ng               Produce a no guess board (default: false)\n"
      << "  --vim              Enable vim mode controls for movement (default: "
         "false)\n"
      << "  --file <value>     Loads a minefield from the file, when used with "
         "--ng it checks to see if the board is ngsolvable\n"
      << "  --help             Display this help message\n";
}

using KeyMap = std::unordered_map<Direction, int>;

// Function to create action map with captured variables
auto create_action_map(int &cursor_row, int &cursor_col,
                       std::vector<std::vector<Cell>> &board, bool &game_over,
                       int height, int width, int mines_count, bool &vim) {

  // Vim-style map for direction keys
  KeyMap vim_map = {{Direction::up, static_cast<int>('k')},
                    {Direction::down, static_cast<int>('j')},
                    {Direction::left, static_cast<int>('h')},
                    {Direction::right, static_cast<int>('l')}};

  KeyMap regular_map = {{Direction::up, KEY_UP},
                        {Direction::down, KEY_DOWN},
                        {Direction::left, KEY_LEFT},
                        {Direction::right, KEY_RIGHT}};

  KeyMap selected_map = vim ? vim_map : regular_map;

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
         /* board = */
         /*     std::vector<std::vector<Cell>>(height,
          * std::vector<Cell>(width)); */
         /* generate_board(board, mines_count); */
         /* cursor_row = 0; */
         /* cursor_col = 0; */
         /* game_over = false; */
       }}};
}

bool handle_command_line_args(int argc, char *argv[], int &width, int &height,
                              int &mines_count, bool &no_guess, bool &vim,
                              std::string &file_path) {
  // Handle command-line arguments
  bool early_return = false;
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--width" && i + 1 < argc) {
      width = std::stoi(argv[++i]);
    } else if (arg == "--height" && i + 1 < argc) {
      height = std::stoi(argv[++i]);
    } else if (arg == "--mines" && i + 1 < argc) {
      mines_count = std::stoi(argv[++i]);
    } else if (arg == "--ng") {
      no_guess = true;
    } else if (arg == "--vim") {
      vim = true;
    } else if (arg == "--file" && i + 1 < argc) {
      file_path = argv[++i];
    } else if (arg == "--help") {
      display_help();
      early_return = true;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      display_help();
      early_return = true;
    }
  }
  return early_return;
}

int start_game(int argc, char *argv[]) {

  int width = 10;
  int height = 10;
  int mine_count = 10;
  bool no_guess = false;
  bool vim = false;
  std::string file_path;

  bool early_return = handle_command_line_args(
      argc, argv, width, height, mine_count, no_guess, vim, file_path);

  std::srand(std::time(0));

  Board board;

  bool uses_file = not file_path.empty();

  if (uses_file) {
    auto pair = read_board_from_file(file_path);
    board = pair.first;
    mine_count = pair.second;
  } else {
    board = generate_board(mine_count, width, height);
  }

  if (no_guess) {
    Solver solver;

    if (uses_file) {
      // check if the board is ng solvable
      std::optional<std::pair<int, int>> solution =
          solver.solve(board, mine_count);
      if (solution.has_value()) {
        std::cout << "file board is ngs" << std::endl;
      } else {
        std::cout << "file board is not ngs" << std::endl;
      }
    } else {
      // keep trying until we genrate a ngsolvable board
      std::optional<std::pair<int, int>> solution =
          solver.solve(board, mine_count);

      while (not solution.has_value()) {
        std::cout << "gnerating a new board and trying again" << std::endl;
        board = generate_board(mine_count, width, height);
        solution = solver.solve(board, mine_count);
      }

      auto [row, col] = solution.value();
      board[row][col].safe_start = true;
    }
  }

  initialize_ncurses();

  int cursor_row = 0;
  int cursor_col = 0;
  bool game_over = false;
  bool user_requested_quit;

  auto action_map = create_action_map(cursor_row, cursor_col, board, game_over,
                                      height, width, mine_count, vim);

  auto start_time = std::chrono::high_resolution_clock::now();

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

  if (field_clear(board)) {
    std::cout << "Well done, you've won :)" << std::endl;
  } else {
    std::cout << "You hit a mine :(" << std::endl;
  }

  auto end_time = std::chrono::high_resolution_clock::now();

  // Calculate the elapsed time
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  // Extract minutes, seconds, and milliseconds
  auto minutes =
      std::chrono::duration_cast<std::chrono::minutes>(duration).count();
  duration -= std::chrono::minutes(minutes);
  auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  duration -= std::chrono::seconds(seconds);
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  // Output the elapsed time in minutes, seconds, and milliseconds
  std::cout << "Game duration: " << minutes << " minutes, " << seconds
            << " seconds, " << milliseconds << " milliseconds" << std::endl;

  return 0;
}
