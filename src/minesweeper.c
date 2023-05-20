/**
 * @file minesweeper.c
 *
 * @author Ivano Izzo
 */
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "../include/minesweeper.h"

int height, width, mines, goal, moves = 0, uncovered = 0;
bool game_over;
cell **board;

int select_diff();
void print_diff_menu();
int discover(int, int);
bool discoverable(int, int);
bool is_game_over(cell *);
void print_results(int);

/**
 * @brief initial setup
 */
void init()
{
    switch (select_diff())
    {
    case 1:
        height = 9;
        width = 9;
        mines = 10;
        break;
    case 2:
        height = 16;
        width = 16;
        mines = 40;
        break;
    case 3:
        height = 16;
        width = 30;
        mines = 99;
        break;
    default:
        return;
    }
    goal = height * width - mines;

    board = (cell **)malloc(height * sizeof(cell));
    if (!board)
        exit(EXIT_FAILURE);
    for (int i = 0; i < height; i++)
    {
        board[i] = (cell *)malloc(width * sizeof(cell));
        if (!board[i])
            exit(EXIT_FAILURE);
    }

    game_loop();

    for (int i = 0; i < height; i++)
        free(board[i]);
    free(board);
}

/**
 * @brief asks the user to select the game difficulty
 */
int select_diff()
{
    int ch;

    print_diff_menu();
    while ((ch = getch()))
    {
        if (ch == KEY_MOUSE)
        {
            MEVENT event;
            if (getmouse(&event) == OK && (event.bstate & BUTTON1_CLICKED))
                switch (event.y)
                {
                case 3:
                    return 1;
                case 5:
                    return 2;
                case 7:
                    return 3;
                default:
                    break;
                }
        }
        else if (ch == 'q')
            return 0;
        else if (ch == KEY_RESIZE)
            print_diff_menu();
    }
    return -1;
}

/**
 * @brief prints the difficulties to choose from
 */
void print_diff_menu()
{
    clear();
    refresh();

    printf("\n\t▆■■■■■■ DIFFICULTY ■■■■■■");
    printf("▆\n\b█\r\t█");
    printf("\n\b█" H_GRN " ■■■■■■ BEGINNER ■■■■■■ " RESET);
    printf("█\n\b█\r\t█");
    printf("\n\b█" H_YEL " ■■■■ INTERMEDIATE ■■■■ " RESET);
    printf("█\n\b█\r\t█");
    printf("\n\b█" H_RED " ■■■■■■■ EXPERT ■■■■■■■ " RESET);
    printf("█\n\b█\r\t█\n\b█");
    for (int i = 23; i >= 0; i--)
        printf("■");
    printf("█\n\r");

    refresh();
}

/**
 * @brief the game loop - it ends when the game is over or the user quits
 */
void game_loop()
{
    int ch, row, col;

    place_mines();
    print_board();
    while (!game_over)
    {
        if ((ch = getch()) == KEY_MOUSE)
        {
            MEVENT event;
            if (getmouse(&event) == OK)
            {
                row = event.y - 1;
                col = (event.x);

                if (col % 2 == 0)
                    col /= 2;
                else
                    continue;

                if (row >= 0 && row < height && col >= 0 && col < width)
                {
                    if (event.bstate & BUTTON1_CLICKED)
                        play(row, col);
                    else if (event.bstate & BUTTON3_CLICKED)
                        flag(row, col);
                }
            }
        }
        else if (ch == 'q')
            return;
        else if (ch == KEY_RESIZE)
            print_board();
    }
    print_board();
    while ((ch = getch()) != 'q')
        if (ch == KEY_RESIZE)
            print_board();
}

/**
 * @brief places the mines randomly on the board
 */
void place_mines()
{
    int mine_row, mine_col, mines_left = mines;
    srand(time(NULL));

    while (mines_left)
    {
        mine_row = rand() % height;
        mine_col = rand() % width;
        cell *pos = &(board[mine_row][mine_col]);

        if (!pos->is_mine)
        {
            pos->is_mine = true;
            signal_mine(mine_row, mine_col);
            mines_left--;
        }
    }
}

/**
 * @brief updates the number of surrounding mines of cells touching this mine
 *
 * @param row the mine's row
 * @param col the mine's column
 */
void signal_mine(int row, int col)
{
    for (int i = row - 1; i <= row + 1; i++)
        if (i >= 0 && i < height)
            for (int j = col - 1; j <= col + 1; j++)
                if (j >= 0 && j < width)
                {
                    cell *pos = &(board[i][j]);
                    if (!pos->is_mine)
                        (pos->surrounding_mines)++;
                }
}

/**
 * @brief prints the current state of the board
 */
