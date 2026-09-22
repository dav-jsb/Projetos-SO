#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 10 //Número de threads;
#define I 4 //Número de incógnitas;
#define P 10 //Número de iterações;

typedef struct{
    int pos_init;
    int partition;
} thread_data;

pthread_barrier_t barrier;
float coeficients[I][I] = {{10, -1, 2, 0}, {-1, 11, -1 , 3}, {2, -1, 10, -1}, {0, 3, -1, 8}};
float variable[I];
float independent[I] = {6, 25, -11, 15};

void InitBarrier(int count, pthread_barrier_t* barrier_var){
    int rc;
    if (rc = pthread_barrier_init(barrier_var, NULL, count)){
        printf("Erro ao inicializar barrier\n");
        exit(1);
    }
}

void DestroyBarrier(pthread_barrier_t* barrier_var){
    pthread_barrier_destroy(barrier_var);
}

int Scheduler(int* schedule){
    int remainder = I%N;
    int partition = I/N;
    int count = N;

    for (int i = 0; i < I; i++){
        schedule[i] = partition;
        if (remainder > 0){ 
            schedule[i]++;
            remainder--;
        }
    }

    if (!partition) count = I;
    return count;
}

void SetVariable(){
    for (int i = 0; i < I; i++) variable[i] = 1;
}

void* Jacobi(void* thread_file){
    thread_data data = *(thread_data*) thread_file;
    int iter = P;
    float sum_ax;
    int pos_init = data.pos_init;
    int partition = data.partition;

    if (partition){
        float temp_variable[partition];
        while (iter > 0){
            for (int i = 0; i < partition; i++){
                sum_ax = 0;
                for (int j = 0; j < I; j++){
                    if (j != (pos_init + i)) sum_ax += coeficients[pos_init + i][j] * variable[j];
                }
                temp_variable[i] = (independent[pos_init + i] - sum_ax) / coeficients[pos_init+i][pos_init+i];
            }
            iter--;
            pthread_barrier_wait(&barrier);
            for (int i = 0; i < partition; i++){
                variable[pos_init + i] = temp_variable[i];
            }
            pthread_barrier_wait(&barrier);
        }
    }
    pthread_exit(NULL);
}

int main(){
    thread_data data_list[N];
    pthread_t thread_list[N];
    int schedule[N];
    int count, rc, jc;
    int pos = 0;
    
    SetVariable();
    count = Scheduler(schedule);
    InitBarrier(count, &barrier);
    
    for (int i = 0; i < count; i++){
        data_list[i].partition = schedule[i];
        data_list[i].pos_init = pos;
        pos += data_list[i].partition;

        if (rc = pthread_create(&thread_list[i], NULL, Jacobi, (void*) &data_list[i])){
            printf("Erro na criacao da thread\n");
            exit(1);
        }

    }

    for (int i = 0; i < count; i++){
        jc = pthread_join(thread_list[i], NULL);
        if (jc){
            printf("Erro de join na thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < I; i++){
        printf("x%d: %f; ", i+1, variable[i]);
    }
    printf("\n");

    DestroyBarrier(&barrier);
    pthread_exit(NULL);
}