/**
 *  This is example of game, that you can use for debugging.
 */
#include <cstdlib>
#include <ctime>

#include "my_player.h"
#include "game_engine.h"


int main() {
    srand(time(NULL));

    /// Creates observer to log every event to std::cout
    BasicObserver obs(std::cout);

    /// Create setting for game
    GameSettings settings = {
        .field_size = {
            .min = {.x = -5, .y = -5},
            .max = {.x = 5, .y = 5},
        },
        .max_moves = 0,
        .win_length = 5,
        .move_timelimit_ms = 0,
    };
    
    /// Creates game engine with given settings
    GameEngine game(settings);

    /// Adds observer to the view of this game engine
    game.get_view().add_observer(obs);
    /// Creates first player
    RandomPlayer player1("Vasya");
    //MyPlayer player1("Oleg");
    //RealPlayer player1("Real1");
    /// Adds it to play Xs
    game.set_player(player1);
    /// Create second player
    //RandomPlayer player2("Kolya");
    //RealPlayer player2("Real2");
    MyPlayer player2("I am");
    /// Adds it to play Os
    game.set_player(player2);
    /// Starts game until someone wins.
    game.play_game();
    return 0;
}
