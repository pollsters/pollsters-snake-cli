#ifndef SNAKE_H
#define SNAKE_H

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <map>
#include <deque>
#include <algorithm>
#include <limits>

using namespace std;
using std::chrono::system_clock;
using namespace std::this_thread;

bool waiting_for_choice;  // NEW global flag
char direction = 'r';
pair<int, int> poison_food = {-1, -1};
bool paused = false;

// leaderboard
vector<int> high_scores;

void input_handler() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    map<char, char> keymap = {{'d', 'r'}, {'a', 'l'}, {'w', 'u'}, {'s', 'd'}, {'q', 'q'}};

    while (true) {
        if (waiting_for_choice) {
            this_thread::sleep_for(chrono::milliseconds(100));  // let cin work
            continue;
        }

        char input = getchar();
        if (keymap.find(input) != keymap.end()) {
            direction = keymap[input];
        } else if (input == 'q') {
            exit(0);
        } else if (input == 'p') {
            paused = !paused;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void render_game(int size, deque<pair<int, int>> &snake, pair<int, int> food)
{
    system("clear");
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            if (i == food.first && j == food.second)
            {
                cout << "🍎";
            }
            else if (i == poison_food.first && j == poison_food.second)
            {
                cout << "💣";
            }
            else if (find(snake.begin(), snake.end(), make_pair(int(i), int(j))) != snake.end())
            {
                cout << "🐍";
            }
            else
            {
                cout << "⬜";
            }
        }
        cout << endl;
    }
}

pair<int, int> get_next_head(pair<int, int> current, char direction)
{
    pair<int, int> next;
    if (direction == 'r')
    {
        next = make_pair(current.first, (current.second + 1) % 10);
    }
    else if (direction == 'l')
    {
        next = make_pair(current.first, current.second == 0 ? 9 : current.second - 1);
    }
    else if (direction == 'd')
    {
        next = make_pair((current.first + 1) % 10, current.second);
    }
    else if (direction == 'u')
    {
        next = make_pair(current.first == 0 ? 9 : current.first - 1, current.second);
    }
    return next;
}

pair<int, int> get_random_empty_cell(int size, const deque<pair<int, int>> &snake, pair<int, int> other_food)
{
    pair<int, int> pos;
    do
    {
        pos = make_pair(rand() % size, rand() % size);
    } while (find(snake.begin(), snake.end(), pos) != snake.end() || pos == other_food);
    return pos;
}

void show_leaderboard()
{
    cout << "\n🏆 Top 10 High Scores 🏆\n";
    sort(high_scores.begin(), high_scores.end(), greater<int>());
    for (size_t i = 0; i < high_scores.size() && i < 10; i++)
    {
        cout << i + 1 << ". " << high_scores[i] << endl;
    }
}

bool game_play()
{
    int sleep_time = 500;
    const int poison_activation_speed = 300;
    system("clear");

    deque<pair<int, int>> snake;
    snake.push_back(make_pair(0, 0));

    pair<int, int> food;
    pair<int, int> poison;

    // food
    do
    {
        food = make_pair(rand() % 10, rand() % 10);
    } while (find(snake.begin(), snake.end(), food) != snake.end());

    // poison
    do
    {
        poison = make_pair(rand() % 10, rand() % 10);
    } while (find(snake.begin(), snake.end(), poison) != snake.end() || poison == food);

    poison_food = poison;

    for (pair<int, int> head = make_pair(0, 1);; head = get_next_head(head, direction))
    {
        cout << "\033[H";

        if (paused)
        {
            render_game(10, snake, food);
            cout << "\n⏸ Game Paused. Press 'p' to resume." << endl;
            sleep_for(chrono::milliseconds(200));
            continue;
        }

        if (sleep_time > poison_activation_speed)
        {
            poison_food = {-1, -1};
        }

        // collision
        if (find(snake.begin(), snake.end(), head) != snake.end())
        {
            system("clear");
            cout << "Game Over (Self Collision)" << endl;
            cout << "Your Score: " << snake.size() << endl;
            high_scores.push_back(snake.size());
            show_leaderboard();
            return false;
        }
        else if (head.first == poison_food.first && head.second == poison_food.second)
        {
            system("clear");
            cout << "Game Over (Poison Consumed)" << endl;
            cout << "Your Score: " << snake.size() << endl;
            high_scores.push_back(snake.size());
            show_leaderboard();
            return false;
        }
        else if (head.first == food.first && head.second == food.second)
        {
            snake.push_back(head);

            if (sleep_time > 100)
            {
                sleep_time -= 25;
            }

            food = get_random_empty_cell(10, snake, poison_food);

            if (sleep_time <= poison_activation_speed)
            {
                poison_food = get_random_empty_cell(10, snake, food);
            }
            else
            {
                poison_food = {-1, -1};
            }
        }
        else
        {
            snake.push_back(head);
            snake.pop_front();
        }

        render_game(10, snake, food);
        cout << "length of snake: " << snake.size() << endl;
        cout << "speed: " << 600 - sleep_time << " moves/sec" << endl;
        cout << "Press 'p' to pause/resume." << endl;

        sleep_for(chrono::milliseconds(sleep_time));
    }
}

// run loop
void run_game()
{
    srand(time(0));
    while (true)
    {
        bool playAgain = game_play();

        // pause input thread
        waiting_for_choice = true;

        cout << "\nPlay Again? (y/n): ";
        char choice;
        cin >> choice;

        // resume input thread
        waiting_for_choice = false;

        if (choice == 'n' || choice == 'N')
        {
            cout << "\nThanks for playing! 👋\n";
            break;
        }

        // reset state for new game
        direction = 'r';
        paused = false;
    }
}
#endif