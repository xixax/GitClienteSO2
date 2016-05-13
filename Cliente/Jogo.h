#include "Main.h"
#include "Mapa.h"

typedef struct Jogos{
	Mapa *mapa;
	Jogador jogador;//utilizado para enviar o jogador do cliente
	//monstro
	// _TCHARbuf[256];
	int jogocomecou;//0 nao e 1 sim
}Jogo;