void print_board()
{
    clear();
    refresh();

    // stats top
    printf(H_GRN);
    int len = printf("■ %d/%d", uncovered, goal);
    for (int i = 0; i < width * 2 - len - 3; i++)
        printf(" ");
    printf(B_H_RED "*" H_RED " %2d\n\r" RESET, mines);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            cell *pos = &(board[i][j]);
            int num_mines = pos->surrounding_mines;

            if (pos->is_mine && pos->discovered && game_over)
                printf(BG_RED B_WHT "\b * " RESET);
            else if (pos->discovered || game_over)
                if (pos->is_mine)
                    printf(B_RED "* " RESET);
                else if (num_mines)
                {
                    switch (num_mines)
                    {
                    case 1:
                        printf(H_BLU);
                        break;
                    case 2:
                        printf(H_GRN);
                        break;
                    case 3:
                        printf(H_YEL);
                        break;
                    case 4:
                        printf(H_MAG);
                        break;
                    default:
                        printf(H_CYN);
                        break;
                    }
                    printf("%d " RESET, num_mines);
                }
                else
                    printf(". ");
            else
            {
                if (pos->is_flagged)
                    printf(RED);
                printf("■ " RESET);
            }
        }
        if (game_over)
            print_results(i);
        printf("\n\r");
    }

    // stats bottom
    printf(B_H_CYN "# " H_CYN "%d\n\r" RESET, moves);

    refresh();
}

/**
 * @brief discover a cell and checks if more can be discovered
 *
 * @param row the row of the starting cell
 * @param col the column of the starting cell
 * @return 1 if the game is over;
 *         0 if the game is not over
 */
int play(int row, int col)
{
    cell *pos = &(board[row][col]);

    if (pos->is_flagged)
        return 0;
    if (pos->discovered)
    {
        if (!pos->surrounding_mines)
            return 0;
        if (discoverable(row, col))
        {
            moves++;
            for (int i = row - 1; i <= row + 1; i++)
                if (i >= 0 && i < height)
                    for (int j = col - 1; j <= col + 1; j++)
                        if (j >= 0 && j < width && !board[i][j].is_flagged)
                            discover(i, j);
        }
        else
            return 0;
    }
    else
    {
        moves++;
        pos->discovered = true;
        uncovered++;
        if (is_game_over(pos))
            return 1;
        discover(row, col);
    }
    print_board();
    return 0;
}

/**
 * @brief places or removes a flag on an undiscovered cell
 *
 * @param row the row of the starting cell
 * @param col the column of the starting cell
 */
void flag(int row, int col)
{
    if (!board[row][col].discovered)
    {
        bool *flag = &(board[row][col].is_flagged);
        (*flag = !*flag) ? mines-- : mines++;
        print_board();
    }
}

/**
 * @brief discovers all the possible cells from a starting cell
 *
 * @param row the row of the starting cell
 * @param col the column of the starting cell
 * @return 1 if a mine is reached; 0 otherwise
 */
int discover(int row, int col)
{
    cell *this = &(board[row][col]);

    if (!this->discovered)
    {
        this->discovered = true;
        uncovered++;
        if (is_game_over(this))
            return 1;
    }
    for (int i = row - 1; i <= row + 1; i++)
        if (i >= 0 && i < height)
            for (int j = col - 1; j <= col + 1; j++)
                if (j >= 0 && j < width)
                {
                    cell *neighbor = &(board[i][j]);
                    if (!neighbor->discovered)
                    {
                        if (!neighbor->surrounding_mines && !neighbor->is_mine)
                            discover(i, j);
                        else if (!this->surrounding_mines)
                        {
                            neighbor->discovered = true;
                            uncovered++;
                            if (is_game_over(neighbor))
                                return 1;
                        }
                    }
                }
    return 0;
}

/**
 * @brief checks if the number of surrounding flags matches the surrounding mines
 *
 * @param row the row of the starting cell
 * @param col the column of the starting cell
 */
bool discoverable(int row, int col)
{
    int nmines = 0;
    bool clear = true;

    for (int i = row - 1; i <= row + 1; i++)
        if (i >= 0 && i < height)
            for (int j = col - 1; j <= col + 1; j++)
                if (j >= 0 && j < width)
                {
                    cell *neighbor = &(board[i][j]);
                    if (neighbor->is_flagged)
                    {
                        if (neighbor->is_mine)
                            nmines++;
                        else
                            return true;
                    } // incorrect flag placement (leads to a loss)
                    if (!neighbor->discovered && !neighbor->is_flagged)
                        clear = false;
                }
    if (nmines == board[row][col].surrounding_mines && !clear)
        return true;
    return false;
}

/**
 * @brief checks if the game is over
 *
 * @param this a cell of the game board
 * @return true if the cell contains a mine or if all the cells have been uncovered
 */
bool is_game_over(cell *this)
{
    return (game_over = (this->is_mine || !(goal - uncovered)));
}

/**
 * @brief prints the final state of the board after a game is over, with some final stats
 *
 * @param line the line to print
 */
void print_results(int line)
{
    switch (line)
    {
    case 0:
        printf(B_H_WHT "\t------ Game Over ------" RESET);
        break;
    case 1:
        printf("\tMoves: " H_CYN "%d" RESET, moves);
        break;
    case 2:
        printf("\tCells uncovered: " H_GRN "%d/%d" RESET, uncovered, goal);
        break;
    case 3:
        printf("\tRemaining cells: " H_YEL "%d" RESET, goal - uncovered);
        break;
    case 4:
        printf("\tMines left: " H_RED "%d" RESET, mines);
        break;
    case 5:
        break;
    case 6:
        if (goal - uncovered)
            printf("\tYou " BG_RED B_H_WHT " LOST " RESET " - Try again");
        else
            printf("\tYou " BG_GRN B_H_YEL " WON " RESET " - Well done!");
        break;
    case 8:
        printf("\t      q to exit");
        break;
    }
}
