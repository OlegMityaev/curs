#include "my_player.h"
#include <cstdlib>

static field_index_t rand_int(field_index_t min, field_index_t max) {
    return min + rand() % (max - min + 1);
}

Point RandomPlayer::play(const GameView& game) {
    Boundary b = game.get_settings().field_size;
    Point result;
    do {
        result = {
            .x = rand_int(b.min.x, b.max.x),
            .y = rand_int(b.min.y, b.max.y),
        };
    } while(game.get_state().field->get_value(result) != Mark::None);
    return result;
}

bool operator<(const Point& p1, const Point& p2)
{
    int i1 = p1.x + p1.y * 100;
    int i2 = p2.x + p2.y * 100;
    return i1 < i2;
}

bool MyPlayer::is_win(const GameView& game, const Mark& value,
    int iter, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[]) const {
    int width = winLength;
    int height = width;
    int minx, maxx;
    int miny, maxy;

    if (width != game.get_settings().field_size.get_width() || height != game.get_settings().field_size.get_height()) {
        
        maxx = delta;
        minx = -delta;
        maxy = delta;
        miny = -delta;
    }
    else {
        minx = game.get_settings().field_size.min.x;
        maxx = game.get_settings().field_size.max.x;
        miny = game.get_settings().field_size.min.y;
        maxy = game.get_settings().field_size.max.y;
    }
    
    Point move = Point(iter % width + minx, iter / width + miny);
    std::map<Point, Mark> myField;
    int it = 0;
    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            if (filled_cur[it] == false) myField[Point(x, y)] = Mark::None;
            else {
                if (crosses_cur[it] == false) myField[Point(x, y)] = Mark::Zero;
                else myField[Point(x, y)] = Mark::Cross;
            }
            it++;
        }
    }
    
    int goal = 0;
    
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && (dy == 0 || dy == -1)) continue;
            int max_forward = 0, max_backward = 0;
            Point p;

            p = move;
            do {
                p.x += dx;
                p.y += dy;
                ++max_forward;
            } while (max_forward <= winLength
                && p.x >= minx && p.x <= maxx && p.y >= miny && p.y <= maxy
                && myField[p] == value);

            p = move;
            do {
                p.x -= dx;
                p.y -= dy;
                ++max_backward;
            } while (max_backward <= winLength - max_forward + 1
                && p.x >= minx && p.x <= maxx && p.y >= miny && p.y <= maxy
                && myField[p] == value);

            if (max_forward + max_backward - 1 >= winLength) {
                return true;
            }
        }
    }
    
    return false;
}

bool MyPlayer::check_draw(const bool filled[], const int& size_of_field) const {
    for (int i = 0; i < size_of_field; ++i) {
        if (!filled[i]) {
            return false;
        }
    }
    return true;
}

int MyPlayer::evaluate(const GameView& game, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[], int it) {
    int score = 0;

    int width = winLength;
    int height = width;
    int minx, maxx;
    int miny, maxy;

    if (width != game.get_settings().field_size.get_width() || height != game.get_settings().field_size.get_height()) {
        
        maxx = delta;
        minx = -delta;
        maxy = delta;
        miny = -delta;
    }
    else {
        minx = game.get_settings().field_size.min.x;
        maxx = game.get_settings().field_size.max.x;
        miny = game.get_settings().field_size.min.y;
        maxy = game.get_settings().field_size.max.y;
    }

    auto checkLine = [&](int itStart, int dx, int dy, bool isCross) -> int {
        int count = 0;
        int lineScore = 0;
        for (int i = 0; i < winLength; ++i) {
            int iter = itStart + i * dx + width * dy;
            if (iter >= size_of_field || iter < 0) break;
            Point p = Point(iter % width + minx, iter / width + miny);
            if (!game.get_state().field->get_current_boundary().is_within(p)) break;
            if (filled_cur[iter] && crosses_cur[iter] == isCross) {
                count++;
                lineScore += 10; // Более весомый балл за каждую свою метку в линии
            }
            else if (filled_cur[iter] && crosses_cur[iter] != isCross) {
                return 0; // Линия блокирована, оценка 0
            }
        }
        return count == winLength ? 100 : lineScore; // Возврат 100 за победную линию
    };

    for (int i = 0; i < size_of_field; ++i) {
        if (filled_cur[i]) {
            if (crosses_cur[i]) {
                score += checkLine(i, 1, 0, true); // Горизонтально
                score += checkLine(i, 0, 1, true); // Вертикально
                score += checkLine(i, 1, 1, true); // Диагональ 
                score += checkLine(i, 1, -1, true); // Диагональ /
            }
            else {
                score -= checkLine(i, 1, 0, false); // Горизонтально
                score -= checkLine(i, 0, 1, false); // Вертикально
                score -= checkLine(i, 1, 1, false); // Диагональ 
                score -= checkLine(i, 1, -1, false); // Диагональ /
            }
        }
    }

    return score;
}

