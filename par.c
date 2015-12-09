// par.c
// Scott Walker
// Mallory Walsh
// Mark Ayala
// This simulates monopoly in parallel using MPI. It can make use of any number of processes that is a power of 2.
// The main purpose of this program is to determine the most visited cells and the highest profit generating cells.
// Note: this program will put debug output in the .dbg files

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>
#include <stdint.h>
#include <stddef.h>

#define BSIZE 40
#define NUMPLAYERS 4

//#define DEBUG

#define MEC(call) {int res; res = call; if (res != MPI_SICCESS) {fprintf(stderr, "Call %i \n", res); MPI_Abort(MPI_COMM_WORLD, res);}}

FILE ** output;
int globalrank;
int globalsize;

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

// These list combinations of properties that allow one to increase rent or develop
static const int GROUPS[8][3] = {{1, 3, 0}, {6, 8, 9}, {11, 13, 14}, {16, 18, 19}, 
                                 {21, 23, 24}, {26, 27, 29}, {31, 32, 34}, {37, 39, 0}};
static const int TRAINS[4] = {5, 15, 25, 35};
static const int UTILITIES[2] = {12, 28};

// this struct is used so players only need to send necessary data
typedef struct senddata
{
    long long money[4];
    int  pvalue;
    char plocation;
    char order;
    char trade;
} playerdata;

// each player's local data
struct player
{
    char location; // p[0]
    char order; // p[1]
    char properties[5];
    // properties p[3 - 7]
    long long money; // p[8-15]
};

// information for each square
typedef struct location
{
    int location;
    int value;
    int rent;
    int owner;
    long long profits;
    long long visited;
} boardcell;

int roll();
void init_board(struct location * board);
void init_players(struct player * players);
int chance(struct player * players, int n);
void comm_chest(struct player * players, int n);
void utility(struct player * players, struct location * board, const int multiplier, const int n, int * pvalue, char * plocation);
void railroad(struct player * players, struct location * board, const int multiplier, const int n, int * pvalue, char * plocation);
void property(struct player * players, struct location * board, const int n, int * pvalue, char * plocation);
void move(struct player * players, struct location * board, const int n, int * pvalue, char * plocation);
int count_group(struct location * b, const int group, const int n);
int count_owned(struct location * b, const int n);
void trade(struct player * players, struct location * b, const int n, int * pvalue, char * plocation, struct senddata * d);
void results(struct player * p, struct location * b);
void remove_properties(struct location * b, const int n);
void print_board_info(struct location * b);
int cont(struct player * players);
void send_info(struct senddata * send, struct player * players, struct location * board, 
               const int rank, MPI_Comm game, const MPI_Datatype MPI_MONO_DATA);
void gather_results(struct player * players, struct location * board, MPI_Comm * games, 
                    const int numcomms, const int rank);

// simulates a dice roll
int roll()
{
    return (rand() % 6) + 1;
}

// this initialises the monopoly board with property values and sets other data to zero
// -1 indicates a property cant be bought
// -2 indicates a property can be bought
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
            case 8:
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
            case 34:
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
                break;
            default:
                board[i].rent = 0;
                board[i].value = 0;
                board[i].owner = -1;
                break;
        }
    }
}

// this initialises the payer struct array
void init_players(struct player * players)
{
    int i;
    for (i = 0; i < NUMPLAYERS; i++)
    {
        players[i].order = i;
        players[i].money = 1500;
        players[i].location = 0;
    }
}

// simulation for the chance cards
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

// simulates the community chest cards
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

// handles players landing on a utility
void utility(struct player * players, struct location * board, const int multiplier, const int n, int * pvalue, char * plocation) 
{
    struct player * p = &players[n];
    // if property is owned pay up
    if (board[p->location].owner > -1)
    {
        long long amt;
        amt = roll() * 4 * multiplier;
        p->money -= amt;
        players[board[p->location].owner].money += amt;
        board[p->location].profits += amt;
    }
    // otherwise buy it
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        fprintf(output[globalrank], "Player %d bought location %d\n", n, p->location);
#endif
            *pvalue = board[p->location].value;
            *plocation = p->location;
        }
    }

}

