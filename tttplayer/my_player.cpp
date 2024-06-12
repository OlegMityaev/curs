#include "my_player.h"
#include <cstdlib>
#include "windows.h"


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
    system("cls");
    return result;
}

bool operator<(const Point& p1, const Point& p2)
{
    int i1 = p1.x + p1.y * 100;
    int i2 = p2.x + p2.y * 100;
    return i1 < i2;
}

void MyPlayer::near_point(const Point& center, bool filled_cur[], bool crosses_cur[], bool filled_min[], bool crosses_min[]) {

    int x_start = center.x - delta, x_end = center.x + delta, y_start = center.y - delta, y_end = center.y + delta;
    if (center.x - delta < m_minx) {
        x_start = m_minx;
        x_end = x_start + delta * 2;
    }
    else if (center.x + delta > m_maxx) {
        x_end = m_maxx;
        x_start = x_end - delta * 2;
    }
    if (center.y - delta < m_miny) {
        y_start = m_miny;
        y_end = y_start + delta * 2;
    }
    else if (center.y + delta > m_maxy) {
        y_end = m_maxy;
        y_start = y_end - delta * 2;
    }

    int help_count = 0;

    int iter = 0;

    for (int y = y_start; y <= y_end; ++y) {
        for (int x = x_start; x <= x_end; ++x) {
            iter = x + abs(m_minx) + (y + abs(m_miny)) * m_width;
            filled_min[help_count] = filled_cur[iter];
            crosses_min[help_count] = crosses_cur[iter];
            help_count++;
        }
    }

}

inline void MyPlayer::init(const GameView& game) {
    
    m_width = game.get_settings().field_size.get_width();
    m_height = game.get_settings().field_size.get_height();
    m_minx = game.get_settings().field_size.min.x;
    m_miny = game.get_settings().field_size.min.y;
    m_maxx = game.get_settings().field_size.max.x;
    m_maxy = game.get_settings().field_size.max.y;
    delta = ((m_height * m_width) > 25) ? 2 : 1; // для ограничения квадратов
    winLength = delta * 2 + 1; // для определения победы в ограниченном квадрате
    size_min_field = winLength * winLength;
    antiWinMove = Point(m_minx - 10, m_miny - 10);
}

