Programming Language:
The programming language used for this task is Python.
Version: Python 3.11.5

Code Structure:
The `red_blue_nim_alpha.py` file is the main implementation of the red-blue-nim game, structured to efficiently handle both "standard" and "misère" versions of the game. The code begins with importing the `sys` module, which is essential for accessing command-line arguments. These arguments allow the user to specify the initial conditions of the game, such as the number of marbles in each pile, the version of the game to be played, who starts first, and the depth of the MinMax algorithm if depth limitation is used.
The first function, `parse_arguments()`, is responsible for parsing these command-line inputs. It checks the number of arguments provided and assigns default values if some arguments are not given. The function extracts and returns the number of red and blue marbles, the game version (either "standard" or "misère"), the first player (either "computer" or "human"), and the depth for the MinMax search. If the required arguments are missing, the function provides usage instructions and exits.
The game logic is encapsulated in the `RedBlueNim` class. The constructor method, `__init__()`, initializes the game state by setting up the number of marbles in each pile, the game version, the first player, the depth for the MinMax search, and the current turn. This class also contains several methods to manage the game. The `is_game_over()` method checks if the game has ended by verifying if either pile of marbles is empty. The `evaluate()` method calculates and returns the score based on the current game state, with different scoring logic depending on whether the game is in "standard" or "misère" mode.
To generate possible moves, the `get_possible_moves()` method creates a list of all valid actions, which involve removing 1 or 2 marbles from either the red or blue pile. The `apply_move()` method updates the game state by subtracting the specified number of marbles from the chosen pile, while the `undo_move()` method allows the game to revert to a previous state by adding the marbles back, which is particularly useful for the MinMax algorithm during its search process.
The MinMax algorithm with Alpha-Beta pruning is implemented in the `minimax()` method. This method recursively explores possible moves to determine the best strategy for the computer player. Depending on whether the game is maximizing or minimizing the score, the method evaluates each move, updates alpha and beta values, and prunes branches of the search tree that are not promising, improving efficiency. The `computer_move()` method then uses this MinMax function to determine the optimal move for the computer, applies the move, and displays the updated game state.
The `human_move()` method handles user input for the human player's turn. It prompts the player to choose a pile and the number of marbles to remove, validates the input, and then updates the game state accordingly. After each move, whether by the computer or the human, the `display_state()` method is called to print the current number of red and blue marbles, allowing players to see the updated game state.
Finally, the `play_game()` method manages the overall flow of the game. It alternates turns between the computer and the human player until the game ends, as determined by the `is_game_over()` method. Once the game concludes, the method calculates and prints the winner and the final score based on the remaining marbles. This method ensures a smooth gameplay experience, guiding the game from start to finish.
The code is designed to be run from the command line, with specific instructions provided in the `parse_arguments()` function. The main function, `main()`, initializes the game by creating an instance of the `RedBlueNim` class with the parsed arguments and then starts the game loop. Overall, the code is modular, with clear separation of concerns, making it both easy to understand and maintain.

How to Run the Code:
1. Make sure Python 3.11.5 is installed on your system.
2. Save the provided code into a file named `red_blue_nim_alpha.py`.
3. Open a terminal on visual studio code through Anaconda.
4. Navigate to the directory where `red_blue_nim_alpha.py` is saved.
5. Run the program using the following command: python red_blue_nim_alpha.py <num-red> <num-blue> <version> <first-player> <depth>

Evaluation Function Reasoning for alpha beta pruning:

In the Red-Blue-Nim game, the evaluation function is designed to align with the game's win/loss conditions:
- Standard Version: The player who empties a pile loses. The function returns a negative score when the game ends to penalize losing states, and a positive score during play to reflect control over the game.
- Misère Version: The player who empties a pile wins. The function returns a positive score when the game ends, rewarding winning states. During play, it still uses a positive score to assess the potential to force the opponent into a losing move.

The evaluation function effectively guides the computer's decision-making by scoring game states based on their favorability towards achieving the win conditions specific to each game version.
