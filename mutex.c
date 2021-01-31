#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 3 //tamanho do buffer

#define N 1// Thr_Produtor
#define M 3// Thr_Cliente

pthread_t  Thr_Produtor[N], Thr_Cliente[M];

typedef struct {
    int buf[BUF_SIZE]; // definindo o buffer
    size_t len; // controla o número de itens do buffer
    pthread_mutex_t mutex; // variável mutex 
    pthread_cond_t can_produce; // variável condicional indicando que item(s) foram removidos
    pthread_cond_t can_consume; // variável condicional indicando que item(s) foram adicionados 
} buffer_t;

void* producer(void *arg) {

  /*Lê os argumentos do buffer_t inicializado no main*/ 
  buffer_t *buffer = (buffer_t*)arg;

  /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
  pthread_mutex_lock(&buffer->mutex);

  /* testa se o buffer está cheio */
  if (buffer->len == BUF_SIZE){printf("Buffer cheio!\n");}
  while(buffer->len == BUF_SIZE) {
    // é bloqueado e espera por um sinal que itens foram removidos
    pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
  }

  // Se o buffer não está cheio, produz o item aleatoriamente 
  int item = rand()%10;
  printf("Produzido: %d\n", item);

  // insere item no buffer 
  buffer->buf[buffer->len] = item;
  ++buffer->len; //incrementa o contador 

  // sinaliza p/ consumidor despertar que um item foi produzido 
  pthread_cond_signal(&buffer->can_consume);
  // desbloqueia o mutex (libera a trava)
  pthread_mutex_unlock(&buffer->mutex);

  return NULL;
}

void* consumer(void *arg) {

  /*Lê os argumentos do buffer_t inicializado no main*/
  buffer_t *buffer = (buffer_t*)arg;

  /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
  pthread_mutex_lock(&buffer->mutex);
  
  /* testa se o buffer está vazio */
  if (buffer->len == 0){printf("Buffer vazio!\n");}
  while(buffer->len == 0) {
    // é bloqueado e espera por um sinal que itens foram produzidos 
    pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
  }

  // decrementa o contador
  --buffer->len;
  printf("Consumido: %d\n", buffer->buf[buffer->len]);

  // sinaliza p/ produtor despertar que um item foi consumido  
  pthread_cond_signal(&buffer->can_produce);

  // desbloqueia o mutex (libera a trava)
  pthread_mutex_unlock(&buffer->mutex);

  return NULL;
}

int main(int argc, char *argv[]) {

  srand((unsigned int)time(NULL));
  // inicializa elementos do buffer_t
  buffer_t buffer = {
    .len = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .can_produce = PTHREAD_COND_INITIALIZER,
    .can_consume = PTHREAD_COND_INITIALIZER
  };

  long i;
    for(i = 0; i < N; i++){
        pthread_create(&Thr_Produtor[i], NULL, producer, (void*)&i); //cria produtor
    }
    for(i = 0; i < M; i++){
         pthread_create(&Thr_Cliente[i], NULL, consumer, (void*)&i); //cria consumidor
    }
    for(i = 0; i < N;i++){
        pthread_join(Thr_Produtor[i], NULL); // espera o thread produtor terminar
    }
    for(i = 0; i < M; i++)
    {
        pthread_join(Thr_Cliente[i], NULL); // espera o thread consumidor terminar
    }

  printf("Programa finalizado com sucesso!!\n");
  return 0;
}
