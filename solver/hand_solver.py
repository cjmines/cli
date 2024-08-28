from pprint import pprint

class Cell:
    def __init__(self, hidden, flagged, num_adjacent_mines, location):
        self.hidden = hidden
        self.flagged = flagged
        self.num_adjacent_mines = num_adjacent_mines
        self.location = location

    def __repr__(self):
        return f"Cell(hidden={self.hidden}, num_adjacent_mines={self.num_adjacent_mines}, location={self.location})"


def get_hidden_adjacent_cells(board, x, y):
    """
    Given a position (x, y) on the board, returns all hidden cells that are adjacent to the cell at (x, y).

    Args:
        board (list of list of Cell): The game board represented as a 2D list of Cell objects.
        x (int): The row index of the target cell.
        y (int): The column index of the target cell.

    Returns:
        list of Cell: A list of hidden adjacent cells.
    """
    adjacent_cells = []
    rows = len(board)
    cols = len(board[0])

    # Directions for the adjacent cells (8 directions: vertical, horizontal, diagonal)
    directions = [
        (-1, -1), (-1, 0), (-1, 1),
        (0, -1),         (0, 1),
        (1, -1), (1, 0), (1, 1)
    ]

    for direction in directions:
        dx, dy = direction
        nx, ny = x + dx, y + dy

        if 0 <= nx < rows and 0 <= ny < cols:
            adjacent_cell = board[ny][nx]
            if adjacent_cell.hidden:
                adjacent_cells.append(adjacent_cell)

    return adjacent_cells

def build_board_from_text(board_start_state):
    board = []
    state_rows = board_start_state.strip().split("\n")

    for row_idx, state_row in enumerate(state_rows):
        board_row = []
        for col_idx, state_char in enumerate(state_row):
            hidden = state_char == 'H'

            if not hidden:
                num_adjacent_mines = int(state_char)
            else:
                num_adjacent_mines = -1

            cell = Cell(hidden=hidden, flagged=False, num_adjacent_mines=num_adjacent_mines, location=(row_idx, col_idx))
            board_row.append(cell)
        board.append(board_row)

    return board

# def print_board(board):
#     for row in board:
#         print(" ".join(['m' if cell.is_mine else str(cell.num_adjacent_mines) for cell in row]))

def print_board_start_state(board):
    for row in board:
        print(" ".join(['H' if cell.hidden else ('F' if cell.flagged else (str(cell.num_adjacent_mines))) for cell in row]))

def is_valid_location(board, row, col):
    return 0 <= row < len(board) and 0 <= col < len(board[0])

def get_adjacent_cells(board, row, col):
    adj_cells = []
    for r in range(row - 1, row + 2):
        for c in range(col - 1, col + 2):
            if (r != row or c != col) and is_valid_location(board, r, c):
                adj_cells.append(board[r][c])
    return adj_cells

def apply_constraints(board):
    constraints_satisfied = True
    for row in board:
        for cell in row:
            if cell.num_adjacent_mines != 0:  # Not a mine and has a number
            # if cell.num_adjacent_mines != 0 and not cell.is_mine:  # Not a mine and has a number
                adjacent_cells = get_adjacent_cells(board, *cell.location)
                mine_count = sum(1 for adj in adjacent_cells if adj.flagged)
                if mine_count != cell.num_adjacent_mines:
                    constraints_satisfied = False
                    print(f"Constraint failed at {cell.location}: Expected {cell.num_adjacent_mines}, Found {mine_count}")
    return constraints_satisfied

def find_starting_cell(board):
    for row in board:
        for cell in row:
            if not cell.hidden and cell.num_adjacent_mines >= 1:
                return cell.location

    return (0, 0)  # Fallback to (0, 0) if no suitable cell is found

def backtrack(board, row, col):

    print_board_start_state(board)

    pprint(get_adjacent_cells(board, row, col))

    if row == len(board):
        return apply_constraints(board)

    next_row, next_col = (row, col + 1) if col + 1 < len(board[0]) else (row + 1, 0)

    # if board[row][col].is_mine:
    #     return backtrack(board, next_row, next_col)

    for flagged in [True, False]:
        board[row][col].flagged = flagged
        print(f"Assigning cell {board[row][col].location} -> hidden={hidden}")
        print("Board Start State:")
        print_board_start_state(board)

        if apply_constraints(board):
            print("Constraints satisfied.")
            if backtrack(board, next_row, next_col):
                return True
        else:
            print("Constraints not satisfied.")

        board[row][col].hidden = True
        print(f"Backtracking from cell {board[row][col].location}")

    return False

def solve_csp(board_start_state):
    board = build_board_from_text(board_start_state)
    start_row, start_col = find_starting_cell(board)
    print(f"Starting backtracking at cell ({start_row}, {start_col})")
    if backtrack(board, start_row, start_col):
        return board
    else:
        return None

# Example usage
text_board = """
11100
1m210
12m10
12210
1m100
"""

board_start_state = """
HH100
HH210
HHH10
HH210
HH100
"""

solved_board = solve_csp(board_start_state)

if solved_board:
    print("Final solved board:")
    print_board(solved_board)
else:
    print("No solution found.")
