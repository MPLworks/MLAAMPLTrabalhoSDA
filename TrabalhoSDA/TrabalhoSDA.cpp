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
#include "opcclient.h"
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
int nseq = 0, Tecla=0;
HANDLE hEventoESC, hEventoP;
//Variaveis timer
HANDLE hTimer;
LARGE_INTEGER Present;

VARIANT varValueW[5]; //to store the write value
VARIANT varValueR[4]; //to store the read value

// Funções de Criação das Mensagens
DWORD WINAPI ThreadTeclado(LPVOID index);

DWORD WINAPI OPCClient (LPVOID index);

int main(int argc, char **argv)
{	
	SetConsoleTitle(L"Aplicacao de Software - Cliente Socket");

	//Inicializa variaves de escrita no OPC
	VariantInit(&varValueW[0]); //  4 -  Velocidade de translação (cm/s) 
	VariantInit(&varValueW[1]); //  5 -  Coordenada espacial X do ponto de ataque (cm) 
	VariantInit(&varValueW[2]); //  6 -  Coordenada espacial Y do ponto de ataque (cm)
	VariantInit(&varValueW[3]); //  7 -  Coordenada espacial Z do ponto de ataque (cm)
	VariantInit(&varValueW[4]); //  8 -  Taxa de recuperação de minério (kg/min)

    //Variáveis socket
    WSADATA wsaData;
    SOCKET s;
	int status,statusSocket, port;
	char* ipaddr;
    SOCKADDR_IN ServerAddr;
	int nseqaux;

	//Variáveis Threads Secundárias
	int j;
	HANDLE hThread[2];
	DWORD dwRet;
	DWORD dwThreadTeclado, dwThreadOPC;
	DWORD dwExitCode = 0;

	//Evento ACK
	HANDLE hACK99;
	hACK99 = CreateEvent(NULL, FALSE, FALSE, L"EventoACK99");

	// Criação de Threads
	hThread[0]= (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)ThreadTeclado, NULL, 0, (CAST_LPDWORD)&dwThreadTeclado);
	if (hThread[0]) 	cout << "Thread de leitura do teclado criada com Id=" << dwThreadTeclado << "\n";

	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)OPCClient, NULL, 0, (CAST_LPDWORD)&dwThreadOPC);
	if (hThread[1]) 	cout << "Thread do OPC Client criada com Id=" << dwThreadOPC << "\n";

	//Crianção dos Eventos de encerramento e de solicitação 
	hEventoESC = CreateEvent(NULL, TRUE, FALSE, L"EventoESC"); // reset manual
	hEventoP = CreateEvent(NULL, FALSE, FALSE, L"EventoP"); // reset automatico

	//Criação do Temporizador
	hTimer=CreateWaitableTimer(NULL, FALSE, L"Timer");
	Present.QuadPart = -(10000 * 200);
	status = SetWaitableTimer(hTimer, &Present, 500, NULL, NULL, FALSE);


	//Criação das mensagens
	char msgstatus[TAMSTATUS + 1];
	char msgack99[TAMACK + 1] = "NNNNNN$99";
	char msgreq[TAMREQ + 1] = "NNNNNN$33";
	char msgpos[TAMPOS + 1];
	char msgack22[TAMACK + 1];
	char buf[100];

	// Verifica se o que foi passado na linha de comando está correto
	if (argc != 3) {
		printf("User: TrabalhoSDA <IP> <port>\n");
		exit(0);
	}
	else {
		ipaddr = argv[1];
		port = atoi(argv[2]);
	}

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

	DWORD ret;
    int encerramento = 1;
	HANDLE hEvento = hEventoESC;

	/*----------------------------------------------------------------------------------
									Loop da conexão
	------------------------------------------------------------------------------------*/
	do{
		
		ret = WaitForSingleObject(hEvento, 100);
		encerramento = ret - WAIT_OBJECT_0;
		

		if (encerramento == 0) {
			break;
		}
		else {
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
			printf("Iniciando conexao com o Sistema de Mapeamento 3D...\n");
			statusSocket = connect(s, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
			if (statusSocket == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAEHOSTUNREACH) {
					printf("Rede inacessivel... aguardando 5s e tentando novamente\n");
					/*else {
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
			else {
				cout << "Conexao com o Sistema de Mapeamento 3D (servidor) efetuada com sucesso" << endl;
			}

			int tipo = 4;
			/*----------------------------------------------------------------------------------
											Loop da troca de mensagens
			------------------------------------------------------------------------------------*/
			do {


				HANDLE hEventos[4];
				hEventos[0] = hTimer;
				hEventos[1] = hEventoP;
				hEventos[2] = hACK99;
				hEventos[3] = hEventoESC;

				//ESPERA PELOS TIPOS DE MENSAGEM DE ACORDO COM OS EVENTOS
				ret = WaitForMultipleObjects(4, hEventos, FALSE, INFINITE);
				tipo = ret - WAIT_OBJECT_0;
				//Envio da mensagem tipo 11 e recebimento da mensagem tipo 22
				if (tipo == 0) { 
					nseq++;
					if (nseq == 99999) {
						nseq = 1;
					}
					/*
					 0 -  Taxa de recuperação de minério (kg/min)         - Random.UInt1             - leitura
					 1 -  Potência atual consumida (kW)                   - Random.Real              - leitura
					 2 -  Temperatura motor de translação (C)             - Saw-toothed Waves.Real4  - leitura
					 3 -  Temperatura motor da roda de caçambas (C)       - Square Waves.Real4       - leitura

					*/
					
					string msg;
					char parte[6];

					//Formatação da mensagem 11 de acordo com os dados que vieram do OPC 
					memset(parte, 0, sizeof(parte));
					sprintf(parte, "%06d", nseq);
					for (int j = 0; j < 6; j++)
						msg += parte[j];
					msg += '$';

					msg += "11$";
					memset(parte, 0, sizeof(parte));
					sprintf(parte, "%06d", varValueR[0].intVal);
					for (int j = 0; j < 6; j++)
						msg += parte[j];
					msg += '$';

					memset(parte, 0, sizeof(parte));
					sprintf(parte, "%04.2f", varValueR[1].fltVal);
					for (int j = 0; j < 6; j++)
						msg += parte[j];
					msg += '$';

					memset(parte, 0, sizeof(parte));
					sprintf(parte, "%04.2f", varValueR[2].fltVal);
					for (int j = 0; j < 6; j++)
						msg += parte[j];
					msg += '$';;
					memset(parte, 0, sizeof(parte));
					sprintf(parte, "%04.2f", varValueR[3].fltVal);
					for (int j = 0; j < 6; j++)
						msg += parte[j];

					//Colocando a msg dentro da msgstatus
					for (int j = 0; j < TAMSTATUS + 1; j++)
						msgstatus[j] = msg[j];

			
					//Envio da msg 11 
					statusSocket = send(s, msgstatus, TAMSTATUS, 0);
					if (statusSocket == TAMSTATUS) {
						printf("Mensagem de Status enviada ao Sist. de mapeamento 3D: \n%s\n", msgstatus);
					}
					else {
						if (statusSocket == 0)
							cout << "Algum tipo de erro inesperado ocorreu" << endl;
						else if (statusSocket == SOCKET_ERROR) {
							statusSocket = WSAGetLastError();
							printf("Falha no envio da mensagem tipo 11 codigo de erro = %d\n", statusSocket);
						}
						printf("Tentando reconexão ..\n\n");//CONFERIR TENTATIVA DE RECONEXÃO
						break;
					}

					//Aguarda recebimento do ACK22
					memset(buf, 0, sizeof(buf));
					statusSocket = recv(s, buf, TAMACK, 0);
					sscanf(buf, "%6d", &nseqaux);
					if (++nseq != nseqaux) {
						printf("Numero sequencial de mensagem incorreto [1]: recebido %d ao inves de %d\n", nseqaux, nseq++);
						closesocket(s);
						WSACleanup();
						exit(0);
					}
					else {
						nseq = nseqaux;
						if (strncmp(&buf[7], "22", 2) == 0) {
							strncpy(msgack22, buf, TAMACK + 1);
							printf("Mensagem de ACK tipo 22 recebida do Sist. de mapeamento 3D:\n%s\n", msgack22);
						}
						else {
							printf("Codigo incorreto do recebimento do ACK 22, código recebido foi %s\n", &buf[7]);
							printf("Encerrando o programa...\n");
							closesocket(s);
							WSACleanup();
							exit(0);
						}
					}

				}
				// Envio da mensagem 33 recebimento da mensagem 55 e setar evento p/ ACK99
				else if (tipo == 1) {
					nseq++;
					if (nseq == 99999) {
						nseq = 1;
					}
					//Formatação da mensagem p/ o envio
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "%06d", nseq);
					memcpy(msgreq, buf, 6);
					statusSocket = send(s, msgreq, TAMREQ, 0);
					//Verificar status e printar na tela
					if (statusSocket == TAMREQ) {
						printf("Mensagem de requisicao enviada ao Sist. de mapeamento 3D: \n%s\n", msgreq);
					}
					else {
						if (statusSocket == 0)
							cout << "Algum tipo de erro inesperado ocorreu" << endl;
						else if (statusSocket == SOCKET_ERROR) {
							statusSocket = WSAGetLastError();
							printf("Falha no envio da mensagem tipo 22 codigo de erro = %d\n", statusSocket);
						}
						printf("Tentando reconexão ..\n\n");//CONFERIR TENTATIVA DE RECONEXÃO
						break;
					}
					//Recebendo msg 55
					statusSocket = recv(s, buf, TAMPOS, 0);
					sscanf(buf, "%6d", &nseqaux);
					if (++nseq != nseqaux) {

						printf("Numero sequencial de mensagem incorreto [1]: recebido %d ao inves de %d\n", nseqaux, nseq);
						closesocket(s);
						WSACleanup();
						exit(0);
					}
					else {
						if (strncmp(&buf[7], "55", 2) == 0) {
							nseq = nseqaux;
							strncpy(msgpos, buf, TAMPOS + 1);
							printf("Mensagem de posicionamento recebida do ao Sist. de mapeamento 3D: \n%s\n", msgpos);

							string escreve = msgpos;

							//0 0 6 7 3 3 $ 5 5 $ 0  0  0  1  .  0  $  0  0  8  0  0  $  0  0  2  0  5  $  0  0  7  5  0  $  0  8  0  0  .  0
							//0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40



							varValueW[0].fltVal = stof(escreve.substr(10, 6));
							varValueW[1].intVal = stoi(escreve.substr(17, 5));
							varValueW[2].intVal = stoi(escreve.substr(23, 5));
							varValueW[3].intVal = stoi(escreve.substr(29, 5));
							varValueW[4].fltVal = stof(escreve.substr(35, 6));


							SetEvent(hACK99);
						}
						else {
							printf("Codigo incorreto do recebimento da msg tipo 55, código recebido foi %s\n", &buf[7]);
							printf("Encerrando o programa...\n");
						}
					}
				}
				//Envio do ACK99
				else if (tipo == 2) {
					nseq++;
					if (nseq == 99999) {
						nseq = 1;
					}
					//Formatação da mensagem p/ o envio
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "%06d", nseq);
					memcpy(msgack99, buf, 6);
					statusSocket = send(s, msgack99, TAMACK, 0);
					//Verificar status e printar na tela
					if (statusSocket == TAMACK) {
						printf("Mensagem de ACK tipo 99 enviada do Sist. de mapeamento 3D:: \n%s\n", msgack99);
					}
					else {
						if (statusSocket == 0)
							cout << "Algum tipo de erro inesperado ocorreu" << endl;
						else if (statusSocket == SOCKET_ERROR) {
							statusSocket = WSAGetLastError();
							printf("Falha no envio da mensagem tipo 99 codigo de erro = %d\n", statusSocket);
						}
						printf("Tentando reconexão ..\n\n");//CONFERIR TENTATIVA DE RECONEXÃO
						break;
					}

				}
				//Evento ESC
				else if (tipo == 3) {
					break;
				}

			} while (tipo != 3);
		}

	} while (encerramento != 0);

	printf("Finalizando conexão....\n");
	closesocket(s);
	WSACleanup();

	// Encerrando as threads 
	dwRet = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	//CheckForError((dwRet >= WAIT_OBJECT_0)&& (dwRet < WAIT_OBJECT_0 + 5));
	for (j = 0; j < 2; j++) {
		status = GetExitCodeThread(&hThread[j], &dwExitCode);
		cout << "thread " << j << " terminou: codigo " << dwExitCode << "\n";
		CloseHandle(hThread[j]);	// apaga referência ao objeto
	}  // for 


	
	//Fechar Handles
	CloseHandle(hTimer);
	CloseHandle(hACK99);
	CloseHandle(hEventoP);
	CloseHandle(hEventoESC);
	

	
   

    return(0);
}

DWORD WINAPI ThreadTeclado(LPVOID index) {


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
			Sleep(1000);
		}
		else {
			cout << "\n Evento nao cadastrado \n";
			Tecla = 0;
		}


	} while (Tecla != ESC);
	cout << "Thread leitura do teclado encerrando...\n";

	_endthreadex((DWORD)index);
	return(0);

}

DWORD WINAPI OPCClient(LPVOID index) {

	opcInit();

	do {

		for (int i = 0; i < 4; i++) {
			varValueR[i] = opcClient(varValueW,i);
		}

	}while (Tecla != ESC);

	cout << "Thread OPC encerrando...\n";
	opcFinish();
	
	_endthreadex((DWORD)index);
	return(0);

}