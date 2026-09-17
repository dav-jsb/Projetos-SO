#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_WRITE 5 // Nº de threads escritoras
#define NUM_READ 3 // Nº de threads de leitura

int elemento = 0; // Variável que vai preencher as posições do array.
int array[NUM_WRITE]; // Array que será usado pelas threads de escrita

pthread_mutex_t mutex;
pthread_cond_t cond_parada; // Variável de condição para bloquear as threads de leitura enquanto uma escrita estiver em execução
// A ideia é que, na função de leitura, haja um pthread_cond_wait para travar as threads de leitura enquanto as threads escritoras
// ainda não tiverem sido executadas por inteiro. Ao final das últimas (as threads escritoras), haverá um pthread_cond_broadcast
// para liberar todas as threas de leitura simultaneamente

void* write_rotina(){
    while(1){


        elemento++;
        pthread_cond_broadcast(&cond_parada); // Libera todas as threas de leitura
    }
}


void* read_rotina(){
    while(1){
        pthread_cond_wait(&cond_parada ,&mutex); // Aguarda a finalização das threads de escritura
        // Inserir leitura
    }
}

int main(int argc, char** argv){
    pthread_t threads[NUM_READ + NUM_WRITE]; // Array que irá armazenar todas as threads criadas

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_parada, NULL);

    for(int i = 0; i < NUM_READ + NUM_WRITE; i++){
        if(i < NUM_WRITE){ // Cria 5 threads escritoras
            if(pthread_create(threads + i, NULL, &write_rotina, NULL)){ // Adicionar argumentos para mandar p/ função
                perror("Erro ao criar as threads escritoras");
                return 1;
            }
        }

        else{ // Cria 3 threads leitoras
            if(pthread_create(threads + i, NULL, &read_rotina, NULL)){ // Adicionar argumentos p/ mandar p/ função
                perror("Erro ao criar as threads leitoras");
                return 2;
            }
        }
    }

    for(int j = 0; j < NUM_READ + NUM_WRITE; j++){
        if(pthread_join(threads[j], NULL)){ // Checagem de erro
            perror("Erro no join das threads");
            return 3;
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_parada);

    return 0;
}