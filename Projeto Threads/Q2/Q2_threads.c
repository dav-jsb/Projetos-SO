#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define N 5        //Número de arquivos e número de threads;
#define L 5        //Número de linhas por arquivo;
#define L_TABELA 7 //Número de linhas da tabela;

pthread_mutex_t lock[L_TABELA];

typedef struct{ 
    char nome[50];
    int consultorio;
} paciente;

typedef struct{
    int file_id; //Identificador do arquivo;
    paciente _lista_pacientes[L];
} leitura_thread;

typedef struct{
    pthread_t leitora;
    pthread_t escritora;
} le_thread;

void InitMutex(){
    int rc;
    
    for (int i = 0; i < L_TABELA; i++){
        rc = pthread_mutex_init(&lock[i], NULL);
        if (rc){
            printf("Erro ao criar MUTEX\n");
            exit(1);
        }
    }
}

void DestroyMutex(){
    for (int i = 0; i < L_TABELA; i++){
        pthread_mutex_destroy(&lock[i]);
    }
}

void GerarTabela(){
    printf("\033[2J");
    printf("\033[H");
    for (int i = 0; i < L_TABELA; i++){
        printf("\033[37;4%dmPaciente --- Consultorio %d\n\033[0m", i, (i+1));
    }
}

void* LerArquivo(void* thread_file){
    int i = 0; // Variável para iterar em cima dos elementos da lista de pacientes;
    char buffer[100];
    FILE* arquivo = NULL;
    char nome_arquivo[50];

    leitura_thread* leitura = malloc(sizeof(leitura_thread));
    if (leitura == NULL){
        printf("Erro de alocacao de memoria para alocacao do arquivo\n");
        exit(1);
    }

    leitura->file_id = *(int*) thread_file;

    sprintf(nome_arquivo, "arquivo_%d.txt", leitura->file_id+1);

    if(!(arquivo = fopen(nome_arquivo, "r"))){ // Checagem de erro caso o arquivo não abra;
        printf("Problema ao abrir o arquivo %d\n", leitura->file_id+1);
        exit(1);
    }

    while (!((fgets(buffer, sizeof(buffer), arquivo)) == NULL)){ // Loop de leitura de cada uma das linhas dos arquivos;
        sscanf(buffer, "%s %d", leitura->_lista_pacientes[i].nome, &leitura->_lista_pacientes[i].consultorio);
        i++;
    }

    fclose(arquivo);
    pthread_exit((void*) leitura);
}

void* Escrita(void* thread_leitora){
    int rc;
    int consultorio;
    int mutex_index;

    leitura_thread* resultado_leitura = NULL;

    pthread_t* leitora = (pthread_t*) thread_leitora;
    
    rc = pthread_join(*leitora, (void**) &resultado_leitura);
    if (rc){
        printf("Erro de join da thread de escrita\n");
        exit(1);
    }

    for (int i = 0; i < L; i++){ //L é tanto o número de linhas por arquivo, como também o número de pacientes por arquivo;
        consultorio = resultado_leitura->_lista_pacientes[i].consultorio;
        mutex_index = consultorio - 1;
        
        pthread_mutex_lock(&lock[mutex_index]);
        printf("\033[%d;1H\033[2K", consultorio);
        printf("\033[37;4%dmPaciente %s Consultorio %d\033[0m\033[%d;1H\n",(consultorio-1)%8, resultado_leitura->_lista_pacientes[i].nome, consultorio, L_TABELA+2);
        sleep(5);
        pthread_mutex_unlock(&lock[mutex_index]);
    }
    printf("\033[8;1H");

    free(resultado_leitura);
    pthread_exit(NULL);
}

int main(){
    int rc;
    int indices[N];
    le_thread le_thread_list[N];
    GerarTabela();
    InitMutex();

    for (int i = 0; i < N; i++){
        indices[i] = i;
        rc = pthread_create(&le_thread_list[i].leitora, NULL, LerArquivo, (void*) &indices[i]);
        
        if (rc){
            printf("Erro na criação da thread de leitura %d\n", i+1);
            exit(1);
        }

        rc = pthread_create(&le_thread_list[i].escritora, NULL, Escrita, (void*) &le_thread_list[i].leitora);

        if (rc){
            printf("Erro na criacao da thread de escrita %d\n", i+1);
            exit(1);
        }
    }

    for (int i = 0; i < N; i++) {
        pthread_join(le_thread_list[i].escritora, NULL);
    }

    DestroyMutex();
    return 0;
}