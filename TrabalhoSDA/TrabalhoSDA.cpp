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
#define TAMPOS    41  // 6+2+6+5+5+5+6 caracteres + 6 separadores/



//-----Variáveis Globais----//
int nseq = 1, Tecla=0;
HANDLE hEventoESC, hEventoP;


// Funções de Criação das Mensagens
char*  novaMensagem11(int* nseq);
char*  novaMensagem33(int* nseq);
char*  novaMensagem99(int* nseq);

//Threads
DWORD WINAPI ThreadTeclado(LPVOID);
int main(int argc, char **argv)
{	
	SetConsoleTitle(L"Aplicação de Software - Cliente Socket");
    //Variáveis socket//
    WSADATA wsaData;
    SOCKET s;
	int status,statusSocket, port;
	char* ipaddr;
    SOCKADDR_IN ServerAddr;
	//Variáveis Threads Secundárias
	HANDLE hThreadTeclado;
	DWORD dwThreadTeclado;
	DWORD dwExitCode = 0;

	// Criação de Threads
	hThreadTeclado = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)ThreadTeclado, NULL, 0, (CAST_LPDWORD)&dwThreadTeclado);
	if (hThreadTeclado) 	cout << "Thread de leitura do teclado criada com Id=" << dwThreadTeclado << "\n";

	//Variáveis do Temporizador//
    HANDLE hTimer;
    LARGE_INTEGER Present;

	hTimer= CreateWaitableTimer(NULL, FALSE, L"Timer");
	Present.QuadPart = -(10000 * 200);
	status = SetWaitableTimer(hTimer, &Present, 500, NULL, NULL, FALSE);

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

DWORD WINAPI ThreadTeclado(LPVOID index) {
	// Eventos
	hEventoESC = CreateEvent(NULL, TRUE, FALSE, L"EventoESC"); // reset manual
	hEventoP = CreateEvent(NULL, FALSE, FALSE, L"EventoP"); // reset automatico
	int status; 
	do {
		cout << "\n Tecle <p> para simular o evento de solitacao de mensagem \n <ESC> para sair \n";
		Tecla = _getch();

		if (Tecla == p) {
			status = SetEvent(hEventoP);
			cout << "\n Evento P ocorreu \n";
			Tecla = 0;
		}
		else if (Tecla == ESC) {
			status = SetEvent(hEventoESC);
			cout << "\n Evento ESC ocorreu \n";
		}
		else {
			cout << "\n Evento nao cadastrado \n";
			Tecla = 0;
		}


	} while (Tecla != ESC);
	_endthreadex((DWORD)index);
	return(0);

}


char* novaMensagem11(int* nseq) {
	string msg;
	char parte[5];
	char texto[TAMSTATUS] = "      ";
	int aux = rand() % 999999;

	sprintf(parte, "%05d", *nseq);
	msg = parte;
	msg += "$";
	msg += to_string(11) + "$";
	msg += to_string(aux) + "$";
	aux = rand() % 9999;
	msg += to_string((float)aux / 10) + "$";
	aux = rand() % 9999;
	msg += to_string((float)aux / 10) + "$";
	aux = rand() % 9999;
	msg += to_string((float)aux / 10);

	*nseq++;
	if (*nseq == 99999) {
		*nseq = 1;
	}


	strcpy(texto, msg.c_str());

	return texto;
}

char* novaMensagem33(int* nseq) {
	string msg;
	char parte[5];
	char texto[TAMREQ];
	int j;

	sprintf(parte, "%05d", *nseq);
	msg = parte;
	msg += "$";
	msg += to_string(33);

	*nseq++;

	if (*nseq == 99999) {
		*nseq = 1;
	}

	strcpy(texto, msg.c_str());





	return texto;




}

char* novaMensagem99(int* nseq) {
	string msg;
	char parte[5];
	char texto[TAMREQ];
	int j;

	sprintf(parte, "%05d", *nseq);
	msg = parte;
	msg += "$";
	msg += to_string(99);

	*nseq++;

	if (*nseq == 99999) {
		*nseq = 1;
	}

	strcpy(texto, msg.c_str());


	return texto;


}

