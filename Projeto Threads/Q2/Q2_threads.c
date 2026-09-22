#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define N 5 //Número de arquivos e threads;
#define L 3 //Número de linhas;
#define L_TABELA 7 //Número de linhas da tabela;

typedef struct{
    char nome[50];
    int consultorio;
}paciente;

pthread_mutex_t lock[L_TABELA];

void InitMutex(){
    for (int i = 0; i < L_TABELA; i++){
        if (pthread_mutex_init(&lock[i], NULL)){
            printf("Erro ao inicializar mutex\n");
            exit(1);
        }
    }
}

void DestroyMutex(){
    for (int i = 0; i < L_TABELA; i++){
        pthread_mutex_destroy(&lock[i]);
    }
}

void Leitura(paciente* lista, int id_file){
    FILE* file = NULL;
    char file_name[50];
    sprintf(file_name, "arquivo_%d.txt", id_file);
    char buffer[50];
    int iterator = 0;

    if (!(file = fopen(file_name, "r"))){
        printf("Erro de abertura de arquivo\n");
        exit(1);
    }

    while (!(fgets(buffer, sizeof(buffer), file) == NULL)){
        sscanf(buffer, "%s %d", lista[iterator].nome, &lista[iterator].consultorio);
        iterator++;
    }
    fclose(file);
}

void GenerateTabela(){
    printf("\033[2J");
    printf("\033[H");
    for (int i = 0; i < L_TABELA; i++){
        printf("\033[37;4%dmPaciente --- Consultorio %d\n\033[0m", i%8, (i+1));
    }
    printf("\033[%d;1H", L_TABELA);
}

void* WriteTabela(void* file_id){
    int id = *(int*) file_id;
    paciente lista[L];
    Leitura(lista, id+1);

    for (int i = 0; i < L; i++){
        pthread_mutex_lock(&lock[i]);
        printf("\033[%d;1H", lista[i].consultorio);
        printf("\033[2K");
        printf("\033[37;4%dmPaciente %s Consultorio %d\033[0m\n\033[1A", (lista[i].consultorio-1)%8, lista[i].nome, lista[i].consultorio);
        //printf("\033[%d;1H", L_TABELA+1);
        sleep(5);
        pthread_mutex_unlock(&lock[i]);
    }
    pthread_exit(NULL);
}

int main(){
    int iterator = 0;
    pthread_t thread_list[N];
    int file_ids[N];
    int rc;

    InitMutex();
    GenerateTabela();
    
    while (iterator < N){
        file_ids[iterator] = iterator;
        if (rc = pthread_create(&thread_list[iterator], NULL, WriteTabela, (void*) &file_ids[iterator])){
            printf("Erro ao criar threads\n");
            exit(1);
        }
        iterator++;
    }
    
    for (int i = 0; i < N; i++){
        if (rc = pthread_join(thread_list[i], NULL)){
            printf("Erro de espera de thread\n");
            exit(1);
        }
    }

    printf("\033[%d;1H", L_TABELA+2);

    DestroyMutex();
    pthread_exit(NULL);
}