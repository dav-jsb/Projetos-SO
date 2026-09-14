#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 2 //Número de arquivos;
#define L 2 //Número de linhas por arquivo;

typedef struct{
    pthread_t leitora; //Thread que lê o arquivo;
    pthread_t escritora; //Thread que escreve na tabela;
    int file_id; //Identificador do arquivo;
    paciente _lista_pacientes[L];
} le_thread;

typedef struct{ 
    char nome[50];
    int consultorio;
} paciente;

void* LerArquivo(void* thread_file){
    paciente _lista_pacientes[L];
    FILE* arquivo = NULL;
    char nome_arquivo[50];
    le_thread* leitura = (le_thread*) thread_file;
    char buffer[100];
    int i = 0; // Variável para iterar em cima dos elementos da lista de pacientes;

    sprintf(nome_arquivo, "arquivo_%d.txt\n", leitura->file_id);

    if(!(arquivo = fopen(nome_arquivo, "r"))){ // Checagem de erro caso o arquivo não abra;
        perror("Problema ao abrir o arquivo");
        exit(1);
    }

    while (!((fgets(buffer, sizeof(buffer), arquivo)) == NULL)){ // Loop de leitura de cada uma das linhas dos arquivos;
        if(!(sscanf(buffer, "%s %d", leitura->_lista_pacientes[i].nome, leitura->_lista_pacientes[i].consultorio))){ // Armazenamento e checagem de erro;
            printf("Formato de linha inválido\n");
        }
        
        i++;
    }

    fclose(arquivo);
    pthread_exit((void*) leitura);

}

void* PrintTabela(void* thread_file){

    
}

int main(int argc, char** argv){


    return 0;
}