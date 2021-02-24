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



// Funções de Criação das Mensagens
char*  novaMensagem11(int* nseq);
char*  novaMensagem33(int* nseq);
char*  novaMensagem99(int* nseq);

int main(int argc, char **argv)
{
    //Variáveis socket//
    WSADATA wsaData;
    SOCKET s;
    int statusSocket;
    //SOCKADDR_IN ServerAddr;

    //Variáveis do Temporizador//
    HANDLE hTimer;
    LARGE_INTEGER Present;



    
    return(0);
}


char* novaMensagem11(int* nseq) {
	string msg;
	int aux = rand() % 9999;

	sprintf(msg, "%05d", m1.nseq);
	msg += "$";
	msg += to_string(11) + "$";
	msg += to_string(rand() % 999999) + "$";
	msg += to_string((float)aux / 10) + "$";
	aux = rand() % 9999;
	msg += to_string(m1.tempTrans) + "$";
	aux = rand() % 9999;
	msg += to_string((float)aux / 10;);

	*nseq++;
	if (*nseq == 99999) {
		*nseq = 1;
	}

	return msg;
};

char* novaMensagem33(int* nseq) {
	string msg;

	sprintf(msg, "%05d", m1.nseq);
	msg += "$";
	msg += to_string(33);

	*nseq++;
	if (*nseq == 99999) {
		*nseq = 1;
	}


	return msg;
};

char* novaMensagem99(int* nseq) {
	string msg;

	sprintf(msg, "%05d", m1.nseq);
	msg += "$";
	msg += to_string(99);

	*nseq++;
	if (*nseq == 99999) {
		*nseq = 1;
	}


	return msg;

};

