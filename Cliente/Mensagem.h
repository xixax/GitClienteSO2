#include "Main.h"

typedef struct Mensagem{
	_TCHAR Username[30];
	_TCHAR Password[30];
	int sucesso;

	int comando;//0-cima 1-baixo 2-esquerda 3-direita 4-Faz Login 5-Faz Registo 6-Criar Jogo 7-Juntar a jogo 8-Começa jogo
}Mensagem;