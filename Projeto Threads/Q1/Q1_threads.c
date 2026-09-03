//Abordagem para resolução do problema:
//Nossa abordagem consiste em utilizar-se de uma função de escalonamento;
//A qual deve designar, para cada thread, o trecho do vetor que ela vai somar;
//Nosso escalonamento é feito na função Scheduler, a qual tem como entradas o número de elementos do vetor e o número de threads;

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 5 // Número de threads;
#define NUM_ARRAY 10000 // Número de elementos do vetor;

typedef struct{
    int thread_id; //Id da thread;
    int pos_init;  //Posição de início da thread no vetor;
    int partition; //Pedaço do vetor que a thread deve somar;
    int* data;     //Cópia do vetor numérico;
    int partial;   //Resultado da soma parcial;
} _subarray;       //Abstração: Pedaço da array que a thread deve operar sobre;
                   //Prática: Armazena posição de início, partição, cópia da array e soma parcial de cada thread;

//Função geradora da thread;
void* Sum(void* subarray){
    _subarray* array = (_subarray*) subarray;
    int pos_init = array->pos_init;
    int partition = array->partition;

    for (int j = pos_init; j < (pos_init + partition); j++){ //Loop responsável por somar cada elemento do trecho da array designado para cada thread;
        array->partial += array->data[j];                    //A definição do loop garante que uma thread não vá somar valores incoerentes da memória;
    }

    printf("Parcial da thread %d: %d\n", array->thread_id, array->partial); //Print da soma parcial da thread;
    pthread_exit((void*) array);             //Retorno da thread;
}

//No Scheduler, vamos designar para cada uma das threads quantos elementos elas vão processar;
int* Scheduler(int num_array, int n){
    int partition = num_array / n; //Primeiro fazemos a partição inteira da array;
    int remainder = num_array % n; //Resto da divisão do número de elementos da array para a quantidade de threads (utilizamos para melhor distribuir os valores para cada thread);
    int i = 0;

    int* schedule = NULL;

    if (!(schedule = (int*) malloc(n * sizeof(int)))){ //Alocamos a memória para o schedule, pois queremos retornar a partição de cada Thread;
        printf("Erro de alocação de memória no scheduler!\n");
        return NULL;
    }

    while (i < n){
        schedule[i] = partition;
        
        if (remainder > 0){ //Se houver resto, ele vai naturalmente sendo distribuído entre as threads (do começo para o final);
            schedule[i]++;
            remainder--;
        }

        i++;
    }

    return schedule;
}

int* SetArray(int* array){                  //Esta função gera uma array com NUM_ARRAY (valor definido no início do código) de tamanho;
    for (int i = 0; i < NUM_ARRAY; i++){    //Ela gera NUM_ARRAY valores repetidos (está inicialmente setado para 2, logo o resultado da soma deve ser NUM_ARRAY * 2);
        array[i] = 2;
    }

    return array;
}

int main(){
    int rc;
    int pos, total = 0;
    pthread_t thread_list[N];                   //Array de threads;
    int* schedule = Scheduler(NUM_ARRAY, N);    //Passamos para o Scheduler os valores de NUM_ARRAY e N;
    int* numeric_array = NULL; 
    _subarray __subarray[N];

    if (!(numeric_array = (int*) malloc(NUM_ARRAY * sizeof(int)))){     //Alocação de memória para a array de números;
        printf("Erro de alocação de memória para array numérica!\n");   //Usamos ponteiros para que possamos repassar essa cópia para cada thread;
        exit(1);
    }

    numeric_array = SetArray(numeric_array);

    for (int i = 0; i < N; i++){ 
        //Passamos para cada thread os parâmetros importantes para a função;
        __subarray[i].thread_id = i;
        __subarray[i].data = numeric_array;
        __subarray[i].partition = schedule[i];
        __subarray[i].pos_init = pos;
        __subarray[i].partial = 0;

        rc = pthread_create(&thread_list[i], NULL, Sum, (void*) &__subarray[i]); //Criação de cada uma das N threads;
        
        if (rc){
            printf("Erro de criação da thread; Código de erro: %d\n", rc);
            exit(1);
        }

        pos += schedule[i];
    }

    //Dentro deste loop, somamos os valores parciais de cada thread;
    for (int j = 0; j < N; j++){
        _subarray* _array;
        
        rc = pthread_join(thread_list[j], (void**) &_array);
        if (rc){
            printf("Erro de join da thread; Código de erro: %d\n", rc);
            exit(1);
        }

        total += _array->partial;
    }

    //Liberamos a memória da array numérica e do schedule;
    free(numeric_array);
    free(schedule);

    printf("Total: %d\n", total);
}