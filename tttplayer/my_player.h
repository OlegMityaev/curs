#pragma once
#include <ostream>
#include "player.h"
#include <iostream>
//#include "Windows.h"
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
    int moves_count = 0;
    int delta = 0; // для ограничения квадратов
    //int winLength = delta * 2 + 1; // для определения победы в ограниченном квадрате
    int winLength = 0;
    Point antiWinMove = Point(-1000, -1000);
    int size_min_field = 1; 
    int m_width = 0, m_height = 0, m_minx = 0, m_miny = 0, m_maxx = 0, m_maxy = 0;
    Point lastMove = Point(0, 0);
    void near_point(const Point& center, bool filled_cur[], bool crosses_cur[], bool filled_min[], bool crosses_min[]);
    bool is_win(const GameView& game, const Mark& value, int iter, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[]);
    int evaluate(const GameView& game, Mark value, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[]);
    int minimax(const GameView& game, bool is_maximizing, int depth, int iter, bool crosses_cur[], bool filled_cur[], const int& size_of_field, int alpha, int beta);
    inline void init(const GameView& game);
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
