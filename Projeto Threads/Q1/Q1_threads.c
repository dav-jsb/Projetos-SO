#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 5 // Número de threads;
#define NUM_ARRAY 3 // Número de elementos do vetor;

typedef struct{
    int pos_init;
    int partition;
    int* data;
    int partial;
} _subarray;

void* Sum(void* subarray){
    _subarray* array = (_subarray*) subarray;
    int pos_init = array->pos_init;
    int partition = array->partition;

    for (int j = pos_init; j < (pos_init + partition); j++){
        array->partial += array->data[j];
    }

    printf("Parcial: %d\n", array->partial);

    pthread_exit((void*) array);
}

int* SetToZero(int* array){
    for (int i = 0; i < NUM_ARRAY; i++){
        array[i] = 29;
    }

    return array;
}

//No Scheduler, vamos designar para cada uma das threads quantos elementos elas vão processar;
int* Scheduler(int num_array, int n){
    int partition = num_array / n; //Primeiro fazemos a partição inteira da array;
    int remainder = num_array % n;
    int i = 0;

    int* schedule = NULL;

    if (!(schedule = (int*) malloc(n * sizeof(int)))){ //Alocamos a memória para o schedule, pois queremos retornar a partição de cada Thread;
        printf("Erro de alocação de memória no scheduler!\n");
        return NULL;
    }

    while (i < n){
        schedule[i] = partition;
        
        if (remainder > 0){
            schedule[i]++;
            remainder--;
        }

        i++;
    }

    return schedule;
}

int main(int argc, char** argv){
    pthread_t thread_list[N];
    int pos, total = 0;
    int rc;
    int* numeric_array = NULL;
    int* schedule = Scheduler(NUM_ARRAY, N);
    int part_results[N];
    _subarray __subarray[N];

    if (!(numeric_array = (int*) malloc(NUM_ARRAY * sizeof(int)))){
        printf("Erro de alocação de memória para array numérica!\n");
        exit(1);
    }

    numeric_array = SetToZero(numeric_array);

    for (int i = 0; i < N; i++){ //mudar i para N
        __subarray[i].data = numeric_array;
        __subarray[i].partition = schedule[i];
        __subarray[i].pos_init = pos;
        __subarray[i].partial = 0;

        rc = pthread_create(&thread_list[i], NULL, Sum, (void*) &__subarray[i]);
        if (rc){
            printf("Erro de criação da thread; Código de erro: %d\n", rc);
            exit(1);
        }

        pos += schedule[i];
    }

    for (int j = 0; j < N; j++){ //MUDAR J PARA N
        _subarray* _array;
        
        rc = pthread_join(thread_list[j], (void**) &_array);
        if (rc){
            printf("Erro de join da thread; Código de erro: %d\n", rc);
            exit(1);
        }

        total += _array->partial;
    }

    free(numeric_array);
    free(schedule);

    printf("Total: %d\n", total);
}