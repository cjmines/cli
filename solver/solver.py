from itertools import combinations
from typing import List, Tuple, Dict, Set, Optional

class MinesweeperCSP:
    def __init__(self, board: List[str], width: int, height: int):
        self.board: List[str] = board
        self.width: int = width
        self.height: int = height
        self.variables: Dict[Tuple[int, int], Set[int]] = {}  # Maps cell positions to domain {0, 1}
        self.constraints: List[Tuple[List[Tuple[int, int]], int]] = []  # List of constraints
        self.opened_cells: Dict[Tuple[int, int], int] = {}  # Cells that have been opened (position -> number of adjacent mines)
        self.flagged_cells: Set[Tuple[int, int]] = set()  # Cells that have been flagged as mines

    def add_variable(self, position: Tuple[int, int], domain: Set[int]) -> None:
        print(f"Adding variable for position {position} with domain {domain}")
        self.variables[position] = domain

    def add_constraint(self, cells: List[Tuple[int, int]], value: int) -> None:
        print(f"Adding constraint: Sum of cells {cells} must equal {value}")
        self.constraints.append((cells, value))

    def is_consistent(self, assignment: Dict[Tuple[int, int], int]) -> bool:
        for cells, value in self.constraints:
            total = sum(assignment.get(cell, 0) for cell in cells)
            if total != value:
                print(f"Inconsistent: Sum of cells {cells} = {total}, expected {value}")
                return False
        return True

    def backtracking_search(self, assignment: Dict[Tuple[int, int], int] = {}) -> Optional[Dict[Tuple[int, int], int]]:
        if len(assignment) == len(self.variables):
            print(f"Solution found: {assignment}")
            return assignment

        # Select the next variable to assign
        unassigned_vars = [v for v in self.variables if v not in assignment]
        var = unassigned_vars[0]
        print(f"Assigning variable {var}")

        for value in self.variables[var]:
            new_assignment = assignment.copy()
            new_assignment[var] = value
            print(f"Trying value {value} for variable {var}")

            if self.is_consistent(new_assignment):
                print(f"Assignment {var} = {value} is consistent")
                result = self.backtracking_search(new_assignment)
                if result is not None:
                    return result
            else:
                print(f"Assignment {var} = {value} is not consistent, backtracking")

        print(f"Backtracking from variable {var}")
        return None

    def solve(self) -> Optional[Dict[Tuple[int, int], int]]:
        print("Starting CSP Solver")
        return self.backtracking_search()

    def open_cell(self, y: int, x: int) -> None:
        """Open a cell on the board."""
        if (y, x) in self.opened_cells or (y, x) in self.flagged_cells:
            return

        cell_value = self.board[y][x]
        if cell_value == 'M':
            print(f"Opened a mine at {(y, x)} - game over!")
            return
        elif cell_value == '0':
            print(f"Opened cell {(y, x)} with 0 adjacent mines")
            self.opened_cells[(y, x)] = 0
        else:
            num_mines = int(cell_value)
            print(f"Opened cell {(y, x)} with {num_mines} adjacent mines")
            self.opened_cells[(y, x)] = num_mines

        # Add constraints based on newly opened cell
        adjacent_cells = [(ny, nx) for ny in range(max(0, y-1), min(self.height, y+2))
                          for nx in range(max(0, x-1), min(self.width, x+2))
                          if (ny, nx) != (y, x) and (ny, nx) not in self.opened_cells and (ny, nx) not in self.flagged_cells]
        self.add_constraint(adjacent_cells, num_mines)
        for cell in adjacent_cells:
            if cell not in self.variables:
                self.add_variable(cell, {0, 1})

    def flag_cell(self, y: int, x: int) -> None:
        """Flag a cell as a mine."""
        if (y, x) not in self.opened_cells and (y, x) not in self.flagged_cells:
            print(f"Flagged cell {(y, x)} as a mine")
            self.flagged_cells.add((y, x))
            if (y, x) in self.variables:
                del self.variables[(y, x)]  # Remove flagged cells from variables list

def create_minesweeper_csp(board: List[str]) -> MinesweeperCSP:
    height = len(board)
    width = len(board[0])
    csp = MinesweeperCSP(board, width, height)

    for y in range(height):
        for x in range(width):
            cell_value = board[y][x]
            if cell_value != 'M' and cell_value != '0':
                num_mines = int(cell_value)  # Convert string number to integer
                adjacent_cells = [(ny, nx) for ny in range(max(0, y-1), min(height, y+2))
                                  for nx in range(max(0, x-1), min(width, x+2))
                                  if (ny, nx) != (y, x) and board[ny][nx] == '0']
                csp.add_constraint(adjacent_cells, num_mines)
                for cell in adjacent_cells:
                    if cell not in csp.variables:
                        csp.add_variable(cell, {0, 1})  # Safe or mine
    return csp

# Example usage
board = [
    "11100",
    "1M210",
    "12M10",
    "12210",
    "1M100",
]

csp = create_minesweeper_csp(board)
csp.open_cell(0, 0)  # Example of opening a cell
solution = csp.solve()
print("Final solution:", solution)
