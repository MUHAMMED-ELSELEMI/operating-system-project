#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100         // Maximum number of processes
#define RAM_SIZE 2048             // Total RAM size
#define RESERVED_RAM_FOR_CPU1 512 // Reserved RAM for CPU1
#define CPU2_QUANTUM_MEDIUM 8     // Quantum time for medium priority processes in CPU2
#define CPU2_QUANTUM_LOW 16       // Quantum time for low priority processes in CPU2

// Structure to represent a process
typedef struct {
    char name[5];       // Process name
    int arrival_time;   // Arrival time of the process
    int priority;       // Priority of the process
    int burst_time;     // Burst time of the process
    int ram;            // RAM required by the process
    int cpu_rate;       // CPU rate required by the process
} Process;

// Function prototypes
void load_processes(const char *filename, Process *process_list, int *process_count);
void allocate_processes(Process *process_list, int process_count, FILE *output_file);
void display_cpu_queues(Process *process_list, int process_count) {
    printf("CPU-1 que1(priority-0) (FCFS)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 0 && process_list[i].ram <= RESERVED_RAM_FOR_CPU1) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");

    printf("CPU-2 que2(priority-1) (SJF)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 1) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");

    printf("CPU-2 que3(priority-2) (RR-q8)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 2) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");

    printf("CPU-2 que4(priority-3) (RR-q16)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 3) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");
}

void sort_by_burst_time(Process *queue, int count);
void round_robin(Process *queue, int count, int quantum, FILE *output_file);

int main(int argc, char *argv[]) {
    if (argc != 2) { // Check for correct usage
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    Process process_list[MAX_PROCESSES]; // Array to hold processes
    int process_count = 0;               // Number of processes

    // Load processes from input file
    load_processes(argv[1], process_list, &process_count);

    // Open output file for writing
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) { // Check if the file opened successfully
        perror("Error opening output file");
        return 1;
    }

    // Allocate processes to CPUs
    allocate_processes(process_list, process_count, output_file);

    // Display CPU queues
    display_cpu_queues(process_list, process_count);

    // Close the output file
    fclose(output_file);

    return 0;
}
