#include <cstdlib>
#include <vector>
#include <ctime>
#include <unordered_map>

// Define Direction enum class
enum class Direction { up, down, left, right };

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
 * @brief Initializes the Minesweeper board with mines and adjacent mine
 * counts.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param mines_count Number of mines to place on the board.
 */
void initialize_board(std::vector<std::vector<Cell>> &board, int mines_count);

/**
 * @brief Initializes the ncurses environment and color pairs.
 */
void initialize_ncurses();

/**
 * @brief Gets the color pair for a specific number and cursor state.
 *
 * @param number The number on the cell (0-8) or special value for mine or flag.
 * @param is_cursor Boolean flag indicating if the cell is under the cursor.
 *
 * @return The color pair number.
 */
int get_color_pair(int number, bool is_cursor);


/**
 * @brief Displays the Minesweeper board with colored cells and cursor.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param cursor_row Current row of the cursor.
 * @param cursor_col Current column of the cursor.
 */
void display_board(const std::vector<std::vector<Cell>> &board, int cursor_row,
                   int cursor_col); 
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
bool reveal_cell(std::vector<std::vector<Cell>> &board, int row, int col);

/**
 * @brief Reveals all adjacent cells of a specified cell on the Minesweeper
 * board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the center cell.
 * @param col Column index of the center cell.
 */
bool reveal_adjacent_cells(std::vector<std::vector<Cell>> &board, int row,
                           int col);

/**
 * @brief Flags or unflags a cell on the Minesweeper board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the cell to flag or unflag.
 * @param col Column index of the cell to flag or unflag.
 */
void toggle_flag_cell(std::vector<std::vector<Cell>> &board, int row, int col);
/**
 * @brief Flags a cell on the Minesweeper board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the cell to flag
 * @param col Column index of the cell to flag
 */
void flag_cell(std::vector<std::vector<Cell>> &board, int row, int col);

/**
 * @brief Flags all adjacent cells of a specified cell on the Minesweeper board.
 *
 * @param board 2D vector representing the Minesweeper board.
 * @param row Row index of the center cell.
 * @param col Column index of the center cell.
 */
void flag_adjacent_cells(std::vector<std::vector<Cell>> &board, int row,
                         int col); 
/**
 * @brief If the minefield has successfully been cleared
 */
bool field_clear(std::vector<std::vector<Cell>> &board); 
/**
 * @brief Displays the help text for the command-line Minesweeper game.
 */
void display_help(); 
using KeyMap = std::unordered_map<Direction, int>;

// Vim-style map for direction keys
/* KeyMap vim_map = {{Direction::up, static_cast<int>('k')}, */
/*                   {Direction::down, static_cast<int>('j')}, */
/*                   {Direction::left, static_cast<int>('h')}, */
/*                   {Direction::right, static_cast<int>('l')}}; */

// Function to create action map with captured variables
auto create_action_map(int &cursor_row, int &cursor_col,
                       std::vector<std::vector<Cell>> &board, bool &game_over,
                       int height, int width, int mines_count);
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
int start_game(int argc, char *argv[]);
