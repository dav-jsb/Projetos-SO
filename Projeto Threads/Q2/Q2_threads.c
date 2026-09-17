#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 2        //Número de arquivos;
#define L 2        //Número de linhas por arquivo;
#define L_TABELA 7 //Número de linhas da tabela;

pthread_mutex_t lock[L_TABELA];

typedef struct{ 
    char nome[50];
    int consultorio;
} paciente;

typedef struct{
    int file_id; //Identificador do arquivo;
    pthread_t leitora; //Thread que lê o arquivo;
    pthread_t escritora; //Thread que escreve na tabela;
    paciente _lista_pacientes[L];
} le_thread;

void* LerArquivo(void* thread_file){
    char buffer[100];
    FILE* arquivo = NULL;
    char nome_arquivo[50];
    le_thread* leitura = (le_thread*) thread_file;
    int i = 0; // Variável para iterar em cima dos elementos da lista de pacientes;

    sprintf(nome_arquivo, "arquivo_%d.txt\n", leitura->file_id);

    if(!(arquivo = fopen(nome_arquivo, "r"))){ // Checagem de erro caso o arquivo não abra;
        perror("Problema ao abrir o arquivo");
        exit(1);
    }

    while (!((fgets(buffer, sizeof(buffer), arquivo)) == NULL)){ // Loop de leitura de cada uma das linhas dos arquivos;
        if(!(sscanf(buffer, "%s %d", leitura->_lista_pacientes[i].nome, &leitura->_lista_pacientes[i].consultorio))){ // Armazenamento e checagem de erro;
            printf("Formato de linha inválido\n");
        }
        
        i++;
    }

    fclose(arquivo);
    pthread_exit((void*) leitura);

}

void GerarTabela(){
    for (int i = 0; i < L_TABELA; i++){
        printf("\033[37;%dmPaciente --- Consultorio %d\n\033[0m", (i+40), (i+1));
    }
}

void DestroyMutex(){
    for (int i = 0; i < L_TABELA; i++){
        pthread_mutex_destroy(&lock[i]);
    }
}

void InitMutex(){
    int rc;
    
    for (int i = 0; i < L_TABELA; i++){
        rc = pthread_mutex_init(&lock[i], NULL);
        if (rc){
            DestroyMutex();
            printf("Erro ao criar MUTEX\n");
            exit(1);
        }
    }
}

void* Escrita(void* thread_file){
    le_thread* escrita = (le_thread*) thread_file;

    for (int i = 0; i < L; i++){ //L é tanto o número de linhas por arquivo, como também o número de pacientes por arquivo;
        
    }
}

void PrintTabela(le_thread* thread_file_list){
    for (int i = 0; i < N; i++){}
}

int main(int argc, char** argv){
    GerarTabela();
    InitMutex();



    DestroyMutex();
    return 0;
}