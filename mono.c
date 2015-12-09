// mono.c
// Scott Walker
// Mark Ayala
// Mallory Walsh
// This program simulates the game monopoly to discover the most frequently visited cells and
// highest profit generating cells

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define BSIZE 40
#define NUMPLAYERS 4

//#define DEBUG

enum
{
    BROWNG,
    LBLUEG,
    VIOLETG,
    ORANGEG,
    REDG,
    YELLOWG,
    GREENG,
    BLUEG
};

static const int GROUPS[8][3] = {{1, 3, 0}, {6, 8, 9}, {11, 13, 14}, {16, 18, 19}, 
                                 {21, 23, 24}, {26, 27, 29}, {31, 32, 34}, {37, 39, 0}};

static const int TRAINS[4] = {5, 15, 25, 35};

static const int UTILITIES[2] = {12, 28};

struct player
{
    char location; // p[0]
    char order; // p[1]
    int aggro; // not implemented
    char player; // p[2] 
    // properties p[3 - 7]
    long long money; // p[8-15]
};

struct location
{
    int location;
    int value;
    int rent;
    int owner;
    long long profits;
    long long visited;
    char group;
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
                board[i].group = 'W';
                break;
            case 3:
                board[i].value = 60;
                board[i].rent = 4;
                board[i].owner = -2;
                board[i].group = 'W';
                break;
            case 5:
            case 15:
            case 25:
            case 35:
                board[i].value = 200;
                board[i].rent = 25;
                board[i].owner = -2;
                board[i].group = 'T';
                break;
            case 6:
                board[i].value = 100;
                board[i].rent = 6;
                board[i].owner = -2;
                board[i].group = 'L';
                break;
            case 9:
            case 8:
                board[i].value = 120;
                board[i].rent = 8;
                board[i].owner = -2;
                board[i].group = 'L';
                break;
            case 11:
            case 13:
                board[i].value = 140;
                board[i].rent = 10;
                board[i].owner = -2;
                board[i].group = 'P';
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
                board[i].group = 'P';
                break;
            case 16:
            case 18:
                board[i].value = 180;
                board[i].rent = 14;
                board[i].owner = -2;
                board[i].group = 'O';
                break;
            case 19:
                board[i].value = 200;
                board[i].rent = 16;
                board[i].owner = -2;
                board[i].group = 'O';
                break;
            case 21:
            case 23:
                board[i].value = 220;
                board[i].rent = 18;
                board[i].owner = -2;
                board[i].group = 'R';
                break;
            case 24:
                board[i].value = 240;
                board[i].rent = 20;
                board[i].owner = -2;
                board[i].group = 'R';
                break;
            case 26:
            case 27:
                board[i].value = 260;
                board[i].rent = 22;
                board[i].owner = -2;
                board[i].group = 'Y';
                break;
            case 29:
                board[i].value = 280;
                board[i].rent = 24;
                board[i].owner = -2;
                board[i].group = 'Y';
                break;
            case 31:
            case 32:
                board[i].value = 300;
                board[i].rent = 26;
                board[i].owner = -2;
                board[i].group = 'G';
                break;
            case 34:
                board[i].value = 320;
                board[i].rent = 28;
                board[i].owner = -2;
                board[i].group = 'G';
                break;
            case 37:
                board[i].value = 350;
                board[i].rent = 35;
                board[i].owner = -2;
                board[i].group = 'B';
                break;
            case 39:
                board[i].rent = 50;
                board[i].value = 400;
                board[i].owner = -2;
                board[i].group = 'B';
                break;
                break;
            default:
                board[i].rent = 0;
                board[i].value = 0;
                board[i].owner = -1;
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

int chance(struct player * players, int n) 
{
    struct player * p = &players[n];
    int d[4];
    char max = 0;
    int ret = 0;
    int i;
    // 1 is property
    // 2 is utility pay 10x
    // 3 is railroad pay 2x
    // 4 is handle movement
    // 5 is railroad pay norm
    int res = rand() % 16;
    switch (res)
    {
        case 0:
            p->location = 0;
            p->money += 200;
            break;
        case 1:
            p->location = 24;
            ret = 1;
            // property
            break;
        case 2:
            p->location = 11;
            ret = 1;
            // property
            break;
        case 3:
            if ((p->location - 12) * (p->location - 12) >
                (p->location - 28) * (p->location - 28))
            {
                // farther from electric company
                p->location = 28;
            }
            else
            {
                // farther from water works
                p->location = 12;
            }
            // utility pay 10x
            ret = 2;
            break;
        case 4:
            d[0] = (p->location - 5) * (p->location - 5);
            d[1] = (p->location - 15) * (p->location - 15);
            d[2] = (p->location - 25) * (p->location - 25);
            d[3] = (p->location - 35) * (p->location - 35);
            for (i = 0; i < 3; i++)
            {
                if (d[i] < d[i+1])
                {
                    max = i + 1;
                }
            }
            ret = 3;
            // railroad pay 2x
            break;
        case 5:
            p->money += 50;
            break;
        case 6:
            // get out of jail
            // do nothing, houserules
            break;
        case 7:
            p->location -= 3;
            ret = 4;
            // handle movement
            break;
        case 8:
            p->location = 10;
            p->money -= 50;
            break;
        case 9:
            // general repairs
            break;
        case 10:
            p->money -= 15;
            break;
        case 11:
            p->location = 5;
            ret = 5;
            // railroad
            break;
        case 12:
            p->location = 39;
            ret = 1;
            // property
            break;
        case 13:
            p->money -= 200;
            // pay each player 50
            for (i == 0; i < 4; i++)
            {
                if (i != n)
                {
                    players[i].money += 50;
                }
            }
            break;
        case 14:
            p->money += 150;
            break;
        case 15:
            p->money += 100;
            break;
    }
    return ret;
}

void comm_chest(struct player * players, int n)
{
    struct player * p = &players[n];
    int i;
    int res = rand() % 16;
    switch (res)
    {
        case 1:
            p->location = 0;
            p->money += 200;
            break;
        case 2:
            p->money += 200;
            break;
        case 3:
            p->money -= 50;
            break;
        case 4:
            p->money += 50;
            break;
        case 5:
            // get out of jail free
            // do nothing, houserules
            break;
        case 6:
            p->money += 200;
            // take money from other players
            for (i = 0; i < 4; i++)
            {
                if (n != i)
                {
                    players[i].money -= 50;
                }
            }
            break;
        case 7:
            p->money += 100;
            break;
        case 8:
            p->money += 20;
            break;
        case 9:
            p->money += 40;
            // take money from other players
            for (i = 0; i < 4; i++)
            {
                if (n != i)
                {
                    players[i].money -= 10;
                }
            }
            break;
        case 10:
            p->money += 100;
            break;
        case 11:
            p->money -= 100;
            break;
        case 12:
            p->money -= 150;
            break;
        case 13:
            p->money += 25;
            break;
        case 14:
            // street repairs, per house and hotel
            break;
        case 15:
            p->money += 100;
            break;
    }
}

void utility(struct player * players, struct location * board, const int multiplier, int n) 
{
    struct player * p = &players[n];
    if (board[p->location].owner > -1)
    {
        long long amt;
        amt = roll() * 4 * multiplier;
        p->money -= amt;
        players[board[p->location].owner].money += amt;
        board[p->location].profits += amt;
    }
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        printf("Player %d bought location %d\n", n, p->location);
#endif
            board[p->location].owner = p->order;
            p->money -= board[p->location].value;
        }
    }

}

