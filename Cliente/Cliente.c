#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "Jogo.h"
#include "Mensagem.h"

#define PIPE_NAME1 TEXT("\\\\.\\pipe\\teste1")//Le
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\teste2")//Escreve

//Função do João
/*DWORD WINAPI EscrevePipe(LPVOID param){
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
}*/

BOOL escreveMensagem(Mensagem * msg, HANDLE hPipe, DWORD nBytes) {
	if (!WriteFile(hPipe, (LPCVOID)msg, sizeof(*msg), nBytes, NULL)) {
		return FALSE;
	}

	return TRUE;
}

BOOL leMensagem(Mensagem * msg, HANDLE hPipe, DWORD * nBytes) {
	if (!ReadFile(hPipe, (LPVOID)msg, sizeof(*msg), &nBytes, NULL)) {
		return FALSE;
	}

	return TRUE;
}

BOOL leJogo(Jogo * jogo, HANDLE hPipe, DWORD * nBytes) {
	if (!ReadFile(hPipe, (LPVOID)jogo, sizeof(*jogo), &nBytes, NULL)) {
		return FALSE;
	}

	return TRUE;
}

void pedeOpcao(Mensagem * msg) {
	int option;
	BOOL flag = FALSE;

	do {
		_tprintf(TEXT("0 - Fazer Login\n1 - Efetuar Registo\nOpção:"));
		_tscanf(TEXT("%d"), &option);

		switch (option) {
		case 0: msg->comando = 4; flag = TRUE;  break;
		case 1: msg->comando = 5; flag = TRUE; break;
		default:_tprintf(TEXT("Introduza uma opcao valida!\n"));
		}
	} while (!flag);

	_tprintf(TEXT("USERNAME: "));
	_tscanf(TEXT("%s"), msg->Username);
	//_fgetts(msg->Username, 30, stdin);

	_tprintf(TEXT("PASSWORD: "));
	_tscanf(TEXT("%s"), msg->Password);
	//_fgetts(msg->Password, 30, stdin);
}

DWORD WINAPI opcaoIniciarJogo(LPVOID param){
	int option;
	Mensagem msg;
	DWORD n;
	HANDLE pipeEnvia = (HANDLE)param;
	BOOL flag = FALSE;

	do {
		_tprintf(TEXT("0 - para comecar o jogo\nOpção:"));
		_tscanf(TEXT("%d"), &option);

		switch (option) {
		case 0: msg.comando = 8; flag = TRUE;  
			escreveMensagem(&msg, pipeEnvia, &n);
			break;
		default:_tprintf(TEXT("Introduza uma opcao valida!\n"));
		}
	} while (!flag);
}

void escolheopcoes(Mensagem * msg) {
	int option;
	BOOL flag = FALSE;

	do {
		_tprintf(TEXT("0 - Criar Novo Jogo\n1 - Juntar Jogo\nOpção:"));
		_tscanf(TEXT("%d"), &option);

		switch (option) {
		case 0: msg->comando = 6; flag = TRUE;  break;
		case 1: msg->comando = 7; flag = TRUE; break;
		default:_tprintf(TEXT("Introduza uma opcao valida!\n"));
		}
	} while (!flag);
}

void iniciaJogo(Jogo jogo, Mensagem msg, HANDLE hPipe1, HANDLE hPipe2, DWORD * n) {
	BOOL enviou, recebeu,  flag = FALSE;
	int option;

	//Ciclo de envio de comandos
	while (1) {
		do {
			_tprintf(TEXT("0 - Cima\n1 - Baixo\n2 - Esquerda\n3 - Direita\n\nComando-> "));
			_tscanf("%d", &option);

			switch (option) {
			case 0: msg.comando = 0; flag = TRUE;  break;
			case 1: msg.comando = 1; flag = TRUE; break;
			case 2: msg.comando = 2; flag = TRUE; break;
			case 3: msg.comando = 3; flag = TRUE; break;
			default:_tprintf(TEXT("Introduza um comando válido!\n"));
			}
		} while (!flag);

		//Escrever o comando enviado ao servidor
		enviou = escreveMensagem(&msg, hPipe2, &n);

		if (!enviou) {
			_tprintf(TEXT("[Cliente]: Erro ao enviar mensagem\n"));
			exit(-1);
		}

		//Se enviou com sucesso recebe jogo do servidor
		recebeu = leJogo(&jogo,hPipe1, &n);

		if (!recebeu) {
			_tprintf(TEXT("[Cliente]: Erro ao receber mensagem\n"));
			exit(-1);
		}
		else {
			flag = FALSE;
			//Processa a jogada
		}
	}


}

int _tmain(int argc, LPTSTR argv[]){
	TCHAR buf[256];
	HANDLE hPipe1, hPipe2;
	int i = 0;
	int option;
	BOOL ret, enviou, recebeu;
	DWORD n;
	Jogo j;
	Mensagem msg;

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

	/////////////////////////////////////////////////////////////////////
	/////                   Código André                           /////
	///////////////////////////////////////////////////////////////////

	do{
	//Pergunta ao utilizador se quer fazer LOGIN/REGISTO
	pedeOpcao(&msg);
	
	//Enviar mensagem ao servidor
		enviou = escreveMensagem(&msg, hPipe2, &n);

	if (!enviou) {
		_tprintf(TEXT("[CLIENTE]: A mensagem nao foi enviada!\n"));
		return 0;
	}

	//Receber mensagem do servidor
		recebeu = leMensagem(&msg, hPipe1, &n);

	if (!recebeu) {
		_tprintf(TEXT("[CLIENTE]: A mensagem nao foi recebida!\n"));
		return 0;
	}

	} while (msg.sucesso != 1);//faz isto enquanto der erro
	//Não foi bem sucessido

	//Foi efetuado o login/registo, pedir novas informações ao utilizador
	do{
		escolheopcoes(&msg);

		//Envia ao servidor a resposta
		enviou = escreveMensagem(&msg, hPipe2, &n);

		if (!enviou) {
			_tprintf(TEXT("[CLIENTE]: A mensagem nao foi enviada!\n"));
			return 0;
		}

		//Vai receber o jogo e não uma mensagem
		recebeu = leJogo(&j,hPipe1, &n);

	} while (j.mapa==NULL);

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)opcaoIniciarJogo, (LPVOID)hPipe2, 0, NULL);//thread para qualquer jogador possa iniciar o jogo, caso alguem comece esta threa e terminada

	do{
		recebeu = leJogo(&msg, hPipe1, &n);//recebe informacao do servidor que o jogo vai comecar
	} while (msg.comando!=8);

	recebeu = leJogo(&j, hPipe1, &n);//recebe o jogo completo, pronto a jogar
	if (recebeu){
		iniciaJogo(j, msg, hPipe1, hPipe2, &n);
	}

	//criaçao da thread que envia dados e recebe
	/*CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EscrevePipe, (LPVOID)hPipe2, 0, NULL);
	_tprintf(TEXT("[CLIENTE]Liguei-me...\n"));
	while (1) {
		ret = ReadFile(hPipe1, (LPVOID)&j, sizeof(j), &n, NULL);
		if (n > 0){
			j.buf[(n / sizeof(TCHAR))-1] = '\0'; //pos=255
			if (!ret || !n)
				break;
			_tprintf(TEXT("\n[CLIENTE] Recebi %d bytes: '%s'... (ReadFile)\n"), n, j.buf);
		}
	}*/
	CloseHandle(hPipe1);
	CloseHandle(hPipe2);

	Sleep(200);
	return 0;
}