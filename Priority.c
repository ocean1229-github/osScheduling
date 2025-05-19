#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCESSES 10
#define MAX_TIME 10000

typedef struct {
    int pid;
    int arrivalTime;
    int burstTime;
    double priority;
    int startTime;
    int endTime;
    int waitingTime;
} Process;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <output_file> <alpha>\n", argv[0]);
        return 1;
    }
    char* inputFile = argv[1];
    char* outputFile = argv[2];
    double alpha = atof(argv[3]);

    Process procs[NUM_PROCESSES];
    int remBurst[NUM_PROCESSES];
    int completed = 0;
    int currentTime = 0;

    FILE* in = fopen(inputFile, "r");
    if (!in) { perror("Error opening input file"); return 1; }
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (fscanf(in, "%d %lf %d %d", &procs[i].pid, &procs[i].priority,
                   &procs[i].arrivalTime, &procs[i].burstTime) != 4) {
            printf("Invalid input on line %d\n", i+1);
            fclose(in);
            return 1;
        }
        procs[i].startTime = -1;
        procs[i].endTime = 0;
        procs[i].waitingTime = 0;
        remBurst[i] = procs[i].burstTime;
    }
    fclose(in);

    for (int i = 0; i < NUM_PROCESSES - 1; i++) {
        for (int j = 0; j < NUM_PROCESSES - 1 - i; j++) {
            if (procs[j].arrivalTime > procs[j+1].arrivalTime) {
                Process tmp = procs[j];
                procs[j] = procs[j+1];
                procs[j+1] = tmp;
                int rb = remBurst[j];
                remBurst[j] = remBurst[j+1];
                remBurst[j+1] = rb;
            }
        }
    }

    FILE* out = fopen(outputFile, "w");
    if (!out) { perror("Error opening output file"); return 1; }

    while (completed < NUM_PROCESSES && currentTime < MAX_TIME) {
        int idx = -1;
        double maxPri = -1;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (procs[i].arrivalTime <= currentTime && remBurst[i] > 0) {
                if (idx < 0 || procs[i].priority > maxPri) {
                    maxPri = procs[i].priority;
                    idx = i;
                }
            }
        }
        if (idx < 0) {
            currentTime++;
            continue;
        }
        if (procs[idx].startTime < 0)
            procs[idx].startTime = currentTime;
        fprintf(out, "<time %d> process %d is running\n", currentTime, procs[idx].pid);
        remBurst[idx]--;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (i != idx && procs[i].arrivalTime <= currentTime && remBurst[i] > 0) {
                procs[i].waitingTime++;
                procs[i].priority += alpha;
            }
        }
        currentTime++;
        if (remBurst[idx] == 0) {
            procs[idx].endTime = currentTime;
            completed++;
        }
    }

    // 성능 지표 계산
    double totalResp = 0, totalWait = 0, totalTurn = 0;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        totalResp += procs[i].startTime - procs[i].arrivalTime;
        totalWait += procs[i].waitingTime;
        totalTurn += procs[i].endTime - procs[i].arrivalTime;
    }

    fprintf(out, "===============================\n");
    fprintf(out, "Average response time: %.2f\n", totalResp / NUM_PROCESSES);
    fprintf(out, "Average waiting time: %.2f\n", totalWait / NUM_PROCESSES);
    fprintf(out, "Average turnaround time: %.2f\n", totalTurn / NUM_PROCESSES);

    fclose(out);
    return 0;
}