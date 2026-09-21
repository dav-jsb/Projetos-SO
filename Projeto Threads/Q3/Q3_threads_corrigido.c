#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_WRITE 5 // Nº de threads escritoras
#define NUM_READ 3 // Nº de threads de leitura

int elemento = 0; // Variável que vai preencher as posições do array.
int array[NUM_WRITE]; // Array que será usado pelas threads de escrita
int escritor_ativo = 0; // Variável de 0 ou 1 que informa se há um escritor atualmente escrevendo ou não
int leitores_ativos = 0; // Quantidade de leitores atualmente sendo utilizados

pthread_mutex_t mutex;
pthread_cond_t cond_parada; // Variável de condição para bloquear as threads de leitura enquanto uma escrita estiver em execução
// A ideia é que, na função de leitura, haja um pthread_cond_wait para travar as threads de leitura enquanto as threads escritoras
// ainda não tiverem sido executadas por inteiro. Ao final das últimas (as threads escritoras), haverá um pthread_cond_broadcast
// para liberar todas as threas de leitura simultaneamente

void* write_rotina(void *arg){
    int index = *(int*) arg; // Retorna o tipo para int
    free(arg);

    while(1){
        pthread_mutex_lock(&mutex); // Trava o mutex ao entrar na thread

        while(escritor_ativo || leitores_ativos > 0){ // Enquanto houver uma thread de escrita ainda ativa ou uma thread lendo
            pthread_cond_wait(&cond_parada, &mutex);
        }

        escritor_ativo = 1; // Ativa a nova thread de escrita
        pthread_mutex_unlock(&mutex);

        elemento += 3; // Incrementa de 3 em 3 os valores
        array[index] = elemento;

        pthread_mutex_lock(&mutex);
        escritor_ativo = 0;
        pthread_cond_broadcast(&cond_parada); // Libera todas as threads em wait
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}


void* read_rotina(void *arg){
    int index = *(int*) arg; // Retorna o tipo para int
    free(arg);

    while(1){
        pthread_mutex_lock(&mutex);
        while(escritor_ativo){
            pthread_cond_wait(&cond_parada ,&mutex); // Aguarda a finalização das threads de escritura
        }

        leitores_ativos++;
        pthread_mutex_unlock(&mutex);

        printf("Leitura %d: %d %d %d %d %d\n\n", index-4, array[0], array[1], array[2], array[3], array[4]);
        // Printamos desta maneira para que cada thread mostre que está lendo simultaneamente sem que haja problemas no print.
        // Caso o array fosse printado em um for, as threads teriam um overlap, de forma que uma mesma thread pode entrar no loop e printar
        // simultaneamente, dificultando a leitura dos valores. Desta forma, apesar do código ser pouco escalonável, não há concorrência entre
        // os prints e todas as threads de leitura tem acesso simultaneo ao print do array

        pthread_mutex_lock(&mutex);
        leitores_ativos--;
        if(leitores_ativos == 0){
            pthread_cond_broadcast(&cond_parada); // Libera as threads escritoras
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

int main(int argc, char** argv){
    pthread_t threads[NUM_READ + NUM_WRITE]; // Array que irá armazenar todas as threads criadas

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_parada, NULL);

    for(int i = 0; i < NUM_READ + NUM_WRITE; i++){
        int* number = malloc(sizeof(int));
        *number = i; // Cada thread escritora vai escrever na posição correspondente em que foi criada
        if(i < NUM_WRITE){ // Cria 5 threads escritoras
            if(pthread_create(threads + i, NULL, &write_rotina, number)){
                perror("Erro ao criar as threads escritoras");
                return 1;
            }
        }

        else{ // Cria 3 threads leitoras
            if(pthread_create(threads + i, NULL, &read_rotina, number)){
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
