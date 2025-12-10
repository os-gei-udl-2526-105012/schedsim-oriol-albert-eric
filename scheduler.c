#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "process.h"
#include "queue.h"
#include "scheduler.h"

int num_algorithms() {
  return sizeof(algorithmsNames) / sizeof(char *);
}

int num_modalities() {
  return sizeof(modalitiesNames) / sizeof(char *);
}

size_t initFromCSVFile(char* filename, Process** procTable){
    FILE* f = fopen(filename,"r");
    
    size_t procTableSize = 10;
    
    *procTable = malloc(procTableSize * sizeof(Process));
    Process * _procTable = *procTable;

    if(f == NULL){
      perror("initFromCSVFile():::Error Opening File:::");   
      exit(1);             
    }

    char* line = NULL;
    size_t buffer_size = 0;
    size_t nprocs= 0;
    while( getline(&line,&buffer_size,f)!=-1){
        if(line != NULL){
            Process p = initProcessFromTokens(line,";");

            if (nprocs==procTableSize-1){
                procTableSize=procTableSize+procTableSize;
                _procTable=realloc(_procTable, procTableSize * sizeof(Process));
            }

            _procTable[nprocs]=p;

            nprocs++;
        }
    }
   free(line);
   fclose(f);
   return nprocs;
}

size_t getTotalCPU(Process *procTable, size_t nprocs){
    size_t total=0;
    for (int p=0; p<nprocs; p++ ){
        total += (size_t) procTable[p].burst;
    }
    return total;
}

int getCurrentBurst(Process* proc, int current_time){
    int burst = 0;
    for(int t=0; t<current_time; t++){
        if(proc->lifecycle[t] == Running){
            burst++;
        }
    }
    return burst;
}

bool isBetter(Process *process1, Process *process2, int algorithm, int t) {
    if (algorithm == SJF) {
        return compareBurst(process1, process2) < 0;
    } else if (algorithm == PRIORITIES) {
        return comparePriority(process1, process2) < 0;
    }
    return false;
}

int run_dispatcher(Process *procTable, size_t nprocs, int algorithm, int modality, int quantum){

    Process * _proclist;

    qsort(procTable,nprocs,sizeof(Process),compareArrival);

    init_queue();
    size_t duration = getTotalCPU(procTable, nprocs) +1;

    for (int p=0; p<nprocs; p++ ){
        procTable[p].lifecycle = malloc( duration * sizeof(int));
        for(int t=0; t<duration; t++){
            procTable[p].lifecycle[t]=-1;
        }
        procTable[p].waiting_time = 0;
        procTable[p].return_time = -1;
        procTable[p].response_time = -1;
        procTable[p].completed = false;
    }

    Process* current = NULL;
    int quantum_counter = 0;
    
    for (int t = 0; t < duration; t++) {
        for (int p = 0; p < nprocs; p++) {
            if (procTable[p].arrive_time == t) {
                enqueue(&procTable[p]);
            }
        }
        
        if (current != NULL && current->burst <= 0) {
            current->completed = true;
            current->return_time = t - current->arrive_time;
            current->lifecycle[t] = Finished;
            current = NULL;
            quantum_counter = 0;
        }
        
        if (get_queue_size() > 0) {
            if (algorithm == SJF) {
                _proclist = transformQueueToList();
                qsort(_proclist, get_queue_size(), sizeof(Process), compareBurst);
                setQueueFromList(_proclist);
                free(_proclist);
            } else if (algorithm == PRIORITIES) {
                _proclist = transformQueueToList();
                qsort(_proclist, get_queue_size(), sizeof(Process), comparePriority);
                setQueueFromList(_proclist);
                free(_proclist);
            }
            // FCFS i RR no necessita reordernar

            if (modality == PREEMPTIVE) {
                if (current == NULL) {
                    current = dequeue();
                    quantum_counter=0;
                } else {
                    Process *best = peek(); // Mira el següent a sortir (funció creada per nosaltres)
                    switch(algorithm){
                        case RR:
                            if (quantum_counter >= quantum) {
                                enqueue(current);
                                current = dequeue();
                                quantum_counter=0;
                            }
                            break;
                        default:
                            if (isBetter(best, current, algorithm, t)) {
                                enqueue(current);
                                current = dequeue();
                            }
                    }   
                }
            } else if (modality == NONPREEMPTIVE) {
                if (current == NULL) {
                    current = dequeue();
                }
            }
        }
        
        if (current != NULL) {
            current->lifecycle[t] = Running;
            current->burst--;
            quantum_counter++;

            if (current->response_time == -1) {
                current->response_time = t - current->arrive_time;
            }
        }
        
        if (get_queue_size() > 0) {
            _proclist = transformQueueToList();
            for (int i = 0; i < get_queue_size(); i++) {
                _proclist[i].waiting_time++;
            }
            setQueueFromList(_proclist);
            free(_proclist);
        }
    }

    qsort(procTable,nprocs,sizeof(Process),compareArrival);
    printf("== PROCESSES ==\n");
    for (int p = 0; p < nprocs; p++) {
        printProcess(procTable[p]);
        printf(" - waiting_time=%d\n", procTable[p].waiting_time);
        printf(" - return_time=%d\n", procTable[p].return_time);
        printf(" - response_time=%d\n", procTable[p].response_time);
        printf("\n");
    }

    printSimulation(nprocs,procTable,duration);
    printf("\n");
    printMetrics(duration, nprocs, procTable);

    for (int p=0; p<nprocs; p++ ){
        destroyProcess(procTable[p]);
    }

    cleanQueue();
    return EXIT_SUCCESS;

}