// handles players landing on a railroad
void railroad(struct player * players, struct location * board, const int multiplier, const int n, int * pvalue, char * plocation)
{
    struct player * p = &players[n];
    // if property is owned pay up
    if (board[p->location].owner > -1)
    {
        long long amt;
        amt = board[p->location].rent * multiplier;
        p->money -= amt;
        players[board[p->location].owner].money += amt;
        board[p->location].profits += amt;
    }
    // otherwise buy it
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        fprintf(output[globalrank], "Player %d bought location %d\n", n, p->location);
#endif
            *pvalue = board[p->location].value;
            *plocation = p->location;
        }
    }
}

// handles players landing on a property
void property(struct player * players, struct location * board, const int n, int * pvalue, char * plocation)
{
    struct player * p = &players[n];
    // if property is owned pay up
    if (board[p->location].owner > -1)
    {
        // pay
        long long amt;
        amt = board[p->location].rent;
        p->money -= amt;
        players[board[p->location].owner].money += amt;
        board[p->location].profits += amt;
#ifdef DEBUG
        fprintf(output[globalrank], "Player %d payed player %d $%d\n", n, board[p->location].owner, amt);
        //fprintf(output[globalrank], "player location is %d\n", p->location);
#endif
    }
    // otherwise buy it
    else if (board[p->location].owner == -2)
    {
        if (p->money > board[p->location].value)
        {
#ifdef DEBUG
        fprintf(output[globalrank], "Player %d bought location %d\n", p->order, p->location);
#endif
            *pvalue = board[p->location].value;
            *plocation = p->location;
        }
    }
}

// this function handles players moving around the board
void move(struct player * players, struct location * board, const int n, int * pvalue, char * plocation)
{
    struct player * p = &players[n];
    int ret;
#ifdef DEBUG
    fprintf(output[globalrank], "Player %d moved from %d\n", n, p->location);
#endif
    // check to see if on the "go to jail" cell
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
        // advance the player
        p->location += roll() + roll();
    }
    // check to see if player passed go
    if (p->location > 39)
    {
#ifdef DEBUG
        fprintf(output[globalrank], "Player %d passed Go\n", n);
#endif
        p->location %= 40;
        p->money += 200;
    }
#ifdef DEBUG
    fprintf(output[globalrank], " to %d\n", p->location);
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
            property(players, board, n, pvalue, plocation);
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
                    property(players, board, n, pvalue, plocation);
                    break;
                case 2:
                    utility(players, board, 10, n, pvalue, plocation);
                    break;
                case 3:
                    railroad(players, board, 2, n, pvalue, plocation);
                    break;
                case 4:
                    move(players, board, n, pvalue, plocation);
                    break;
                case 5:
                    railroad(players, board, 1, n, pvalue, plocation);
                    break;
            }
            break;
        case 5:
        case 15:
        case 25:
        case 35:
            railroad(players, board, 1, n, pvalue, plocation);
            // railroad
            break;
        case 12:
        case 28:
            utility(players, board, 1, n, pvalue, plocation);
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
            fprintf(stderr, "ERROR: where are you??? player %d at %d\n", p->order, p->location);
            break;
    }
}

// counts how many properties a player owns in a property group (color)
int count_group(struct location * b, const int group, const int n)
{
    int count = 0;
    int i;
    for (i = 0; i < 3; i++)
    {
        if (b[GROUPS[group][i]].owner == n)
        {
            count++;
        }
    }
    return count;
}

// counts how many properties a player owns overall
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

