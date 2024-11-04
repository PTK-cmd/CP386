#include <stdio.h>
#define MAX_THREADS 50


typedef struct
{
    int id;
    int arrival_time;
    int burst_time;
} Thread;

int main()
{
    FILE *fp;
    fp = fopen("sample_in_schedule.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    
    Thread threads[MAX_THREADS];
    int num_threads = 0;
    while (fscanf(fp, "%d,%d,%d", &threads[num_threads].id, &threads[num_threads].arrival_time, &threads[num_threads].burst_time) == 3)
    {
        num_threads++;
    }
    fclose(fp);

    
    int completion_time = 0;
    int total_turnaround_time = 0;
    int total_waiting_time = 0;
    printf("Thread ID\tArrival Time\tBurst Time\tCompletion Time \tTurnaround Time \tWaiting Time\n");
    printf("----------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < num_threads; i++)
    {
        completion_time += threads[i].burst_time;
        int turnaround_time = completion_time - threads[i].arrival_time;
        int waiting_time = turnaround_time - threads[i].burst_time;
        total_turnaround_time += turnaround_time;
        total_waiting_time += waiting_time;

        printf("%d\t\t%d\t\t%d\t\t%d\t\t\t%d\t\t\t%d\n", threads[i].id, threads[i].arrival_time, threads[i].burst_time, completion_time, turnaround_time, waiting_time);
    }

    double avg_waiting_time = (float)total_waiting_time / num_threads;
    double avg_turnaround_time = (float)total_turnaround_time / num_threads;

    printf("----------------------------------------------------------------------------------------------\n");
    printf("The Average Waiting Time: %.2f\n", avg_waiting_time);
    printf("The Average Turnaround Time: %.2f\n", avg_turnaround_time);

    return 0;
}