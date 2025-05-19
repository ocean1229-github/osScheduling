#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCESSES 10
#define MAX_TIME 10000

typedef struct {
    int pid;
    int arrivalTime;
    int burstTime;
    int startTime;
    int endTime;
} Process;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <output_file> <quantum>\n", argv[0]);
        return 1;
    }
    char* inputFile = argv[1];
    char* outputFile = argv[2];
    int quantum = atoi(argv[3]);

    Process procs[NUM_PROCESSES];
    int origBurst[NUM_PROCESSES];
    int remBurst[NUM_PROCESSES];
    int queue[NUM_PROCESSES];
    int head = 0, tail = 0;
    int nextArrive = 0;
    int completed = 0;
    int currentTime = 0;

    // Read input
    FILE* in = fopen(inputFile, "r");
    if (!in) { perror("Error opening input file"); return 1; }
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int dummy;
        if (fscanf(in, "%d %d %d %d", &procs[i].pid, &dummy,
                   &procs[i].arrivalTime, &procs[i].burstTime) != 4) {
            printf("Invalid input format on line %d\n", i+1);
            fclose(in);
            return 1;
        }
        procs[i].startTime = -1;
        procs[i].endTime = 0;
        origBurst[i] = procs[i].burstTime;
        remBurst[i] = procs[i].burstTime;
    }
    fclose(in);

    // Sort by arrival time
    for (int i = 0; i < NUM_PROCESSES - 1; i++) {
        for (int j = 0; j < NUM_PROCESSES - 1 - i; j++) {
            if (procs[j].arrivalTime > procs[j+1].arrivalTime) {
                Process tmp = procs[j];
                procs[j] = procs[j+1];
                procs[j+1] = tmp;
                int ob = origBurst[j]; origBurst[j] = origBurst[j+1]; origBurst[j+1] = ob;
                int rb = remBurst[j];   remBurst[j]   = remBurst[j+1];   remBurst[j+1]   = rb;
            }
        }
    }

    FILE* out = fopen(outputFile, "w");
    if (!out) { perror("Error opening output file"); return 1; }

    int curr = -1;
    int qcount = 0;

    // Simulation loop
    while (completed < NUM_PROCESSES && currentTime < MAX_TIME) {
        // Enqueue new arrivals
        while (nextArrive < NUM_PROCESSES &&
               procs[nextArrive].arrivalTime == currentTime) {
            queue[tail] = nextArrive;
            tail = (tail + 1) % NUM_PROCESSES;
            fprintf(out, "<time %d> process %d arrived\n", currentTime, procs[nextArrive].pid);
            nextArrive++;
        }
        // If no running process, pick next
        if (curr < 0) {
            if (head != tail) {
                curr = queue[head];
                head = (head + 1) % NUM_PROCESSES;
                qcount = 0;
                if (procs[curr].startTime < 0)
                    procs[curr].startTime = currentTime;
            } else {
                currentTime++;
                continue;
            }
        }
        // Run current for 1 unit
        fprintf(out, "<time %d> process %d is running\n", currentTime, procs[curr].pid);
        remBurst[curr]--;
        currentTime++;
        qcount++;

        // Enqueue arrivals during execution
        while (nextArrive < NUM_PROCESSES &&
               procs[nextArrive].arrivalTime == currentTime) {
            queue[tail] = nextArrive;
            tail = (tail + 1) % NUM_PROCESSES;
            fprintf(out, "<time %d> process %d arrived\n", currentTime, procs[nextArrive].pid);
            nextArrive++;
        }
        // Check if finished
        if (remBurst[curr] == 0) {
            procs[curr].endTime = currentTime;
            completed++;
            curr = -1;
        }
        // Check if quantum expired
        else if (qcount == quantum) {
            queue[tail] = curr;
            tail = (tail + 1) % NUM_PROCESSES;
            curr = -1;
        }
    }

    // Calculate metrics
    double totalResp = 0;
    double totalWait = 0;
    double totalTurn = 0;
    int idleTime = 0;

    // For simplicity, idleTime is not counted here
    for (int i = 0; i < NUM_PROCESSES; i++) {
        totalResp += (procs[i].startTime - procs[i].arrivalTime);
        totalTurn += (procs[i].endTime   - procs[i].arrivalTime);
        totalWait += (totalTurn - origBurst[i]);
    }

    fprintf(out, "===============================\n");
    fprintf(out, "Average response time: %.2f\n", totalResp / NUM_PROCESSES);
    fprintf(out, "Average waiting time: %.2f\n",  totalWait / NUM_PROCESSES);
    fprintf(out, "Average turnaround time: %.2f\n", totalTurn / NUM_PROCESSES);

    fclose(out);
    return 0;
}
