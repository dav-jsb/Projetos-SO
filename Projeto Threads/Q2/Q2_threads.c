#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

//Nosso algoritmo se baseou na criação de uma função simples de leitura que retornaria uma lista de pacientes para cada thread, a qual imprimiria na tabela esses pacientes;
//Utilizamos das variáveis mutex para que cada linha fosse travada por 5 segundos para escrita e demonstração do paciente na tabela;
//Utilizamos a função sleep da unistd para definir esse tempo de espera;

#define N 5 //Número de arquivos e threads;
#define L 3 //Número de linhas;
#define L_TABELA 7 //Número de linhas da tabela;
#define SLEEP 5 //Tempo de exibição da linha na tabela;

typedef struct{ //Struct responável por armazenar o nome e o consultório de cada pessoa lida no arquivo;
    char nome[50];
    int consultorio;
}paciente;

pthread_mutex_t lock[L_TABELA]; //Array de mutex para travamento de cada linha da tabela;

void InitMutex(){//Função de inicialização dos Mutex;
    for (int i = 0; i < L_TABELA; i++){
        if (pthread_mutex_init(&lock[i], NULL)){
            printf("Erro ao inicializar mutex\n");
            exit(1);
        }
    }
}

void DestroyMutex(){//Função de destruição dos mutex;
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

    while (!(fgets(buffer, sizeof(buffer), file) == NULL)){ //Aqui, utilizamos o fgets para ler linha por linha e depois o sscanf para extrair os valores desejados (considerando principalmente que a entrada é amigável);
        sscanf(buffer, "%s %d", lista[iterator].nome, &lista[iterator].consultorio);
        iterator++;
    }
    fclose(file);
}

void GenerateTabela(){//Aqui, limpamos a tela, redirecionamos o ponteiro para o início do terminal e depois printamos a linha base (paciente --- consultório x);
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
    Leitura(lista, id+1); //Passamos a lista de paciente para a função de leitura para que possamos ter a lista de pacientes do arquivo;

    for (int i = 0; i < L; i++){
        pthread_mutex_lock(&lock[i]); //Utilizamos o mutex para travar a linha que necessitamos;
        printf("\033[%d;1H", lista[i].consultorio);
        printf("\033[2K");
        printf("\033[37;4%dmPaciente %s Consultorio %d\033[0m\n\033[1A", (lista[i].consultorio-1)%8, lista[i].nome, lista[i].consultorio); //Print ajustado de cada paciente na tabela;
        //printf("\033[%d;1H", L_TABELA+1);
        sleep(SLEEP); //Sleep de 5 segundos para visualização do paciente na tabela;
        pthread_mutex_unlock(&lock[i]); //Liberação da linha utilizada;
    }
    pthread_exit(NULL);
}

int main(){
    int iterator = 0;
    pthread_t thread_list[N];
    int file_ids[N];
    int rc;

    InitMutex(); //Inicialização dos mutex;
    GenerateTabela(); //Geração da tabela base;
    
    while (iterator < N){
        file_ids[iterator] = iterator; //Definição dos ids de cada arquivo;
        if (rc = pthread_create(&thread_list[iterator], NULL, WriteTabela, (void*) &file_ids[iterator])){
            printf("Erro ao criar threads\n");
            exit(1);
        }
        iterator++;
    }
    
    for (int i = 0; i < N; i++){ //Utilização do join para aguardar que todas as threads terminem suas funções;
        if (rc = pthread_join(thread_list[i], NULL)){
            printf("Erro de espera de thread\n");
            exit(1);
        }
    }

    printf("\033[%d;1H", L_TABELA+2); //Mover o ponteiro para 2 espaços abaixo da tabela;

    DestroyMutex(); //Destruição dos mutex;
    pthread_exit(NULL); //Liberação das threads;
}