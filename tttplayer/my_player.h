#pragma once
#include <ostream>
#include "player.h"
#include <iostream>
#include <vector>
#include <map>

/**
 * This is example player, that plays absolutely randomly.
 */
class RandomPlayer : public Player {
    std::string _name;
public:
    RandomPlayer(const std::string& name): _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override { /*does nothing*/ }
    void notify(const GameView&, const Event&) override { /*does nothing*/ }
};

// мой

class MyPlayer : public Player {
    std::string _name;
    Mark myMark = Mark::None;
    int myCount = 0;
    int delta = 1; // для ограничения квадратов
    int winLength = delta * 2 + 1;
    bool is_win(const GameView& game, const Mark& value, int iter, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[]) const;
    bool check_draw(const bool filled[], const int& size_of_field) const;
    int evaluate(const GameView& game, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[], int it);
    int minimax(const GameView& game, bool is_maximizing, int depth, int iter, bool crosses_cur[], bool filled_cur[], const int& size_of_field, int alpha, int beta);
    Point find_best_move_in_square(const GameView& game, const Point& center, bool crosses_cur[], bool filled_cur[], const int& size_of_field, int& bestValue);

public:
    MyPlayer(const std::string& name) : _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override { myMark = player_mark; }
    void notify(const GameView& game, const Event& event) override {}
};


class RealPlayer : public Player {
    std::string _name;
    Mark myMark = Mark::None; // крестик ходит первым
public:
    RealPlayer(const std::string& name) : _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override { myMark = player_mark; }
    void notify(const GameView& game, const Event& event) override { /*do nothing*/ }

};


/**
 * Simple observer, that logs every event in given output stream.
 */
class BasicObserver : public Observer {
    std::ostream& _out;

    std::ostream& _print_mark(Mark mark);
public:
    BasicObserver(std::ostream& out_stream): _out(out_stream) {}
    void notify(const GameView&, const Event& event) override;
};
