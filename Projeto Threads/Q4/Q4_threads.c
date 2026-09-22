#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

//Em resumo, nossa implementação utilizou-se de um escalonador para definir que variáveis cada thread ficaria responsável;
//Além disso, utilizamos as variáveis de barreira para obrigar que os valores das variáveis só fossem atualizados sincronamente (permitindo o pleno funcionamento do método de Jacobi)
//Dentro da implementação de Jacobi, também utilizamos de uma array que armazenaria os valores temporários de cada um dos resultados de cada iteração (só atualizando sincronamente);

#define N 10 //Número de threads;
#define I 4 //Número de incógnitas;
#define P 10 //Número de iterações;

typedef struct{
    int pos_init; //Posicão inicial na array de variáveis;
    int partition; //Quantidade de variáveis por thread;
} thread_data; //Struct que contém a informação de quais variáveis cada thread vai ficar responsável;

pthread_barrier_t barrier; //Variável de barreira para implementação do algoritmo de Jacobi;
//Sistema linear representado abaixo;
float coeficients[I][I] = {{10, -1, 2, 0}, {-1, 11, -1 , 3}, {2, -1, 10, -1}, {0, 3, -1, 8}};
float variable[I];
float independent[I] = {6, 25, -11, 15};

//Função para inicializar barreira conforme a quantidade de threads utilizadas;
void InitBarrier(int count, pthread_barrier_t* barrier_var){
    int rc;
    if (rc = pthread_barrier_init(barrier_var, NULL, count)){
        printf("Erro ao inicializar barrier\n");
        exit(1);
    }
}

//Função para destruir a variável de barreira;
void DestroyBarrier(pthread_barrier_t* barrier_var){
    pthread_barrier_destroy(barrier_var);
}

int Scheduler(int* schedule){ //Escalonador responsável por definir com quais variáveis cada thread vai ficar responsável e retornar quantas threads serão utilizadas;
    int remainder = I%N;
    int partition = I/N;
    int count = N;

    for (int i = 0; i < I; i++){
        schedule[i] = partition; //Divide a partição para cada uma das threads e depois distribui o resto;
        if (remainder > 0){ 
            schedule[i]++;
            remainder--;
        }
    }

    if (!partition) count = I;//O escalonador também define quantas threads serão necessárias para a resolução do sistema;
    return count;
}

void SetVariable(){ //Função para setar toda a array de variáveis para 1;
    for (int i = 0; i < I; i++) variable[i] = 1;
}

void* Jacobi(void* thread_file){//Algoritmo de Jacobi implementado;
    thread_data data = *(thread_data*) thread_file;
    int iter = P; //Variável que armazena o número de iterações;
    float sum_ax; //Soma de aij*xj com j diferente de i;
    int pos_init = data.pos_init;
    int partition = data.partition;
    float temp_variable[partition]; //Array que armazena os resultados de cada iteração até que possa ser atualizado na array global;
    
    while (iter > 0){
        for (int i = 0; i < partition; i++){
            sum_ax = 0;
            for (int j = 0; j < I; j++){
                if (j != (pos_init + i)) sum_ax += coeficients[pos_init + i][j] * variable[j]; //Cálculo do sum_ax;
            }
            temp_variable[i] = (independent[pos_init + i] - sum_ax) / coeficients[pos_init+i][pos_init+i]; //Resultado para a atual iteração do valor de xi;
        }
        iter--;
        pthread_barrier_wait(&barrier); //Variável de barreira para aguardar que todas as variáveis tenham calculado sua iteração;
        for (int i = 0; i < partition; i++){
            variable[pos_init + i] = temp_variable[i]; //Mudança da variável na array global;
        }
        pthread_barrier_wait(&barrier); //Variável de barreira para aguardar que todas as threads tenham alterado a array global em determinada iteração;
    }
    
    pthread_exit(NULL);
}

int main(){
    thread_data data_list[N]; //Lista de informações definidoras das variáveis para cada thread;
    pthread_t thread_list[N]; //Lista de threads propriamente ditas;
    int schedule[N];
    int count, rc, jc;
    int pos = 0;
    
    SetVariable();
    count = Scheduler(schedule); 
    InitBarrier(count, &barrier); //Definição da quantidade de threads com base no count do escalonador;
    
    for (int i = 0; i < count; i++){ //Criação de cada thread;
        data_list[i].partition = schedule[i];
        data_list[i].pos_init = pos;
        pos += data_list[i].partition;

        if (rc = pthread_create(&thread_list[i], NULL, Jacobi, (void*) &data_list[i])){
            printf("Erro na criacao da thread\n");
            exit(1);
        }

    }

    for (int i = 0; i < count; i++){ //Loop responsável por aguardar o término de cada thread;
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

    //Destruição da variável de barreira e liberação das threads;
    DestroyBarrier(&barrier);
    pthread_exit(NULL);
}