// experimental code: not used
void trade(struct player * players, struct location * b, const int n, int * pvalue, char * plocation, struct senddata * d)
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
        // if player has at least two properties and enough money then attempt a trade
        if (count[n] == 2 && players[n].money > 0)
        {
            // player will buy if seller asks low enough
            sellbid = rand() % (players[n].money / 100 + 1) * b[players[n].location].value;
            buybid = rand() % (players[n].money / 100 + 1) * b[players[n].location].value;
            if (sellbid <= buybid)
            {
#ifdef DEBUG
                fprintf(output[globalrank], "buyer %d already has %d\n", n, count[n]);
#endif
                // find the property in the group not yet owned
                for (j = 0; j < 3; j++)
                {
#ifdef DEBUG
                    fprintf(output[globalrank], "trading group. %d onwer is %d\n", GROUPS[g][j], b[GROUPS[g][j]].owner);
#endif
                    // buy it
                    if (b[GROUPS[g][j]].owner != n && b[GROUPS[g][j]].owner > -1)
                    {
                        seller = b[GROUPS[g][j]].owner;
                        if (owned[seller] > 2 && players[n].money > 0 && players[seller].money > 0)
                        {
#ifdef DEBUG
                            fprintf(output[globalrank], "player %d sold %d to player %d\n", seller, GROUPS[g][j], n);
#endif
                            players[seller].money += sellbid;
                            b[GROUPS[g][j]].owner = n;
                            *plocation = GROUPS[g][j];
                            *pvalue = sellbid;
                            d->money[seller] += sellbid;
                            d->trade = seller;
                            players[n].money -= sellbid;
                        }
                    }
                    b[GROUPS[g][j]].rent *= 2;
                }
            }
        }
    }
}

// print the results of the simulation
void results(struct player * p, struct location * b)
{
    long long totalvisits = 0;
    printf("====================\n");
    int i;
    // money for each player (in game 0)
    for (i = 0; i < NUMPLAYERS; i++)
    {
        printf("Player %d: money %ld location %d\n", i, p[i].money, p[i].location);
    }
    printf("--------------------\n");
    printf("Properties\n");
    // board data
    for (i = 0; i < BSIZE; i++)
    {
        printf("%d: Profits %ld Owner %d Visited: %ld\n", i, b[i].profits, b[i].owner, b[i].visited);
        totalvisits += b[i].visited;
    }
    printf("====================\n\n\n");
    printf("Proportion of time spent in cell\n");
    // proportion of visits in each cell
    for (i = 0; i < BSIZE; i++)
    {
        printf("%d: %lf\n", i, (double) b[i].visited / totalvisits);
    }
    printf("====================\n");
}

// removes player ownership when they loose
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

// print out the board ownership, visits, rent, and profits
void print_board_info(struct location * b)
{
    fprintf(output[globalrank], "board\n");
    int i;
    for (i = 0; i < BSIZE; i++)
    {
        fprintf(output[globalrank], "%d: value %d rent %d owner %d profits %ld, visits %ld\n", 
                i, b[i].value, b[i].rent, b[i].owner, b[i].profits, b[i].visited);
    }
    fprintf(output[globalrank], "\n\n");
}