int MyPlayer::minimax(const GameView& game, bool is_maximizing, int depth, int iter, bool crosses_cur[], bool filled_cur[], const int& size_of_field, int alpha, int beta) {
    //int width = game.get_settings().field_size.get_width();
    //int height = game.get_settings().field_size.get_height();
    //int minx = game.get_settings().field_size.min.x;
    //int miny = game.get_settings().field_size.min.y;

    Mark currentMark = is_maximizing ? myMark : (myMark == Mark::Cross ? Mark::Zero : Mark::Cross);
    Mark otherMark = !is_maximizing ? myMark : (myMark == Mark::Cross ? Mark::Zero : Mark::Cross);
    if (is_win(game, otherMark, iter, size_of_field, crosses_cur, filled_cur)) {
        return is_maximizing ? -(size_of_field + 1) + depth : size_of_field + 1 - depth;
    }
    if (check_draw(filled_cur, size_of_field)) return 0;
    if (depth > 3) return evaluate(game, size_of_field, crosses_cur, filled_cur, iter);

    if (is_maximizing) {
        int maxEval = -10000;
        for (int i = 0; i < size_of_field; ++i) {
            if (!filled_cur[i]) {
                filled_cur[i] = true;
                crosses_cur[i] = (myMark == Mark::Cross);
                int eval = minimax(game, false, depth + 1, i, crosses_cur, filled_cur, size_of_field, alpha, beta);
                filled_cur[i] = false;
                crosses_cur[i] = false;
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) break;
            }
            myCount++;
        }
        return maxEval;
    }
    else {
        int minEval = 10000;
        for (int i = 0; i < size_of_field; ++i) {
            if (!filled_cur[i]) {
                filled_cur[i] = true;
                crosses_cur[i] = !(myMark == Mark::Cross);
                int eval = minimax(game, true, depth + 1, i, crosses_cur, filled_cur, size_of_field, alpha, beta);
                filled_cur[i] = false;
                crosses_cur[i] = false;
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) break;
            }
            myCount++;
        }
        return minEval;
    }
}

Point MyPlayer::find_best_move_in_square(const GameView& game, const Point& center, bool crosses_cur[], bool filled_cur[], const int& size_of_field, int& bestValue) {
    int width = game.get_settings().field_size.get_width();
    int height = game.get_settings().field_size.get_height();
    int minx = game.get_settings().field_size.min.x;
    int miny = game.get_settings().field_size.min.y;
    int maxx = game.get_settings().field_size.max.x;
    int maxy = game.get_settings().field_size.max.y;
    
    int x_start = center.x - delta, x_end = center.x + delta, y_start = center.y - delta, y_end = center.y + delta;
    if (center.x - delta < minx) {
        x_start = minx;
        x_end = x_start + delta * 2;
    }
    else if (center.x + delta > maxx) {
        x_end = maxx;
        x_start = x_end - delta * 2;
    }
    if (center.y - delta < miny) {
        y_start = miny;
        y_end = y_start + delta * 2;
    }
    else if (center.y + delta > maxy) {
        y_end = maxy;
        y_start = y_end - delta * 2;
    }

    Point bestMove(minx, miny);
    
    for (int i = 0; i < width * height; ++i) {
        if (filled_cur[i] == false) {
            bestMove = Point(i % width + minx, i / width + miny);
            break;
        }
    }
    
    for (int y = y_start; y <= y_end; ++y) {
        for (int x = x_start; x <= x_end; ++x) {
            int idx = (y + abs(miny)) * width + (x + abs(minx));
            if (!filled_cur[idx]) {
                filled_cur[idx] = true;
                crosses_cur[idx] = (myMark == Mark::Cross);
                int moveValue = minimax(game, false, 0, idx, crosses_cur, filled_cur, 9, -10000, 10000);
                filled_cur[idx] = false;
                crosses_cur[idx] = false;
                if (moveValue > bestValue) {
                    bestMove = Point(x, y);
                    bestValue = moveValue;
                }
            }
            myCount++;
        }
    }

    return bestMove;
}

