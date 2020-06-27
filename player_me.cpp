#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

int player;
const int SIZE = 8;
const int search =6;

struct Point
{
    int x, y;
    Point() : Point(0, 0) {}
    Point(int x, int y) : x(x), y(y) {}
    bool operator==(const Point &rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Point &rhs) const
    {
        return !operator==(rhs);
    }
    Point operator+(const Point &rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point &rhs) const
    {
        return Point(x - rhs.x, y - rhs.y);
    }
};

std::array<std::array<int, SIZE>, SIZE> myboard;
std::vector<Point> my_next_valid_spots;

class OthelloBoard
{
public:
    enum SPOT_STATE
    {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{Point(-1, -1), Point(-1, 0), Point(-1, 1),
                                           Point(0, -1), /*{0, 0}, */ Point(0, 1),
                                           Point(1, -1), Point(1, 0), Point(1, 1)}};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;

private:
    int get_next_player(int player) const
    {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const
    {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const
    {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc)
    {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const
    {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const
    {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center)
    {
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                {
                    for (Point s : discs)
                    {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }

public:
    OthelloBoard()
    {
        reset();
    }
    void reset()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8 * 8 - 4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const
    {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p)
    {
        if (!is_spot_valid(p))
        {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0)
        {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0)
            {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs)
                    winner = EMPTY;
                else if (black_discs > white_discs)
                    winner = BLACK;
                else
                    winner = WHITE;
            }
        }
        return true;
    }
    int get_state_value()
    {
        int heu = 15;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                if (board[i][j] == player)
                {
                    heu+=2;
                    if ((i <= 1 || i >= SIZE - 2) && (j <= 1 || j >= SIZE - 2))
                    {
                        if ((i == 0 || i == SIZE - 1) && (j == 0 || j == SIZE - 1))
                        {
                            heu += 1000;
                            if ((i == 0 && board[i + 1][j] == player) )
                                heu += 60;
                            if((i == SIZE - 1 && board[i - 1][j] == player) )
                                heu+=60;
                            if((j == 0 && board[i][j + 1] == player)  )
                                heu+=60;
                            if((j == SIZE - 1 && board[i][j - 1] == player) )
                                heu+=60;

                            if(i==0&&j ==0&&board[i+1][j+1]== player)
                                heu+=60;
                            if(i==0&&j ==SIZE-1&&board[i+1][j-1]== player)
                                heu+=60;
                            if(i==SIZE-1&&j ==0&&board[i-1][j+1]== player)
                                heu+=60;
                            if(i==SIZE-1&&j ==SIZE-1&&board[i-1][j-1]== player)
                                heu+=60;
                        }
                        else
                        {
                            heu -= 60;
                        }
                    }
                    if ((i == 0 || i == SIZE - 1) || (j == 0 || j == SIZE - 1))
                    {
                        heu += 10;
                    }
                }
                if(board[i][j]==3-player)
                {
                    heu -=1;
                    if((i == 0 || i == SIZE - 1) && (j == 0 || j == SIZE - 1))
                    {
                        heu-=500;
                    }
                    if ((i == 0 || i == SIZE - 1) || (j == 0 || j == SIZE - 1))
                    {
                        heu -= 5;
                    }
                }
            }
        }
        
<<<<<<< HEAD
        heu-= next_valid_spots.size();
        if(next_valid_spots.size()== 0)
        {
            heu+=15;
        }
=======
        
        
        heu-= next_valid_spots.size();

>>>>>>> cc0169e1e7720f0e208f0ba71a6911a45696c609
        return heu;
    }
};

void read_board(std::ifstream &fin)
{
    fin >> player;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fin >> myboard[i][j];
        }
    }
}

void read_valid_spots(std::ifstream &fin)
{
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++)
    {
        fin >> x >> y;
        my_next_valid_spots.push_back({x, y});
    }
}


int alphabeta( OthelloBoard* pre ,int depth,int a, int b) 
{

    if (depth == 0 || pre->next_valid_spots.size()==0) 
        return pre->get_state_value();

    
    int n_valid_spots = pre->next_valid_spots.size();
    int k=0,temp;
    if (pre->cur_player == player){
        int value =-2147483647;
        
        for (int i = 0; i < n_valid_spots; i++)
        {
            OthelloBoard o ;
            o.board = pre->board;
            o.cur_player = pre->cur_player;
            o.next_valid_spots = pre->next_valid_spots;
            o.put_disc(pre->next_valid_spots[i]);
            
            temp = alphabeta(&o, depth-1,a,b);
            if(temp>value)
            {
                value = temp;
                k=i;
            }
            a = max(a, value);

            if (a >= b)
                break;
        }
        if(depth== search)
        {
            return k;
        }
        return value;
    }
    else{
        int value =2147483647;
       for (int i = 0; i < n_valid_spots; i++)
        {
            OthelloBoard o ;
            o.board = pre->board;
            o.cur_player = pre->cur_player;
            o.next_valid_spots = pre->next_valid_spots;
            o.put_disc(pre->next_valid_spots[i]);
            
            temp = alphabeta(&o, depth-1,a,b);
            value = min(value, temp);
            b = min(b, value);
            if (a >= b)
                break;
        }
        return value;
    }
}

void write_valid_spot(std::ofstream &fout)
{

    OthelloBoard o;
    o.board = myboard;
    o.cur_player= player;
    o.next_valid_spots  =my_next_valid_spots;
    int k = alphabeta( &o ,search,-2147483647,2147483647);
    Point p = my_next_valid_spots[k];

    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char **argv)
{
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
