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
void display_cpu_queues(Process *process_list, int process_count);

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

// Function to load processes from input file
void load_processes(const char *filename, Process *process_list, int *process_count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) { // Check if the file opened successfully
        perror("Error opening file");
        exit(1);
    }

    // Read process details from file
    while (fscanf(file, "%[^,],%d,%d,%d,%d,%d\n", 
                  process_list[*process_count].name, 
                  &process_list[*process_count].arrival_time, 
                  &process_list[*process_count].priority, 
                  &process_list[*process_count].burst_time, 
                  &process_list[*process_count].ram, 
                  &process_list[*process_count].cpu_rate) != EOF) {
        (*process_count)++;
    }

    fclose(file); // Close the file
}

// Function to allocate processes to CPUs
void allocate_processes(Process *process_list, int process_count, FILE *output_file) {
    int ram_usage_cpu1 = 0; // RAM usage by CPU1
    int ram_usage_cpu2 = 0; // RAM usage by CPU2

    // Arrays to hold processes for each CPU and priority queue
    Process cpu1_queue[MAX_PROCESSES];
    Process cpu2_queue_priority1[MAX_PROCESSES];
    Process cpu2_queue_priority2[MAX_PROCESSES];
    Process cpu2_queue_priority3[MAX_PROCESSES];

    // Counters for processes in each queue
    int cpu1_count = 0;
    int cpu2_count_priority1 = 0;
    int cpu2_count_priority2 = 0;
    int cpu2_count_priority3 = 0;

    // Allocate processes to appropriate CPU and queue
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 0) { // CPU1 processes
            if (ram_usage_cpu1 + process_list[i].ram <= RESERVED_RAM_FOR_CPU1) {
                cpu1_queue[cpu1_count++] = process_list[i];
                ram_usage_cpu1 += process_list[i].ram;
                fprintf(output_file, "Process %s is queued to be assigned to CPU-1.\n", process_list[i].name);
            }
        } else { // CPU2 processes
            if (ram_usage_cpu2 + process_list[i].ram <= (RAM_SIZE - RESERVED_RAM_FOR_CPU1)) {
                if (process_list[i].priority == 1) {
                    cpu2_queue_priority1[cpu2_count_priority1++] = process_list[i];
                } else if (process_list[i].priority == 2) {
                    cpu2_queue_priority2[cpu2_count_priority2++] = process_list[i];
                } else if (process_list[i].priority == 3) {
                    cpu2_queue_priority3[cpu2_count_priority3++] = process_list[i];
                }
                ram_usage_cpu2 += process_list[i].ram;
                fprintf(output_file, "Process %s is queued to be assigned to CPU-2.\n", process_list[i].name);
            }
        }
    }

    // Process CPU1 queue using FCFS scheduling
    for (int i = 0; i < cpu1_count; i++) {
        fprintf(output_file, "Process %s is assigned to CPU-1.\n", cpu1_queue[i].name);
        fprintf(output_file, "Process %s is completed and terminated.\n", cpu1_queue[i].name);
    }

    // Process CPU2 queues using different scheduling algorithms
    if (cpu2_count_priority1 > 0) {
        sort_by_burst_time(cpu2_queue_priority1, cpu2_count_priority1); // SJF for priority 1
        for (int i = 0; i < cpu2_count_priority1; i++) {
            fprintf(output_file, "Process %s is assigned to CPU-2 (Priority 1, SJF).\n", cpu2_queue_priority1[i].name);
            fprintf(output_file, "Process %s is completed and terminated.\n", cpu2_queue_priority1[i].name);
        }
    }

    if (cpu2_count_priority2 > 0) {
        round_robin(cpu2_queue_priority2, cpu2_count_priority2, CPU2_QUANTUM_MEDIUM, output_file); // RR for priority 2
    }

    if (cpu2_count_priority3 > 0) {
        round_robin(cpu2_queue_priority3, cpu2_count_priority3, CPU2_QUANTUM_LOW, output_file); // RR for priority 3
    }
}

// Function to sort processes by burst time for SJF scheduling
void sort_by_burst_time(Process *queue, int count) {
    int i, j, min_idx;

    // Selection sort algorithm
    for (i = 0; i < count - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < count; j++) {
            if (queue[j].burst_time < queue[min_idx].burst_time) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            Process temp = queue[min_idx];
            queue[min_idx] = queue[i];
            queue[i] = temp;
        }
    }
}

// Round Robin scheduling algorithm
void round_robin(Process *queue, int count, int quantum, FILE *output_file) {
    int time = 0; // Current time
    int remaining_burst_times[MAX_PROCESSES]; // Array to hold remaining burst times

    // Initialize remaining burst times
    for (int i = 0; i < count; i++) {
        remaining_burst_times[i] = queue[i].burst_time;
    }

    // Process each process in a round-robin manner
    while (1) {
        int done = 1; // Flag to check if all processes are done

        for (int i = 0; i < count; i++) {
            if (remaining_burst_times[i] > 0) {
                done = 0; // There are still processes to execute

                if (remaining_burst_times[i] > quantum) {
                    time += quantum;
                    remaining_burst_times[i] -= quantum;
                } else {
                    time += remaining_burst_times[i];
                    remaining_burst_times[i] = 0;
                    fprintf(output_file, "Process %s is completed and terminated.\n", queue[i].name);
                }
            }
        }

        if (done == 1) { // If all processes are done
            break;
        }
    }
}

// Function to display CPU queues
void display_cpu_queues(Process *process_list, int process_count) {
    printf("CPU-1 queue (priority-0, FCFS)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 0 && process_list[i].ram <= RAM_SIZE / 4) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");

    printf("CPU-2 queue (priority-1, SJF)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 1) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");

    printf("CPU-2 queue (priority-2, RR-q8)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 2) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");

    printf("CPU-2 queue (priority-3, RR-q16)→ ");
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].priority == 3) {
            printf("%s-", process_list[i].name);
        }
    }
    printf("\n");
}
