// Disciplina: Sistemas Distribuidos para Automacao
//
// Programa-cliente para simular o Sistema de Otimização de carregamento de Vagões - Trabalho Final - 2020/2
//
// Maria Luiza de Andrade Alves e Matheus Paiva Loures

#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS //Uso necessário devido a função inet_addr
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT  0x0400	// Necessário para ativar novas funções da versão 4


#include <windows.h>
#include <process.h>	
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <winsock2.h>
#include <ctype.h>
#include <iostream>
#include<string>
#pragma comment (lib, "ws2_32.lib") //Inclui a biblioteca winsocket 

using namespace std;
using std::string;


// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

#define	ESC 	0x1B	//Tecla para encerrar o programa
#define p       0x70    // DEfine tecla do evento p
//----------Tamanho das mensagens-----------//
#define TAMSTATUS 37  // 6+2+6+6+6+6 caracteres + 5 separadores
#define TAMACK     9  // 6+2 caracteres + 1 separador
#define TAMREQ     9  // 6+2 caracteres + 1 separador
#define TAMPOS    41  // 6+2+6+5+5+5+6 caracteres + 6 separadores

//-----Variáveis Globais----//
int nseq = 1;

//-----Estruturas 
typedef struct TIPO11 {
	int nseq = 1;
	int tipo = 11;
	int taxa = 0;
	float potencia = 0.0;
	float tempTrans = 0.0;
	float tempRoda = 0.0;

}TIPO11; // definição do tipo 11

typedef struct TIPO33 {
	int nseq = 1;
	int tipo = 33;
}TIPO33; // definição do tipo 33

typedef struct TIPO99 {
	int nseq = 1;
	int tipo = 99;
}TIPO99; // definição do tipo 99
// Funções de Criação das Mensagens
TIPO11  novaMensagem11();
TIPO33  novaMensagem33();
TIPO99  novaMensagem99();

int main(int argc, char **argv)
{
    //Variáveis socket//
    WSADATA wsaData;
    SOCKET s;
	int statusSocket, port;
	char* ipaddr;
    SOCKADDR_IN ServerAddr;

    //Variáveis do Temporizador//
    HANDLE hTimer;
    LARGE_INTEGER Present;
	// Verifia se o que foi passado na linha de comando está correto
	if (argc != 3) {
		printf("Valores inválidos, reinicie o cliente...\n");
		exit(0);
	}
	ipaddr = argv[1];
	port = atoi(argv[2]);

	//Criar Temporizador

	// Inicializa Winsock versão 2.2
	statusSocket = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (statusSocket != 0) {
		printf("Falha na inicializacao do Winsock 2! Erro  = %d\n", WSAGetLastError());
		WSACleanup();
		exit(0);
	}
	// Estrutura SOCKADDR_IN
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(port);
	ServerAddr.sin_addr.s_addr = inet_addr(ipaddr);

	while (true) {
		// Criação do Socket
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//Mensagens de erro
		if (s == INVALID_SOCKET) {
			statusSocket = WSAGetLastError();
			if (statusSocket == WSAENETDOWN) {
				printf("Rede ou servidor de sockets inacessíveis!\n");
			}
			else {
				printf("Codigo de erro = %d\n", statusSocket);
			}
			WSACleanup();
			exit(0);
		}
		// Estabelece a conexão com o servidor
		printf("Iniciando conexão com o Sistema de Mapeamento 3D...\n");
		statusSocket = connect(s, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
		if (statusSocket == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEHOSTUNREACH) {
				/*//printf("Rede inacessivel... aguardando 5s e tentando novamente\n");
				else {
					Sleep(5000);
					continue;
				}*/
			}
			else {
				printf("Falha na conexao ao servidor ! Erro  = %d\n", WSAGetLastError());
				WSACleanup();
				closesocket(s);
				exit(0);
			}
		}
	}
    
    return(0);
}

