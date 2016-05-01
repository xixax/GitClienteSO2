#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "Jogo.h"


#define PIPE_NAME1 TEXT("\\\\.\\pipe\\teste1")//Le
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\teste2")//Escreve


DWORD WINAPI EscrevePipe(LPVOID param){
	TCHAR buf[256];
	HANDLE pipe = (HANDLE)param;
	DWORD n;
	Jogo j;
	while (1){//estes while(1) são para modificar
		//Ler do terminal
		_tprintf(TEXT("[CLIENTE] Frase: "));
		_fgetts(j.buf, 256, stdin);
		//escrever para o named pipe
		//enviar a struct
		WriteFile(pipe, (LPCVOID)&j, sizeof(j), &n, NULL);
	}
	return 0;
}

int _tmain(int argc, LPTSTR argv[]){
	TCHAR buf[256];
	HANDLE hPipe1, hPipe2;
	int i = 0;
	BOOL ret;
	DWORD n;
	Jogo j;
#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("[CLIENTE]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_NAME1);
	if (!WaitNamedPipe(PIPE_NAME1, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_NAME1);
		exit(-1);
	}

	_tprintf(TEXT("[CLIENTE] Ligação ao servidor... (CreateFile)\n"));
	//Duplex a leitura e escrita tem de ser na mesma thread
	//soluçao 2 pipes
	hPipe1 = CreateFile(PIPE_NAME1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe1 == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_NAME1);
		exit(-1);
	}
	hPipe2 = CreateFile(PIPE_NAME2, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe2 == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_NAME2);
		exit(-1);
	}

	//criaçao da thread que envia dados e recebe
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EscrevePipe, (LPVOID)hPipe2, 0, NULL);
	_tprintf(TEXT("[CLIENTE]Liguei-me...\n"));
	while (1) {
		ret = ReadFile(hPipe1, (LPVOID)&j, sizeof(j), &n, NULL);
		if (n > 0){
			j.buf[(n / sizeof(TCHAR))-1] = '\0'; //pos=255
			if (!ret || !n)
				break;
			_tprintf(TEXT("\n[CLIENTE] Recebi %d bytes: '%s'... (ReadFile)\n"), n, j.buf);
		}
	}
	CloseHandle(hPipe1);
	CloseHandle(hPipe2);

	Sleep(200);
	return 0;
}