#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <signal.h>
#include "log_controller.h"
#include "tcp_controller.h"
#include "tcp_client.h"
#include "types.h"

int printLocker;
pthread_t ptInput, ptTCP;
volatile dadosServidor dados;

void mostraMenu();
void encerrar();

void *handleUserInterrupt(void *unused);
void *threadTCP(void *args);

int main(){
  /* Iniciando data */
  dados.estadoAr = 0;
  dados.temperaturaAr = 0.f;
  dados.estadoLampadas[0] = 0;
  dados.estadoLampadas[1] = 0;
  dados.estadoLampadas[2] = 0;
  dados.estadoLampadas[3] = 0;
  dados.estadoSensoresAbertura[0] = 0;
  dados.estadoSensoresAbertura[1] = 0;
  dados.estadoSensoresAbertura[2] = 0;
  dados.estadoSensoresAbertura[3] = 0;
  dados.estadoSensoresAbertura[4] = 0;
  dados.estadoSensoresAbertura[5] = 0;
  dados.temperatura = 0.f;
  dados.umidade = 0.f;
  dados.requestFlag = 0;
  dados.alarmeOn = 0;
  dados.statusAlarme = 0;
  printLocker = 0;

  /* Setando sinais*/
  signal(SIGINT, encerrar);

  /* Inicializando dispositivos */
  initCSV();
  tcp_setup();

  /* Criando threads */
  pthread_create(&ptTCP, NULL, &threadTCP, NULL);
  pthread_create(&ptInput, NULL, &handleUserInterrupt, NULL);

  while(1){
    if(!printLocker){
      mostraMenu();
    }
    sleep(1);
  }
  return 0;
};

void mostraMenu(){
  struct tm *data_hora_atual;
  time_t segundos;
  time(&segundos);   
  data_hora_atual = localtime(&segundos);
  system("clear");
  printf("---------------- SERVIDOR CENTRAL ----");
  printf("%2d:", data_hora_atual->tm_hour); //hora
  printf("%2d:",data_hora_atual->tm_min);//minuto
  printf("%2d----\n",data_hora_atual->tm_sec);//segundo  printf("--\n");
  printf("Alarme : %s                                   |\n", dados.alarmeOn == 1 ? "ligado   " : "desligado");
  printf("Temperatura atual: %4.2f                             |\nUmidade atual: %4.2f                                 |\n", dados.temperatura, dados.umidade);
  printf("Ar : %s                                       |\n", dados.estadoAr == 1 ? "ligado   " : "desligado");
  printf("Temperatura do Ar : %.2f                             |\n", dados.temperaturaAr);
  for (int i = 0; i < 4; i++){
    printf("Lampada %d: %s                                 |\n", i + 1, dados.estadoLampadas[i] == 1 ? "ligado   " : "desligado");
  }
  for (int i = 0; i < 6; i++){
    printf("Sensor Abertura %d: %s                         |\n", i + 1, dados.estadoSensoresAbertura[i] == 1 ? "ligado   " : "desligado");
  }
  for (int i = 0; i < 2; i++){
    printf("Sensor Presença %d: %s                         |\n", i + 1, dados.estadoSensoresPresenca[i] == 1 ? "ligado   " : "desligado");
  }

  printf("------------------------------------------------------\n");
}

void *threadTCP(void *args){
  tcp_acceptConnection(&dados);
  return NULL;
}

void* handleUserInterrupt(void* unused){
  requisicao req;
  while (1)
  {
    char interrupt, line[20];
    float temp;
    int retry = 0;
    /* Aguardando usuário pressionar enter */
    do{
      interrupt = getchar();
    } while (interrupt != '\n');

    printLocker = 1;
    do{
      system("clear");
      if(retry){
        printf("Inválido.\n\n");
      }
      printf("Escolha uma opção:\n1 - Ligar/Desligar alarme\n2 - Acionar lampadas/ar-condicionado\n");
      fgets(line, sizeof(line), stdin);
      sscanf(line, "%f", &temp);
      retry = 1;
    } while (temp < 0 || temp > 2);
    if (temp == 1){ //
      dados.alarmeOn = dados.alarmeOn == 1 ? 0 : 1;
    }
    else if(temp == 2){
      system("clear");
      printf("Escolha uma opção:\n1 - Ligar ou desligar lampadas\n 2 - Ligar/desligar ar-condicionado\n");
      fgets(line, sizeof(line), stdin);
      sscanf(line, "%f", &temp);
      system("clear");
      if(temp == 1){
        printf("Escolha uma opção:\n");
        for (int i = 0; i < 4; i++)
        {
          printf("%d - Lâmpada %d (%s)\n", i + 1, i + 1, dados.estadoLampadas[i] == 1 ? "ligado" : "desligado");
        }
        fgets(line, sizeof(line), stdin);
        sscanf(line, "%f", &temp);
        req.command = 2;
        req.payload[0] = (int)temp;
        req.payload[1] = dados.estadoLampadas[(int)temp] == 1 ? 0 : 1;
        client_send(req);
      }
      else if(temp == 2){
        if(dados.estadoAr == 1){
          req.command = 1;
          req.payload[0] = 0;
          req.fPayload = 0.f;
          client_send(req);
        }
        else if(dados.estadoAr == 0){
          printf("Insira a temperatuar");
          fgets(line, sizeof(line), stdin);
          sscanf(line, "%f", &req.fPayload);
          req.command = 1;
          req.payload[0] = 1;
          client_send(req);
        }
      }
    }
    printLocker = 0;
    usleep(1000);
    system("clear");
  }
  return NULL;
}

void encerrar(){
  tcp_shutdown();
  exit(0);
}
