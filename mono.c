// mono.c
// Scott Walker
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BSIZE 40
#define NUMPLAYERS 4

//#define DEBUG

struct player
{
    char location; // p[0][7:0]
    long long money; // p[1][63:0]
    char order; // p[0][15:8]
    int aggro; // not implemented
    char player; // p[0][23:16] 
};

struct location
{
    int location;
    int value;
    int rent;
    int owner;
    long long profits;
    long long visited;
};

int roll()
{
    return (rand() % 6) + 1;
}

void init_board(struct location * board)
{
    int i;
    for (i = 0; i < BSIZE; i++)
    {
        board[i].location = i;
        board[i].owner = -1;
        board[i].profits = 0;
        board[i].visited = 0;
        board[i].value = 0;
        switch (i)
        {
            case 1:
                board[i].value = 60;
                board[i].rent = 2;
                board[i].owner = -2;
                break;
            case 3:
                board[i].value = 60;
                board[i].rent = 4;
                board[i].owner = -2;
                break;
            case 5:
            case 15:
            case 25:
            case 35:
                board[i].value = 200;
                board[i].rent = 25;
                board[i].owner = -2;
                break;
            case 6:
                board[i].value = 100;
                board[i].rent = 6;
                board[i].owner = -2;
                break;
            case 9:
                board[i].value = 120;
                board[i].rent = 8;
                board[i].owner = -2;
                break;
            case 11:
            case 13:
                board[i].value = 140;
                board[i].rent = 10;
                board[i].owner = -2;
                break;
            case 12:
            case 28:
                board[i].value = 150;
                board[i].rent = -1;
                board[i].owner = -2;
                break;
            case 14:
                board[i].value = 160;
                board[i].rent = 12;
                board[i].owner = -2;
                break;
            case 16:
            case 18:
                board[i].value = 180;
                board[i].rent = 14;
                board[i].owner = -2;
                break;
            case 19:
                board[i].value = 200;
                board[i].rent = 16;
                board[i].owner = -2;
                break;
            case 21:
            case 23:
                board[i].value = 220;
                board[i].rent = 18;
                board[i].owner = -2;
                break;
            case 24:
                board[i].value = 240;
                board[i].rent = 20;
                board[i].owner = -2;
                break;
            case 26:
            case 27:
                board[i].value = 260;
                board[i].rent = 22;
                board[i].owner = -2;
                break;
            case 29:
                board[i].value = 280;
                board[i].rent = 24;
                board[i].owner = -2;
                break;
            case 31:
            case 32:
                board[i].value = 300;
                board[i].rent = 26;
                board[i].owner = -2;
                break;
            case 33:
                board[i].value = 320;
                board[i].rent = 28;
                board[i].owner = -2;
                break;
            case 37:
                board[i].value = 350;
                board[i].rent = 35;
                board[i].owner = -2;
                break;
            case 39:
                board[i].rent = 50;
                board[i].value = 400;
                board[i].owner = -2;
                break;
            default:
                board[i].rent = 0;
                board[i].value = 0;
                board[i].owner = -2;
                break;
        }
    }
}

void init_players(struct player * players)
{
    int i;
    for (i = 0; i < NUMPLAYERS; i++)
    {
        players[i].order = i;
        players[i].money = 1500;
        players[i].aggro = 40 + rand() % 60;
        players[i].location = 0;
    }
}

void property(struct location * board, int l)
{
    //board[l].
}

void utility() {}

void chance() {}

void comm_chest() {}

void tax() {}

void railroad() {}

void go() {}

void jail() {}

void freeparking() {}

void gotojail() {}

void move(struct player * players, struct location * board, int n)
{
    struct player * p = &players[n];
    board[p->location].visited++;
#ifdef DEBUG
    printf("Player %d moved from %d\n", n, p->location);
#endif
    if (p->money <= 0)
    {
        return;
    }
    if (p->location == 30)
    {
        // go to jail
        p->location = 10;
        return;
    }
    else
    {
        p->location += roll() + roll();
    }
#ifdef DEBUG
    printf(" to %d\n", p->location);
#endif
    if (p->location > 39)
    {
#ifdef DEBUG
        printf("Player %d passed Go\n", n);
#endif
        p->location %= 40;
        p->money += 200;
    }
    if (board[p->location].owner > 0)
    {
        // pay
        long long amt;
        if (board[p->location].rent == -1)
        {
            amt = 4 * roll();
            p->money -= 4 * roll();
            board[p->location].profits += amt;
            players[board[p->location].owner].money += amt;
        }
        else
        {
            amt = board[p->location].rent;
            p->money -= amt;
            players[board[p->location].owner].money += amt;
            board[p->location].profits += amt;
        }
#ifdef DEBUG
        printf("Player %d payed player %d $%d\n", n, board[p->location].owner, amt);
#endif
    }
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        printf("Player %d bought location %d\n", n, p->location);
#endif
            board[p->location].owner = n;
            p->money -= board[p->location].value;
        }
    }
    else
    {
        return;
    }
}

void results(struct player * p, struct location * b)
{
    printf("====================\n");
    int i;
    for (i = 0; i < NUMPLAYERS; i++)
    {
        printf("Player %d: money %ld location %d\n", i, p[i].money, p[i].location);
    }
    printf("--------------------\n");
    printf("Properties\n");
    for (i = 0; i < BSIZE; i++)
    {
        printf("%d: Profits %ld Owner %d Visited: %ld\n", i, b[i].profits, b[i].owner, b[i].visited);
    }
    printf("====================\n\n\n");
}

void print_board_info(struct location * b)
{
    printf("board\n");
    int i;
    for (i = 0; i < BSIZE; i++)
    {
        printf("%d: value %d rent %d owner %d profits %ld\n", i, b[i].value, b[i].rent, b[i].owner, b[i].profits);
    }
    printf("\n\n");
}

int cont(struct player * players)
{
    int left = 0;
    int i;
    for (i = 0 ; i < NUMPLAYERS; i++)
    {
        if (players[i].money > 0)
        {
            left++;
        }
    }
    if (left > 1)
    {
        return 1;
    }
    return 0;
}

int main()
{
    long long[2] p1;
    long long[2] p2;
    long long[2] p3;
    long long[2] p4;
    srand(time(NULL));
    struct location board[BSIZE];
    init_board(board);
    print_board_info(board);
    struct player players[NUMPLAYERS];
    init_players(players);

    results(players, board);
    int itr = 10000;
    int i;
    //while (cont(players))
    while (itr)
    {
        itr--;
        for (i = 0; i < NUMPLAYERS; i++)
        {
            if (players[i].money >= 0)
            {
                move(players, board, i);
            }
            else
            {
                players[i].order = -1;
            }
        }

    }

    results(players, board);

    return 0;
}
