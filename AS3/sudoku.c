#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9

// Sudoku puzzle solution
int sudoku[SIZE][SIZE];

// Thread function to check that each column contains the digits 1 through 9
void *check_column(void *arg)
{
    int col = *(int *)arg;
    int check[SIZE] = {0};
    for (int i = 0; i < SIZE; i++)
    {
        int num = sudoku[i][col];
        if (num < 1 || num > SIZE || check[num - 1])
        {
            pthread_exit((void *)0);
        }
        check[num - 1] = 1;
    }
    pthread_exit((void *)1);
}

// Thread function to check that each row contains the digits 1 through 9
void *check_row(void *arg)
{
    int row = *(int *)arg;
    int check[SIZE] = {0};
    for (int i = 0; i < SIZE; i++)
    {
        int num = sudoku[row][i];
        if (num < 1 || num > SIZE || check[num - 1])
        {
            pthread_exit((void *)0);
        }
        check[num - 1] = 1;
    }
    pthread_exit((void *)1);
}

// Thread function to check that each 3x3 sub-grid contains the digits 1 to 9
void *check_subgrid(void *arg)
{
    int subgrid = *(int *)arg;
    int check[SIZE] = {0};
    int start_row = (subgrid / 3) * 3;
    int start_col = (subgrid % 3) * 3;
    for (int i = start_row; i < start_row + 3; i++)
    {
        for (int j = start_col; j < start_col + 3; j++)
        {
            int num = sudoku[i][j];
            if (num < 1 || num > SIZE || check[num - 1])
            {
                pthread_exit((void *)0);
            }
            check[num - 1] = 1;
        }
    }
    pthread_exit((void *)1);
}

int main()
{
    // Read Sudoku puzzle solution from file given
    FILE *f = fopen("sample_in_sudoku.txt", "r");
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fscanf(f, "%d", &sudoku[i][j]);
        }
    }
    fclose(f);

    // Create threads to check each column in Sudoku
    pthread_t column_threads[SIZE];
    int column_args[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        column_args[i] = i;
        pthread_create(&column_threads[i], NULL, check_column, &column_args[i]);
    }

    // Create threads to check each row in Sudoku
    pthread_t row_threads[SIZE];
    int row_args[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        row_args[i] = i;
        pthread_create(&row_threads[i], NULL, check_row, &row_args[i]);
    }

    // Create threads to check each sub-grid in Sudoku
    pthread_t subgrid_threads[SIZE];
    int subgrid_args[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        subgrid_args[i] = i;
        pthread_create(&subgrid_threads[i], NULL, check_subgrid, &subgrid_args[i]);
    }

    // Wait for all threads to finish
    int column_results[SIZE];
    int row_results[SIZE];
    int subgrid_results[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        pthread_join(column_threads[i], (void **)&column_results[i]);
        pthread_join(row_threads[i], (void **)&row_results[i]);
        pthread_join(subgrid_threads[i], (void **)&subgrid_results[i]);
    }

    // Check if Sudoku puzzle solution is valid
    int valid = 1;
    for (int i = 0; i < SIZE; i++)
    {
        if (!column_results[i] || !row_results[i] || !subgrid_results[i])
        {
            valid = 0;
            break;
        }
    }

    // Output sudoku and validate it
    printf("Sudoku Puzzle Solution is:\n");
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }
    if (valid)
    {
        printf("Sudoku puzzle is valid.\n");
    }
    else
    {
        printf("Sudoku puzzle is invalid.\n");
    }

    return 0;
}