// gather the results from every thread to find the total visits and profits
void gather_results(struct player * players, struct location * board, MPI_Comm * games, 
                    const int numcomms, const int rank)
{
    int i;
    long long visits[40];
    long long profits[40];
    long long * allvisits = (long long *) malloc(40 * 4 * numcomms * sizeof(long long));
    long long * allprofits = (long long *) malloc(40 * 4 * numcomms * sizeof(long long));
#ifdef DEBUG
    fprintf(output[globalrank], "tag 5 from rank %d\n", rank);
#endif
    // send only the data needed
    for (i = 0; i < 40; i++)
    {
        visits[i] = board[i].visited;    
        profits[i] = board[i].profits;
    }
#ifdef DEBUG
    fprintf(output[globalrank], "tag 6 from rank (barrier 2)%d\n", rank);
#endif
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(visits, 40, MPI_LONG_LONG, allvisits, 40, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Gather(profits, 40, MPI_LONG_LONG, allprofits, 40, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
#ifdef DEBUG
    fprintf(output[globalrank], "tag 7 from rank %d, globalsize %d, numcomms %d\n", rank, globalsize, numcomms);
#endif
    if (globalrank == 0)
    {
        for (i = 0; i < 40 * globalsize * numcomms; i++)
        {
#ifdef DEBUG
            fprintf(output[globalrank], "GATHER allvisits[i] is %ld\n", allvisits[i]);
#endif
            board[i % 40].visited += allvisits[i];
            board[i % 40].profits += allprofits[i];
        }
    }
#ifdef DEBUG
    fprintf(output[globalrank], "tag 8 from rank %d\n", rank);
#endif
}

// this is how players keep in touch with each other at the end of their turn
// sends only the necessary player data
void send_info(struct senddata * send, struct player * players, struct location * board, 
               const int rank, MPI_Comm game, const MPI_Datatype MPI_MONO_DATA)
{
    struct senddata p[4];
#ifdef DEBUG
    if (rank > 3) fprintf(stderr, "INVALID RANK\n");
    fprintf(output[globalrank], "tag 1 from rank %d (barrier 1)\n", rank);
    fprintf(output[globalrank], "d: send data for player %d, plocation %d, pvalue %d\n", 
            send->order, send->plocation, send->pvalue);
#endif
    MPI_Barrier(game);
    MPI_Allgather(send, 1, MPI_MONO_DATA, p, 1, MPI_MONO_DATA, game);
#ifdef DEBUG
    fprintf(output[globalrank], "tag 2 from rank %d\n", rank);
#endif
    int i, j;
    int buyer[4] = {0, 0, 0, 0};
    int escrow[4];
    int numplayers = (globalsize > 3 ? 4 : 2);
    // check to see if other players are trying to buy the property.
    for (i = 0; i < numplayers; i++)
    {
#ifdef DEBUG
        fprintf(output[globalrank], "gathered for player %d, plocation %d, pvalue %d\n", 
                p[i].order, p[i].plocation, p[i].pvalue);
#endif
        if (p[i].plocation < 40)
        {
            escrow[i] = p[i].plocation;
        }
#ifdef DEBUG
        else
        {
            fprintf(stderr, "invalid purchase location %d for player %d\n", p[i].plocation, i);
        }
#endif
    }
    for (i = 0; i < numplayers; i++)
    {
        buyer[i] = p[i].order;
        for (j = 0; j < numplayers; j++)
        {
            players[i].money += p[j].money[i];
            if (p[i].plocation == escrow[j] && i != j)
            {
                if (p[j].order < buyer[i])
                {
                    buyer[i] = p[j].order;
                }
            }
        }
#ifdef DEBUG
        fprintf(output[globalrank], "player %d bought %d\n", rank, send->plocation); 
#endif
        // once lowest ranked buyer is found buy the property
        players[buyer[i]].money -= p[buyer[i]].pvalue;
        board[p[i].plocation].owner = p[buyer[i]].order;
    }
#ifdef DEBUG
    fprintf(output[globalrank], "tag 3 from rank %d\n", rank);
#endif
}

// TODO: add player payments for chance/chest
int main(int argc, char ** argv)
{
    struct timeval t1, t2;
    MPI_Init(&argc, &argv);
    gettimeofday(&t1, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &globalrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    globalsize = size;
    srand(time(NULL) + globalrank);
    struct location board[BSIZE];
    struct player players[NUMPLAYERS];
    int itr = 10000;
    long long bills[4]; // how much you owe each player at end of round
    init_players(players);
    init_board(board);
    char plocation;
    int pvalue;
    int numcomms = 1;
    MPI_Group world_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    playerdata d;
    d.money[0] = 0;
    d.money[1] = 0;
    d.money[2] = 0;
    d.money[3] = 0;
    output = (FILE **) malloc(size * sizeof(FILE *));

    // if 1 process created just run sequentially
    if (size == 1)
    {
        int done[4] = {1, 1, 1, 1};
        while (itr)
        {
            itr--;
            int i;
            for (i = 0; i < NUMPLAYERS; i++)
            {
                plocation = 0;
                pvalue = 0;
                if (players[i].money > 0)
                {
                    move(players, board, i, &pvalue, &plocation);
                    if (plocation)
                    {
                        board[plocation].owner = i;
                        players[i].money -= pvalue;
                    }
                }
                else
                {
                    players[i].order = -1;
                    if (done[i])
                    {
                        remove_properties(board, i);
                        done[i] = 0;
                    }
                }
            }
        }
        gettimeofday(&t2, NULL);
        results(players, board);
        double exectime = (t2.tv_sec - t1.tv_sec) * 1000000 + ((t2.tv_usec - t1.tv_usec));
        printf("Exec Time %lf\n", exectime);
        return 0;
    }

    // create a communicator for each monopoly game (for n > 4)
    MPI_Group * gamesel;
    MPI_Comm * games;
    int ranksel[4];
    if (size > 4)
    {
        numcomms = size / 4;
        games = (MPI_Comm *) malloc(numcomms * sizeof(MPI_Comm));
        gamesel = (MPI_Group *) malloc(numcomms * sizeof(MPI_Group));
        int i;
        for (i = 0; i < numcomms; i++)
        {
            ranksel[0] = 4 * i;
            ranksel[1] = 4 * i + 1;
            ranksel[2] = 4 * i + 2;
            ranksel[3] = 4 * i + 3;
            MPI_Group_incl(world_group, 4, ranksel, &gamesel[i]);
            MPI_Comm_create(MPI_COMM_WORLD, gamesel[i], &games[i]);
        }
    }
    else
    {
        // n < 4 so use MPI_COMM_WORLD
        games = (MPI_Comm *) malloc(1 * sizeof(MPI_Comm));
        games[0] = MPI_COMM_WORLD;
        numcomms = 1;
    }

    // create an MPI type so that we can use our player data struct in MPI communication calls
    const int nitems = 5;
    int blocklengths[5] = {4, 1, 1, 1, 1};
    MPI_Datatype types[5] = {MPI_LONG_LONG, MPI_INT, MPI_CHAR, MPI_CHAR, MPI_CHAR};
    MPI_Datatype MPI_MONO_DATA;
    MPI_Aint offsets[5];
    offsets[0] = offsetof(playerdata, money);
    offsets[1] = offsetof(playerdata, pvalue);
    offsets[2] = offsetof(playerdata, plocation);
    offsets[3] = offsetof(playerdata, order);
    offsets[4] = offsetof(playerdata, trade);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_MONO_DATA);
    MPI_Type_commit(&MPI_MONO_DATA);

    MPI_Comm_rank(games[globalrank / 4], &rank);

#ifdef DEBUG
    char fname[10];
    snprintf(fname, 10, "mon%d.dbg", globalrank);
    output[globalrank] = fopen(fname, "w");
    fprintf(output[globalrank], "MAIN begin loop\n");
    print_board_info(board);
#endif
    // run the game for 40000 turns (10000 per player)
    while (itr > 0)
    {
        itr--;
        pvalue = 0;
        plocation = 0;
        d.trade = 0;
        d.order = rank;
#ifdef DEBUG
        fprintf(output[globalrank], "MAIN tag 1 rank %d\n", rank);
#endif
        move(players, board, rank, &pvalue, &plocation);
        d.pvalue = pvalue;
        d.plocation = plocation;
#ifdef DEBUG
        fprintf(output[globalrank], "using comm %d\n", globalrank / 4);
        if (games[globalrank / 4] != MPI_COMM_WORLD)
        {
            fprintf(output[globalrank], "COMM ERROR\n");
        }
#endif
        send_info(&d, players, board, rank, games[globalrank / 4], MPI_MONO_DATA);
#ifdef DEBUG
        fprintf(output[globalrank], "MAIN tag 3 rank %d\n", rank);
        print_board_info(board);
#endif
    }
    
#ifdef DEBUG
    fprintf(output[globalrank], "MAIN last tag rank %d\n", rank);
#endif
    // get results from each process
    gather_results(players, board, games, numcomms, globalrank);
    gettimeofday(&t2, NULL);
    if (globalrank == 0)
    {
        results(players, board);
    }


#ifdef DEBUG
    fclose(output[globalrank]);
#endif
    double exectime = (t2.tv_sec - t1.tv_sec) * 1000000 + ((t2.tv_usec - t1.tv_usec));
    if (globalrank == 0)
    {
        printf("Exec Time %lf\n", exectime);
    }
    MPI_Finalize();

    return 0;
}