Point MyPlayer::play(const GameView& game) {

    clock_t start = clock(); // для отчета

    int size_of_field = game.get_settings().field_size.get_width() * game.get_settings().field_size.get_height();

    bool* filled_cur = new bool[size_of_field];
    bool* crosses_cur = new bool[size_of_field];
    int width = game.get_settings().field_size.get_width();
    int height = game.get_settings().field_size.get_height();
    int minx = game.get_settings().field_size.min.x;
    int miny = game.get_settings().field_size.min.y;
    int maxx = game.get_settings().field_size.max.x;
    int maxy = game.get_settings().field_size.max.y;

    std::map<Point, Mark> occupied_points;
    int moves_count = 0;
    // Инициализация массивов filled_cur и crosses_cur
    int iter = 0;
    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            Mark value = game.get_state().field->get_value(Point(x, y));
            filled_cur[iter] = (value != Mark::None);
            crosses_cur[iter] = (value == Mark::Cross);
            
            if (filled_cur[iter]) {
                occupied_points[Point(x, y)] = value;
                moves_count++;
            }
            iter++;
        }
    }

    myCount = 0;

    int bestValueNew = -10000;
    int bestValue = -10000;
    Point bestMove(minx, miny);
    // Если поле пустое, ставим ход в центр
    if (moves_count == 0) {
        delete[] filled_cur;
        delete[] crosses_cur;
        return Point(width / 2 + minx, height / 2 + miny);
    }
    else if (moves_count == 1) {

        for (int y = miny; y <= maxy; ++y) {
            for (int x = minx; x <= maxx; ++x) {
                if (occupied_points[Point(x, y)] == Mark::None) continue;
                Point p(x, y);
                Point move = find_best_move_in_square(game, p, crosses_cur, filled_cur,  size_of_field, bestValueNew);
                if (bestValueNew > bestValue) {
                    bestMove = move;
                    bestValue = bestValueNew;
                }
                myCount++;
            }
        }
        
    }
    else {
        if (size_of_field <= 9) {
            for (int i = 0; i < size_of_field; ++i) {
                if (filled_cur[i] == false) {
                    filled_cur[i] = true;
                    crosses_cur[i] = myMark == Mark::Cross ? true : false;
                    int val = minimax(game, false, 0, i, crosses_cur, filled_cur, size_of_field, -10000, 10000);
                    if (val > bestValue) {
                        bestMove = Point(i % width + minx, i / width + miny);
                        bestValue = val;
                    }
                    filled_cur[i] = false;
                    crosses_cur[i] = false;
                }
                myCount++;
            }
        }
        else {
            // Поиск лучшего хода в пределах квадратов 3x3 вокруг занятых клеток

            iter = 0;
            for (int y = miny; y <= maxy; ++y) {
                for (int x = minx; x <= maxx; ++x) {
                    if (occupied_points[Point(x, y)] == Mark::None || occupied_points[Point(x, y)] != myMark) continue;
                    Point p(x, y);
                    Point move = find_best_move_in_square(game, p, crosses_cur, filled_cur, size_of_field, bestValueNew);
                    if (bestValueNew > bestValue) {
                        bestMove = move;
                        bestValue = bestValueNew;
                    }
                    myCount++;
                    iter++;
                }
            }
           
            
        }
    }
    delete[] filled_cur;
    delete[] crosses_cur;
    clock_t end = clock(); // для отчета
    std::cout << "Time to move: " << (double)(end - start) / CLOCKS_PER_SEC * 1000 << " ms, iterations: " << myCount << std::endl; // для отчета
    return bestMove;
}

Point RealPlayer::play(const GameView& game) {
    int x;
    int y;
ENTER:
    std::cout << "Make move like: x y:" << std::endl;
    std::cin >> x >> y;
    Point result = {
        .x = x,
        .y = y,
    };
    if ((game.get_state().field->get_value(result) != Mark::None) ||
        x < game.get_settings().field_size.min.x ||
        x > game.get_settings().field_size.max.x ||
        y < game.get_settings().field_size.min.y ||
        y > game.get_settings().field_size.max.y)
    {
        std::cout << "Wrong point" << std::endl;
        goto ENTER;
    }
    return result;
}

void BasicObserver::notify(const GameView&, const Event& event) {
    if (event.get_type() == MoveEvent::TYPE) {
        auto &data = get_data<MoveEvent>(event);
        _out << "Move:\tx = " <<  data.point.x 
            << ",\ty = " << data.point.y << ":\t";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == PlayerJoinedEvent::TYPE) {
        auto &data = get_data<PlayerJoinedEvent>(event);
        _out << "Player '" << data.name << "' joined as ";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == GameStartedEvent::TYPE) {
        _out << "Game started\n";
        return;
    }
    if (event.get_type() == WinEvent::TYPE) {
        auto &data = get_data<WinEvent>(event);
        _out << "Player playing ";
        _print_mark(data.winner_mark) << " has won\n";
        return;
    }
    if (event.get_type() == DrawEvent::TYPE) {
        auto &data = get_data<DrawEvent>(event);
        _out << "Draw happened, reason: '" << data.reason << "'\n";
        return;
    }
    if (event.get_type() == DisqualificationEvent::TYPE) {
        auto &data = get_data<DisqualificationEvent>(event);
        _out << "Player playing ";
        _print_mark(data.player_mark) << " was disqualified, reason: '" 
            << data.reason << "'\n";
        return;
    }
}

std::ostream& BasicObserver::_print_mark(Mark m) {
    if (m == Mark::Cross) return _out << "X";
    if (m == Mark::Zero) return _out << "O";
    return _out << "?";
}