void printSimulation(size_t nprocs, Process *procTable, size_t duration){

    printf("%14s","== SIMULATION ");
    for (int t=0; t<duration; t++ ){
        printf("%5s","=====");
    }
    printf("\n");

    printf ("|%4s", "name");
    for(int t=0; t<duration; t++){
        printf ("|%2d", t);
    }
    printf ("|\n");

    for (int p=0; p<nprocs; p++ ){
        Process current = procTable[p];
            printf ("|%4s", current.name);
            for(int t=0; t<duration; t++){
                printf("|%2s",  (current.lifecycle[t]==Running ? "E" : 
                        current.lifecycle[t]==Bloqued ? "B" :   
                        current.lifecycle[t]==Finished ? "F" : " "));
            }
            printf ("|\n");
        
    }


}

void printMetrics(size_t simulationCPUTime, size_t nprocs, Process *procTable ){

    printf("%-14s","== METRICS ");
    for (int t=0; t<simulationCPUTime+1; t++ ){
        printf("%5s","=====");
    }
    printf("\n");

    printf("= Duration: %ld\n", simulationCPUTime );
    printf("= Processes: %ld\n", nprocs );

    size_t baselineCPUTime = getTotalCPU(procTable, nprocs);
    double throughput = (double) nprocs / (double) simulationCPUTime;
    double cpu_usage = (double) simulationCPUTime / (double) baselineCPUTime;

    printf("= CPU (Usage): %lf\n", cpu_usage*100 );
    printf("= Throughput: %lf\n", throughput*100 );

    double averageWaitingTime = 0;
    double averageResponseTime = 0;
    double averageReturnTime = 0;
    double averageReturnTimeN = 0;

    for (int p=0; p<nprocs; p++ ){
            averageWaitingTime += procTable[p].waiting_time;
            averageResponseTime += procTable[p].response_time;
            averageReturnTime += procTable[p].return_time;
            averageReturnTimeN += procTable[p].return_time / (double) procTable[p].burst;
    }


    printf("= averageWaitingTime: %lf\n", (averageWaitingTime/(double) nprocs) );
    printf("= averageResponseTime: %lf\n", (averageResponseTime/(double) nprocs) );
    printf("= averageReturnTimeN: %lf\n", (averageReturnTimeN/(double) nprocs) );
    printf("= averageReturnTime: %lf\n", (averageReturnTime/(double) nprocs) );

}