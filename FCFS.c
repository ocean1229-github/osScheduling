#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCESSES 10
#define MAX_TIME 1000

typedef struct {
    int pid;
    int arrivalTime;
    int burstTime;
    double startTime;
    double endTime;
} Process;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE* in = fopen(argv[1], "r");
    if (!in) {
        perror("Error opening input file");
        return 1;
    }

    Process procs[NUM_PROCESSES];
    int dummy;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (fscanf(in, "%d %d %d %d", &procs[i].pid, &dummy,
                   &procs[i].arrivalTime, &procs[i].burstTime) != 4) {
            printf("Invalid input format on line %d\n", i + 1);
            fclose(in);
            return 1;
        }
        procs[i].startTime = 0;
        procs[i].endTime = 0;
    }
    fclose(in);

    // Sort processes by arrival time (simple bubble sort)
    for (int i = 0; i < NUM_PROCESSES - 1; i++) {
        for (int j = 0; j < NUM_PROCESSES - i - 1; j++) {
            if (procs[j].arrivalTime > procs[j + 1].arrivalTime) {
                Process tmp = procs[j];
                procs[j] = procs[j + 1];
                procs[j + 1] = tmp;
            }
        }
    }

    int schedule[MAX_TIME];       // PID running at each time unit, -1 if idle
    int currentTime = 0;
    int processedCount = 0;
    int nextToArrive = 0;

    int readyQueue[NUM_PROCESSES];
    int head = 0, tail = 0;

    // Simulation loop
    while (processedCount < NUM_PROCESSES && currentTime < MAX_TIME) {
        // Enqueue newly arrived processes
        while (nextToArrive < NUM_PROCESSES &&
               procs[nextToArrive].arrivalTime == currentTime) {
            readyQueue[tail] = nextToArrive;
            tail = (tail + 1) % NUM_PROCESSES;
            printf("<time %d> process %d arrived\n", 
                    currentTime, procs[nextToArrive].pid);
            nextToArrive++;
        }

        if (head == tail) {
            // No ready process, CPU is idle
            schedule[currentTime++] = -1;
        } else {
            // Dequeue next process
            int idx = readyQueue[head];
            head = (head + 1) % NUM_PROCESSES;
            Process* p = &procs[idx];
            p->startTime = currentTime;
            printf("<time %d> process %d is running\n", 
                   currentTime, p->pid);

            // Run process for its burst time
            for (int t = 0; t < p->burstTime; t++) {
                schedule[currentTime++] = p->pid;
                // Check for arrivals during execution
                while (nextToArrive < NUM_PROCESSES &&
                       procs[nextToArrive].arrivalTime == currentTime) {
                    readyQueue[tail] = nextToArrive;
                    tail = (tail + 1) % NUM_PROCESSES;
                    printf("<time %d> process %d arrived\n", 
                           currentTime, procs[nextToArrive].pid);
                    nextToArrive++;
                }
            }
            p->endTime = currentTime;
            processedCount++;
        }
    }

    FILE* out = fopen(argv[2], "w");
    if (!out) {
        perror("Error opening output file");
        return 1;
    }

    // Calculate metrics
    double totalResponse = 0;
    double totalWaiting = 0;
    double totalTurnaround = 0;
    int idleTime = 0;

    for (int t = 0; t < currentTime; t++) {
        if (schedule[t] == -1) idleTime++;
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        totalResponse    += procs[i].startTime   - procs[i].arrivalTime;
        totalWaiting     += procs[i].startTime   - procs[i].arrivalTime;
        totalTurnaround  += procs[i].endTime     - procs[i].arrivalTime;
    }

    fprintf(out, "FCFS Scheduling Results\n");
    fprintf(out, "========================\n");
    fprintf(out, "Total time: %d\n", currentTime);
    fprintf(out, "CPU idle time: %d\n", idleTime);
    fprintf(out, "CPU utilization: %.2f%%\n", 
            100.0 * (currentTime - idleTime) / currentTime);
    fprintf(out, "Average response time: %.2f\n", totalResponse   / NUM_PROCESSES);
    fprintf(out, "Average waiting time: %.2f\n",  totalWaiting    / NUM_PROCESSES);
    fprintf(out, "Average turnaround time: %.2f\n", totalTurnaround / NUM_PROCESSES);

    fclose(out);
    return 0;
}
