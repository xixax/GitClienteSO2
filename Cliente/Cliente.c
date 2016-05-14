#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "Jogo.h"
#include "Mensagem.h"

#define PIPE_NAME1 TEXT("\\\\.\\pipe\\teste1")//Le
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\teste2")//Escreve

Jogo j;//preciso disto aqui, pois so podemos enviar 1 parametro para dentro de uma thread
BOOLEAN flgSegundaFase;//necessario para desbloquear jogadores da segunda fase

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
		default:_tprintf(TEXT("Introduza uma opcao valida!\n")); break;
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

	do {
		_tprintf(TEXT("0 - para comecar o jogo\nOpção:"));
		_tscanf(TEXT("%d"), &option);

		switch (option) {
		case 0: msg.comando = 8; flgSegundaFase = TRUE;
			escreveMensagem(&msg, pipeEnvia, &n);
			break;
		default:_tprintf(TEXT("Introduza uma opcao valida!\n")); break;
		}
	} while (!flgSegundaFase);
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
		default:_tprintf(TEXT("Introduza uma opcao valida!\n")); break;
		}
	} while (!flag);
}

void iniciaJogo(Jogo jogo, Mensagem msg, HANDLE hPipe1, HANDLE hPipe2, DWORD * n) {
	BOOL enviou, recebeu,  flag = FALSE;
	int option;

	//Ciclo de envio de comandos
	while (1) {
		_tprintf(TEXT("\n\nJogador\nVida:%d\nLentidao:%d\nPedras:%d\nPosx:%d\nPosy:%d\n\n"),jogo.jogador.vida,jogo.jogador.lentidao,jogo.jogador.pedras,jogo.jogador.posx,jogo.jogador.posy);
		do {
			_tprintf(TEXT("0 - Cima\n1 - Baixo\n2 - Esquerda\n3 - Direita\n\nComando-> "));
			_tscanf(TEXT("%d"), &option);

			switch (option) {
			case 0: msg.comando = 0; flag = TRUE;  break;
			case 1: msg.comando = 1; flag = TRUE; break;
			case 2: msg.comando = 2; flag = TRUE; break;
			case 3: msg.comando = 3; flag = TRUE; break;
			default:_tprintf(TEXT("Introduza um comando válido!\n")); break;
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

DWORD WINAPI actualizaJogo(LPVOID param){
	TCHAR buf[256];
	HANDLE pipe = (HANDLE)param;
	DWORD n;
	BOOL recebeu;
	while (1){
		recebeu = leJogo(&j, pipe, &n);
		if (recebeu){
			_tprintf(TEXT("\n\nJogador\nVida:%d\nLentidao:%d\nPedras:%d\nPosx:%d\nPosy:%d\n\n"), j.jogador.vida, j.jogador.lentidao, j.jogador.pedras, j.jogador.posx, j.jogador.posy);
			_tprintf(TEXT("0 - Cima\n1 - Baixo\n2 - Esquerda\n3 - Direita\n\nComando-> "));
		}
	}
	return 0;
}

int _tmain(int argc, LPTSTR argv[]){
	TCHAR buf[256];
	HANDLE hPipe1, hPipe2, hopcaoIniciarJogo, hactualizaJogo;
	int i = 0;
	int option;
	BOOL ret, enviou, recebeu;
	DWORD n;
	Mensagem msg;
	flgSegundaFase = FALSE;

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

	
	//necessario para desbloquear os outros jogadores da segunda faase

	hopcaoIniciarJogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)opcaoIniciarJogo, (LPVOID)hPipe2, 0, NULL);//thread para qualquer jogador possa iniciar o jogo, caso alguem comece esta threa e terminada

	do{
		recebeu = leMensagem(&msg, hPipe1, &n);//recebe informacao do servidor que o jogo vai comecar
	} while (msg.comando!=8);
	if (flgSegundaFase==FALSE){
		enviou = escreveMensagem(&msg, hPipe2, &n);
	}
	//terminar a thread opcaoIniciarJogo porque ja foi iniciado
	TerminateThread(hopcaoIniciarJogo, 0);
	CloseHandle(hopcaoIniciarJogo);

	recebeu = leJogo(&j, hPipe1, &n);//recebe o jogo completo, pronto a jogar
	if (recebeu){
		hactualizaJogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)actualizaJogo, (LPVOID)hPipe2, 0, NULL);//thread para qualquer jogador possa iniciar o jogo, caso alguem comece esta threa e terminada, falta ver o pq de isto nao estar a funcionar :)
		iniciaJogo(j, msg, hPipe1, hPipe2, &n);
		TerminateThread(hactualizaJogo, 0);
		CloseHandle(hactualizaJogo);
	}
	else{
		_tprintf(TEXT("[CLIENTE]: Nao foi possivel entrar no jogo!\n"));
		return 0;
	}
	
	//falta quando acaba o jogo voltar ao menu principal

	CloseHandle(hPipe1);
	CloseHandle(hPipe2);

	Sleep(200);
	return 0;
}