void railroad(struct player * players, struct location * board, 
              const int multiplier, const int n) 
{
    struct player * p = &players[n];
    if (board[p->location].owner > -1)
    {
        long long amt;
        amt = board[p->location].rent * multiplier;
        p->money -= amt;
        players[board[p->location].owner].money += amt;
        board[p->location].profits += amt;
    }
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        printf("Player %d bought location %d\n", n, p->location);
#endif
            board[p->location].owner = p->order;
            p->money -= board[p->location].value;
        }
    }
}

void property(struct player * players, struct location * board, const int n)
{
    struct player * p = &players[n];
    if (board[p->location].owner > -1)
    {
        // pay
        long long amt;
        amt = board[p->location].rent;
        p->money -= amt;
        players[board[p->location].owner].money += amt;
        board[p->location].profits += amt;
#ifdef DEBUG
        printf("Player %d payed player %d $%d\n", n, board[p->location].owner, amt);
#endif
    }
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        printf("Player %d bought location %d\n", p->order, p->location);
#endif
            board[p->location].owner = p->order;
            p->money -= board[p->location].value;
        }
    }
    else
    {
        return;
    }
}

void move(struct player * players, struct location * board, const int n)
{
    struct player * p = &players[n];
    int ret;
#ifdef DEBUG
    printf("Player %d moved from %d\n", n, p->location);
#endif
    if (p->location == 30)
    {
        // go to jail
        p->location = 10;
        board[p->location].visited++;
        p->money -= 50;
        return;
    }
    else
    {
        p->location += roll() + roll();
    }
    if (p->location > 39)
    {
#ifdef DEBUG
        printf("Player %d passed Go\n", n);
#endif
        p->location %= 40;
        p->money += 200;
    }
#ifdef DEBUG
    printf(" to %d\n", p->location);
#endif
    board[p->location].visited++;
    switch (p->location)
    {
        case 1:
        case 3:
        case 6:
        case 8:
        case 9:
        case 11:
        case 13:
        case 14:
        case 16:
        case 18:
        case 19:
        case 21:
        case 23:
        case 24:
        case 26:
        case 27:
        case 29:
        case 31:
        case 32:
        case 34:
        case 37:
        case 39:
            // properties
            property(players, board, n);
            break;
        case 2:
        case 17:
        case 33:
            comm_chest(players, n);
            break;
        case 7:
        case 22:
        case 36:
            // 1 is property
            // 2 is utility pay 10x
            // 3 is railroad pay 2x
            // 4 is handle movement
            // 5 is railroad pay norm
            // chance
            ret = chance(players, n);
            switch (ret)
            {
                case 1:
                    property(players, board, n);
                    break;
                case 2:
                    utility(players, board, 10, n);
                    break;
                case 3:
                    railroad(players, board, 2, n);
                    break;
                case 4:
                    move(players, board, n);
                    break;
                case 5:
                    railroad(players, board, 1, n);
                    break;
            }
            break;
        case 5:
        case 15:
        case 25:
        case 35:
            railroad(players, board, 1, n);
            // railroad
            break;
        case 12:
        case 28:
            utility(players, board, 1, n);
            // utility
            break;
        case 30:
            p->location = 10;
            p->money -= 50;
            // go to jail
            break;
        case 10:
            // jail
            break;
        case 4:
            p->money -= 200;
            // income tax
            break;
        case 38:
            p->money -= 75;
            // luxury tax
            break;
        case 0:
        case 20:
            // go and free parking, do nothing
            return;
        default:
            printf("ERROR: where are you??? player %d at %d\n", p->order, p->location);
            break;
    }
}

