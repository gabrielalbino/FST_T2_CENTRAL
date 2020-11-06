#include "tcp_controller.h"

int servidorSocket;
int socketCliente;
struct sockaddr_in servidorAddr;
struct sockaddr_in clienteAddr;
unsigned int clienteLength;

void tcp_setup(){
  if((servidorSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		printf("falha no socker do Servidor\n");

	// Montar a estrutura sockaddr_in
	memset(&servidorAddr, 0, sizeof(servidorAddr)); // Zerando a estrutura de dados
	servidorAddr.sin_family = AF_INET;
	servidorAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servidorAddr.sin_port = htons(10116);

	// Bind
	if(bind(servidorSocket, (struct sockaddr *) &servidorAddr, sizeof(servidorAddr)) < 0)
		printf("Falha no Bind\n");

	// Listen
	if(listen(servidorSocket, 10) < 0)
		printf("Falha no Listen\n");		
}

void tcp_shutdown(){
  close(servidorSocket);
}

void tcp_acceptConnection(volatile dadosServidor* dados){
  while(1) {
    clienteLength = sizeof(clienteAddr);
    if((socketCliente = accept(servidorSocket, (struct sockaddr *) &clienteAddr, &clienteLength)) >= 0){
      TrataClienteTCP(socketCliente, dados);
      close(socketCliente);
    }
  }
}
void TrataClienteTCP(int socketCliente, volatile dadosServidor *dados){
  requisicao req;
  requisicao response;
  int tamanhoRecebido;
  if((tamanhoRecebido = recv(socketCliente, &req, sizeof(req), 0)) < 0)
    return;

  switch(req.command){
    case 1: //alarme
      dados->statusAlarme = req.payload[0];
      if (dados->statusAlarme == 1 && dados->alarmeOn == 1){
        printf("PIU PIU PIU");
        saveInFile("Alarme disparado");
      }
      else if(dados->alarmeOn == 1)
        saveInFile("Sensor voltou ao estado desativado");
      break;
    case 2: // sensor 1 mudou
      response.command = 2;
      response.payload[0] = 1;
      response.payload[1] = req.payload[0];
      client_send(response);
      saveInFile("Acendendo luz da sala");
      break;
    case 3: // sensor 2 mudou
      response.command = 2;
      response.payload[0] = 0;
      response.payload[1] = req.payload[0];
      client_send(response);
      saveInFile("Acendendo luz da cozinha");
      break;
    case 4: //atualizar dados
      *dados = req.dadosServidor;
    }
}
