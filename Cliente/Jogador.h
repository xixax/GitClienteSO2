#include "Main.h"

typedef struct Jogador{
	int vida;
	int lentidao;
	int pedras;//unico objecto que pode acumular até 15 pedras
	//variavel a indicar se ataca com pedra ou mao

	//posicao
	int posx;
	int posy;
}Jogador;