//int count_group(const uint64_t prop, const uint64_t g)
int count_group(struct location * b, const int group, const int n)
{
    //printf("counting for %d, group %d\n", n, group);
    int count = 0;
    int i;
    for (i = 0; i < 3; i++)
    {
        //printf("b[GROUPS[group][i]].owner %d, n %d\n", b[GROUPS[group][i]].owner, n);
        if (b[GROUPS[group][i]].owner == n)
        {
            count++;
        }
    }
    //printf("count is %d\n", count);
    return count;
/*
    uint64_t temp = 0;
    int count;
    temp = prop & g;
    while (temp)
    {
        if (temp % 2)
        {
            count++;
        }
        temp >>= 1;
    }
    return count;
*/
}

int count_owned(struct location * b, const int n)
{
    int i;
    int res = 0;
    for (i = 0; i < BSIZE; i++)
    {
        if (b[i].owner == n)
        {
            res++;
        }
    }
    return res;
}

void trade(struct player * players, struct location * b, const int n)
{
    int i, j, g;
    int seller;
    long long sellbid, buybid;
    int owned[4];
    owned[0] = count_owned(b, 0);
    owned[1] = count_owned(b, 1);
    owned[2] = count_owned(b, 2);
    owned[3] = count_owned(b, 3);
    int count[4] = {0, 0, 0, 0};
    // check how many of each group owned
    for (g = 1; g < 7; g++)
    {
        count[0] = count_group(b, g, 0);
        count[1] = count_group(b, g, 1);
        count[2] = count_group(b, g, 2);
        count[3] = count_group(b, g, 3);
        //printf("group %d: player 0 %d player 1 %d player 2 %d player 3 %d\n", g, count[0], count[1], count[2], count[3]);
        if (count[n] == 2 && players[n].money > 0)
        {
            sellbid = rand() % (players[n].money / 100 + 1) * b[players[n].location].value;
            buybid = rand() % (players[n].money / 100 + 1) * b[players[n].location].value;
            if (sellbid <= buybid)
            {
                //printf("buyer %d already has %d\n", n, count[n]);
                for (j = 0; j < 3; j++)
                {
                    //printf("trading group. %d onwer is %d\n", GROUPS[g][j], b[GROUPS[g][j]].owner);
                    if (b[GROUPS[g][j]].owner != n && b[GROUPS[g][j]].owner > -1)
                    {
                        seller = b[GROUPS[g][j]].owner;
                        if (owned[seller] > 2 && players[n].money > 0 && players[seller].money > 0)
                        {
                            printf("player %d sold %d to player %d\n", seller, GROUPS[g][j], n);
                            players[seller].money += sellbid;
                            b[GROUPS[g][j]].owner = n;
                            players[n].money -= sellbid;
                        }
                    }
                    b[GROUPS[g][j]].rent *= 2;
                }
            }
        }
    }
}

void results(struct player * p, struct location * b)
{
    long long totalvisits = 0;
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
        totalvisits += b[i].visited;
    }
    printf("====================\n\n\n");
    printf("Proportion of time spent in cells\n");
    for (i = 0; i < BSIZE; i++)
    {
        printf("%d: %lf\n", i, (double) b[i].visited / totalvisits);
    }
    printf("====================\n");
}

void remove_properties(struct location * b, const int n)
{
    int i;
    for (i = 0; i < BSIZE; i++)
    {
        if (b[i].owner == n)
        {
            b[i].owner = -2;
        }
    }
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
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    srand(time(NULL));
    struct location board[BSIZE];
    init_board(board);
    print_board_info(board);
    struct player players[NUMPLAYERS];
    init_players(players);

//    results(players, board);
    int itr = 10000;
    int done[4] = {1, 1, 1, 1};
    int i;
    //while (cont(players))
    while (itr)
    {
        itr--;
        for (i = 0; i < NUMPLAYERS; i++)
        {
            move(players, board, i);
            trade(players, board, i);
        }

    }

    gettimeofday(&t2, NULL);
    results(players, board);
    double exectime = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    printf("Exec Time %lf\n", exectime);

    return 0;
}
