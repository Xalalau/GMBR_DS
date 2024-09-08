/*
 * GERAL v.Git
 * por Xalalau Xubilozo
 * https://github.com/xalalau/Xalateca/tree/master/C/Geral
 * Licença: MIT
 * */

#ifndef GERAL_H
#define GERAL_H

	#define MAX_CHAR_DIR 255
	#define MAX_CHAR_URL 2000

	#include <stdlib.h> /* system() */

	#if __linux__
		#define PULO "\n"
		#define BARRA "/"
		#define BARRA2 '/'
		#define SISTEMA "Linux"
		#define CLRSCR system("clear")
		#define PAUSA system("read pause")
	#elif _WIN32
		#define PULO "\r\n"
		#define BARRA "\\"
		#define BARRA2 '\\'
		#define SISTEMA "Windows"
		#define CLRSCR system("cls")
		#define PAUSA system("echo. & pause")
	#endif

	// [Classe] Exposição das funções necessárias para os usuários
	struct geral_h {
		void (*erro)();
		void (*pausar)();
		int (*pegarPastaCorrente)();
		int (*verificarExistencia)(char arquivo[]);
		int (*criarPasta)(char pasta[]);
		int (*existeCharXNaStringY)(char ch, char array[]);
	};

	extern struct geral_h geral;

#endif //GERAL_H