bool MyPlayer::is_win(const GameView& game, const Mark& value,
    int iter, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[]) {
    int width = winLength;
    int height = width;
    int minx, maxx;
    int miny, maxy;
    

    if (width != m_width || height != m_height) {
        
        maxx = delta;
        minx = -delta;
        maxy = delta;
        miny = -delta;
    }
    else {
        minx = m_minx;
        maxx = m_maxx;
        miny = m_miny;
        maxy = m_maxy;
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

            if (value != myMark) {
                if (max_forward + max_backward - 1 >= winLength - 1) {
                    antiWinMove = move;
                    return true;
                }
            }
            else {
                if (max_forward + max_backward - 1 >= winLength) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

int MyPlayer::evaluate(const GameView& game, Mark value, const int& size_of_field, const bool crosses_cur[], const bool filled_cur[]) {
    
    int myScore = 0;
    int otherScore = 0;

    int width = winLength;
    int height = width;
    int minx, maxx;
    int miny, maxy;

    if (width != m_width || height != m_height) {
        
        maxx = delta;
        minx = -delta;
        maxy = delta;
        miny = -delta;
    }
    else {
        minx = m_minx;
        maxx = m_maxx;
        miny = m_miny;
        maxy = m_maxy;
    }

    
    auto checkLine = [&](int dx, int dy, bool isCross) -> int {
        int count = 0;
        int lineScore = 0;
        for (int i = 0; i < winLength; ++i) {
            int iter = i + dx + width * dy;
            if (iter >= size_of_field || iter < 0) break;
            Point p = Point(iter % width + minx, iter / width + miny);
            if (!game.get_state().field->get_current_boundary().is_within(p)) break;
            if (filled_cur[iter] && (crosses_cur[iter] == isCross)) {
                count++;
                lineScore += 1; // Более весомый балл за каждую свою метку в линии
            }
            else if (filled_cur[iter] && (crosses_cur[iter] != isCross)) {
                return 0; // Линия блокирована, оценка 0
            }
            else if (!filled_cur[iter]) break;
            myCount++;
        }

        return count == winLength - 1 ? 100 : lineScore;

    };
    
    for (int itStart = 0; itStart < size_of_field; ++itStart) {
        if (filled_cur[itStart]) {
            if (myMark == Mark::Cross) {
                if (crosses_cur[itStart]) {
                    myScore += checkLine(1, 0, true); // Горизонтально
                    myScore += checkLine(0, 1, true); // Вертикально
                    myScore += checkLine(1, 1, true); // Диагональ "\"
                    myScore += checkLine(1, -1, true); // Диагональ /
                }
                else {
                    otherScore += checkLine(1, 0, false); // Горизонтально
                    otherScore += checkLine(0, 1, false); // Вертикально
                    otherScore += checkLine(1, 1, false); // Диагональ "\"
                    otherScore += checkLine(1, -1, false); // Диагональ /
                }
            }
            else if (myMark == Mark::Zero) {
                if (!crosses_cur[itStart]) {
                    myScore += checkLine(1, 0, false); // Горизонтально
                    myScore += checkLine(0, 1, false); // Вертикально
                    myScore += checkLine(1, 1, false); // Диагональ "\"
                    myScore += checkLine(1, -1, false); // Диагональ /
                }
                else {
                    otherScore += checkLine(1, 0, true); // Горизонтально
                    otherScore += checkLine(0, 1, true); // Вертикально
                    otherScore += checkLine(1, 1, true); // Диагональ "\"
                    otherScore += checkLine(1, -1, true); // Диагональ /
                }
            }
        }
    }
    
    return myScore - otherScore;
}


int MyPlayer::minimax(const GameView& game, bool is_maximizing, int depth, int iter, bool crosses_cur[], bool filled_cur[], const int& size_of_field, int alpha, int beta) {

    Mark currentMark = is_maximizing ? myMark : (myMark == Mark::Cross ? Mark::Zero : Mark::Cross);
    Mark otherMark = !is_maximizing ? myMark : (myMark == Mark::Cross ? Mark::Zero : Mark::Cross);
    if (is_win(game, otherMark, iter, size_of_field, crosses_cur, filled_cur)) {
        return is_maximizing ? -1000 : 1000;
    }
    else{
        bool is_draw = true;
        for (int i = 0; i < size_of_field; ++i) {
            if (filled_cur[i] == false) {
                is_draw = false;
                break;
            }
        }
        if (is_draw) return 0;
    }
    if (size_of_field >= 25) {
        if (depth > 0) {
            if (is_maximizing) {
                return -evaluate(game, otherMark, size_of_field, crosses_cur, filled_cur);
            }
            else {
                return evaluate(game, otherMark, size_of_field, crosses_cur, filled_cur);
            }
        }
    }

    if (is_maximizing) {
        int maxEval = -10000;
        for (int i = 0; i < size_of_field; ++i) {
            if (!filled_cur[i]) {
                filled_cur[i] = true;
                crosses_cur[i] = (myMark == Mark::Cross);
                int eval = minimax(game, false, depth + 1, i, crosses_cur, filled_cur, size_of_field, alpha, beta);
                filled_cur[i] = false;
                crosses_cur[i] = false;
                maxEval = (((maxEval) > (eval)) ? (maxEval) : (eval)) ;
                alpha = (((alpha) > (eval)) ? (alpha) : (eval));
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
                crosses_cur[i] = (myMark != Mark::Cross);
                int eval = minimax(game, true, depth + 1, i, crosses_cur, filled_cur, size_of_field, alpha, beta);
                filled_cur[i] = false;
                crosses_cur[i] = false;
                minEval = (((minEval) < (eval)) ? (minEval) : (eval));
                beta = (((beta) < (eval)) ? (beta) : (eval));
                if (beta <= alpha) break;
            }
            myCount++;
        }
        return minEval;
    }
}


Point MyPlayer::play(const GameView& game) {

    clock_t start = clock();

    init(game);

    int size_of_field = game.get_settings().field_size.get_width() * game.get_settings().field_size.get_height();

    bool* filled_cur = new bool[size_of_field];
    bool* crosses_cur = new bool[size_of_field];

    // Инициализация массивов filled_cur и crosses_cur
    int iter = 0;
    for (int y = m_miny; y <= m_maxy; ++y) {
        for (int x = m_minx; x <= m_maxx; ++x) {
            Mark value = game.get_state().field->get_value(Point(x, y));
            filled_cur[iter] = (value != Mark::None);
            crosses_cur[iter] = (value == Mark::Cross);
            iter++;
        }
    }
    clock_t end = 0;
    myCount = 0;
    iter = 0;
    int bestValueNew = -10000;
    int bestValue = -10000;
    Point bestMove((m_minx + m_maxx) / 2, (m_miny + m_maxy) / 2);
    if (size_of_field < 25) {
        for (int i = 0; i < size_of_field; ++i) {
            if (filled_cur[i] == false) {
                filled_cur[i] = true;
                crosses_cur[i] = myMark == Mark::Cross ? true : false;
                int val = minimax(game, false, 0, i, crosses_cur, filled_cur, size_of_field, -10000, 10000);
                
                if (val > bestValue) {
                    bestMove = Point(i % m_width + m_minx, i / m_width + m_miny);
                    bestValue = val;
                }
                filled_cur[i] = false;
                crosses_cur[i] = false;
            }
            myCount++;
        }
    }
    else if (myMark == Mark::Cross && moves_count / 2 == 0) {
        delete[] filled_cur;
        delete[] crosses_cur;
        lastMove = bestMove;
        moves_count += 2;
        return lastMove;
    }
    else {
        // Поиск лучшего хода в пределах квадратов 3x3 вокруг занятых клеток
        iter = 0;
        Point move = Point(m_minx, m_miny);
        for (int i = 0; i < size_of_field; ++i) {
            if (filled_cur[i] == false) {
                move = Point(i % m_width + m_minx, i / m_width + m_miny);
                break;
            }
        }
        for (int y = m_miny; y <= m_maxy; ++y) {
            for (int x = m_minx; x <= m_maxx; ++x) {
                if (filled_cur[iter] == true) {
                    bool* filled_min = new bool[size_min_field];
                    bool* crosses_min = new bool[size_min_field];
                    near_point(Point(x, y), filled_cur, crosses_cur, filled_min, crosses_min);
                    Point p(x, y);
                    int mid_x = x;
                    int mid_y = y;
                    if (x - delta < m_minx) mid_x = m_minx + delta;
                    if (x + delta > m_maxx) mid_x = m_maxx - delta;
                    if (y - delta < m_minx) mid_y = m_miny + delta;
                    if (y + delta > m_maxy) mid_y = m_maxy - delta;
                    
                    int temp_iter = 0;
                    for (int dy = -delta; dy <= delta; ++dy) {
                        for (int dx = -delta; dx <= delta; ++dx) {
                            if (filled_min[temp_iter] == false) {
                                filled_min[temp_iter] = true;
                                crosses_min[temp_iter] = (myMark == Mark::Cross);
                                int moveValue = minimax(game, false, 0, temp_iter, crosses_min, filled_min, size_min_field, -10000, 10000);
                                if (antiWinMove.x >= m_minx) {
                                    antiWinMove = Point(antiWinMove.x + mid_x, antiWinMove.y + mid_y);
                                    end = clock();
                                    std::cout << "Time for move: " << (double)(end - start) / CLOCKS_PER_SEC * 1000 << " ms, iterations: " << myCount << std::endl;
                                    lastMove = bestMove;
                                    moves_count += 2;
                                    system("cls");
                                    return antiWinMove;
                                }
                                filled_min[temp_iter] = false;
                                crosses_min[temp_iter] = false;
                                if (moveValue > bestValueNew) {
                                    move = Point(mid_x + dx, mid_y + dy);
                                    bestValueNew = moveValue;
                                }  
                            }
                            temp_iter++;
                            myCount++;
                        }
                    }
                    if (bestValueNew > bestValue) {
                        bestMove = move;
                        bestValue = bestValueNew;
                    }
                }
                myCount++;
                iter++;
            }
        }
    }
    delete[] filled_cur;
    delete[] crosses_cur;
    end = clock();
    
    std::cout << "Time for move: " << (double)(end - start) / CLOCKS_PER_SEC * 1000 << " ms, iterations: " << myCount << std::endl;
    lastMove = bestMove;
    moves_count += 2;
    system("cls");
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
    system("cls");
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
