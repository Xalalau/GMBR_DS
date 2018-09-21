/*
 *
 * por Xalalau Xubilozo
 * https://github.com/xalalau/GMBR_DS
 * Licença: MIT
 *
 */

#include "../lib/Geral/src/geral.h"
#include "../lib/Inizator/src/inizator.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#if __linux__
	#include <unistd.h>
	#include <pwd.h>
	#define ShellExecute(a, b, c, d, e, f) printf(" ") /* Definicao desbugante de Linux */
	#define SW_SHOWDEFAULT 0
	#define DPADRAO(pw) pw = getpwuid(getuid())
	#define DPADRAO2(pw, homedir) homedir = pw->pw_dir
#elif _WIN32
	#include <direct.h>
	#include <windows.h>
	#define DPADRAO(pw) printf(" ") /* Definicao desbugante de Windows */
	#define DPADRAO2(pw, homedir) printf(" ") /* Definicao desbugante de Windows */
#endif

#define MAX_LIBS 10 /* Número máximo suportado de bibliotecas do Steam. Pode ser aumentado à vontade. */

// Garry's Mod
struct gmod_ {
	char nome[30]; /* Nome do GMod */
	char id[10]; /* ID do GMod no Steam */
	char arquivo_mount[MAX_CHAR_DIR]; /* Caminho até o arquivo mount.cfg Nome do GMod */
};

// Bibliotecas do Steam
struct libs_ {
	char biblioteca[MAX_LIBS][MAX_CHAR_DIR]; /* Caminhos de bibliotecas do Steam */
	int quantidade; /* Quantidade de bibliotecas do Steam */
};

// Steam
struct steam_ {
	char arquivo_bibliotecas[MAX_CHAR_DIR]; /* Caminho até o arquivo libraryfolders.vdf */
	char arquivo_achar[MAX_CHAR_DIR]; /* Caminho até o arquivo steam.pid */
	char arquivo_steamcmd[MAX_CHAR_DIR]; /* Caminho até o arquivo steamcmd.sh ou steamcmd.exe */
	char arquivo_srcds[MAX_CHAR_DIR]; /* Caminho até o arquivo srcds_run ou srcds.exe */
	char aviso_nao_encontrado[16]; /* Mensagem de "Steam não encontrado" */
	struct libs_ libs;
};

// Estrutura principal de controle
struct config_ {
	char diretorio_base[MAX_CHAR_DIR]; /* Caminho até o diretório corrente */
	char caminho_ini_geral[MAX_CHAR_DIR]; /* Caminho até o cfg.no */
	char caminho_ini_contents[MAX_CHAR_DIR]; /* Caminho até o contents.no */
	char secao[4][9]; /* Nome das seções no arquivo cfg.no */
	no *parametros; /* Estrutura do cfg.ini carregada */
	no *contents; /* Estrutura do contents.ini carregada */
	struct gmod_ gmod;
	struct steam_ steam;
} cfg;

void ajustarConfiguracoesBasicas() {
	// Configurações que devem ser feitas primeiro ou não podem ficar espalhadas pelo código

	strcat(strcat(strcat(strcpy(cfg.caminho_ini_geral, cfg.diretorio_base),"cfg"), BARRA),"cfg.ini");
	strcat(strcat(strcat(strcpy(cfg.caminho_ini_contents, cfg.diretorio_base), "cfg"), BARRA),"contents.ini");

	if (strcmp(SISTEMA,"Linux") == 0) {
		strcpy(cfg.steam.arquivo_achar, "steam.pid");
		strcpy(cfg.steam.arquivo_steamcmd, "steamcmd.sh");
		strcpy(cfg.steam.arquivo_srcds, "srcds_run");
	} else if (strcmp(SISTEMA,"Windows") == 0) {
		strcpy(cfg.steam.arquivo_achar, "steam.dll");
		strcpy(cfg.steam.arquivo_steamcmd, "steamcmd.exe");
		strcpy(cfg.steam.arquivo_srcds, "srcds.exe");
	}

	strcpy(cfg.steam.arquivo_bibliotecas, "libraryfolders.vdf");
	strcpy(cfg.steam.aviso_nao_encontrado, "NAO ENCONTRADA!");

	strcpy(cfg.gmod.nome, "Garry's Mod");
	strcpy(cfg.gmod.id, "4020");
	strcpy(cfg.gmod.arquivo_mount, "mount.cfg");
}

int obterPastaCorrente() {
	// Define o diretório base como o corrente
	// Nota: pode ser redefinido em ajustarVariaveis() se o usuário entrar com um endereço completo em "pasta_base"
	// Retorno:
	// 1 = OK, 0 = Erro

	if (geral.pegarPastaCorrente(cfg.diretorio_base)) {
		printf("[GMBR DS] Diretorio corrente obtido\n");

		return 1;
	} else {
		printf("\n[GMBR DS]----------------------------------------------------------------\n");
		printf("# ERRO AO LER O CAMINHO DA PASTA CORRENTE #\n");
		printf("-------------------------------------------------------------------------\n\n");
		geral.pausar();

		return 0;
	}
}

int carregarCfgIni() {
	// Carrega as informações do arquivo cfg.ini
	// Retorno:
	// 1 = OK, 0 = Erro

	// Existe o arquivo cfg.ini?
	if (geral.verificarExistencia(cfg.caminho_ini_geral)) {
		//Sim
		if ((cfg.parametros = ini.ler(cfg.caminho_ini_geral)) != NULL) {
			// Leu com sucesso
			printf("[GMBR DS] Arquivo cfg.ini carregado\n");
		} else {
			// Problema na leitura
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("ERRO AO CARREGAR AS OPCOES DO ARQUIVO\n");
			printf("'%s'\n", cfg.caminho_ini_geral);
			printf("VERIFIQUE SE TODOS OS PARAMETROS ESTAO PRESENTES E BEM FORMATADOS\n");
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;
		}
	} else {
		// Não
		printf("\n[GMBR DS]--------------------------------------------------------------\n");
		printf("NAO FOI POSSIVEL CARREGAR O ARQUIVO\n");
		printf("'%s'\n", cfg.caminho_ini_geral);
		printf("1) VERIFIQUE SE ELE ESTA PRESENTE NA PASTA 'CFG'\n");
		printf("2) VERIFIQUE SE TODOS OS PARAMETROS ESTAO PRESENTES E BEM FORMATADOS\n");
		printf("-----------------------------------------------------------------------\n\n");
		geral.pausar();

		return 0;
	}

	return 1;
}

int carregarContentsIni() {
	// Carrega as informações do arquivo contents.ini
	// Retorno:
	// 1 = OK, 0 = Erro

	// Existe o arquivo contents.ini?
	if (geral.verificarExistencia(cfg.caminho_ini_contents)) {
		// Sim
		if ((cfg.contents = ini.ler(cfg.caminho_ini_contents)) != NULL) {
			// Leu com sucesso
			printf("[GMBR DS] Arquivo contents.ini carregado\n");
		} else {
			// Erro na leitura
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("ERRO AO CARREGAR O ARQUIVO\n");
			printf("'%s'\n", cfg.caminho_ini_contents);
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;
		}
	} else {
		// Não
		printf("\n[GMBR DS]--------------------------------------------------------------\n");
		printf("NAO FOI POSSIVEL CARREGAR O ARQUIVO\n");
		printf("'%s'\n", cfg.caminho_ini_contents);
		printf("-----------------------------------------------------------------------\n\n");
		geral.pausar();

		return 0;
	}

	return 1;
}

int encontrarSteam() {
	// Busca/Verifica/Salva o caminho da pasta do Steam
	// Retorno:
	// 1 = Econtrado, 0 = Não encontrado

	char temp[MAX_CHAR_DIR];

	// Existe algum valor em "pasta_steam" no cfg.ini?
	if (strcmp(ini.buscar(cfg.parametros, "", "pasta_steam"),"") == 0) {
		// Não

		// Vamos buscar a pasta do Steam nos lugares mais comuns onde ela pode estar
		if (strcmp(SISTEMA,"Linux") == 0) {
			// Linux
			struct passwd *pw;
			const char *homedir;
			DPADRAO(pw);
			DPADRAO2(pw, homedir);
			if (geral.verificarExistencia(strcat(strcat(strcat(strcat(strcpy(temp, homedir), BARRA), ".steam"), BARRA), cfg.steam.arquivo_achar)))
				strcpy(ini.buscar(cfg.parametros, "", "pasta_steam"), strcat(strcat(strcpy(temp, homedir), BARRA), ".steam"));
		} else if (strcmp(SISTEMA,"Windows") == 0) {
			// Windows
			char unidade[5][3] = {"C:", "D:", "E:", "F:", "G:"};
			char pasta[5][36] = {
				"\\Steam",
				"\\Program Files (x86)\\Steam",
				"\\Program Files\\Steam",
				"\\Arquivos de Programas (x86)\\Steam",
				"\\Arquivos de Programas\\Steam"
			};
			int i,j;

			for (i=0; i<=3; i++)
				for (j=0; j<=4; j++)
					if (geral.verificarExistencia(strcat(strcat(strcat(strcpy(temp, unidade[i]), pasta[j]), BARRA), cfg.steam.arquivo_achar)))
						strcpy(ini.buscar(cfg.parametros, "", "pasta_steam"), strcat(strcpy(temp, unidade[i]), pasta[j]));
		}

		// Caso eu não a encontre, salvo uma mensagem de erro em "pasta_steam"
		if (strcmp(ini.buscar(cfg.parametros, "", "pasta_steam"),"") == 0)
			ini.alterar(cfg.parametros, "", "pasta_steam", cfg.steam.aviso_nao_encontrado);
	} else {
		// Sim

		// Eu consigo encontrar os arquivos do Steam no local informado?
		if (geral.verificarExistencia(strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steam")), BARRA), cfg.steam.arquivo_achar))) {
			// Sim. Sucesso
			printf("[GMBR DS] A pasta do Steam foi localizada\n");
		} else {
			// Não. Erro
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("A PASTA DO STEAM E INVALIDA:\n");
			printf("'%s'\n", ini.buscar(cfg.parametros, "", "pasta_steam"));
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;
		}
	}

	return 1;
}

void ajustarVariaveis() {
	// Ajusta as informações que foram carregadas dos arquivos de configurações

	char temp[MAX_CHAR_DIR];
	no* contents = cfg.contents;

	// Redefine o caminho para a pasta base se ela estiver alterada nas configurações
	if ((strcmp(ini.buscar(cfg.parametros, "", "pasta_base"), "")) != 0)
		strcat(strcpy(cfg.diretorio_base, ini.buscar(cfg.parametros, "", "pasta_base")), BARRA);

	// Define o caminho da pasta dos contents
	if ( ! geral.existeCharXNaStringY(BARRA[0], ini.buscar(cfg.parametros, "", "pasta_contents"))) { /* A pasta é relativa? Se sim, alterar o valor */
		strcat(strcpy(temp, cfg.diretorio_base), ini.buscar(cfg.parametros, "", "pasta_contents"));
		ini.alterar(cfg.parametros, "", "pasta_contents", temp);
	}

	// Define o caminho da pasta do SteamCMD
	if ( ! geral.existeCharXNaStringY(BARRA[0], ini.buscar(cfg.parametros, "", "pasta_steamcmd"))) { /* A pasta é relativa? Se sim, alterar o valor */
		strcat(strcpy(temp, cfg.diretorio_base), ini.buscar(cfg.parametros, "", "pasta_steamcmd"));
		ini.alterar(cfg.parametros, "", "pasta_steamcmd", temp);
	}

	// Define o caminho para a pasta do servidor de GMod
	if ( ! geral.existeCharXNaStringY(BARRA[0], ini.buscar(cfg.parametros, "", "pasta_servidor"))) { /* A pasta é relativa? Se sim, alterar o valor */
		strcat(strcpy(temp, cfg.diretorio_base), ini.buscar(cfg.parametros, "", "pasta_servidor"));
		ini.alterar(cfg.parametros, "", "pasta_servidor", temp);
	}

	// Define os caminhos da pasta de cada content dentro da pasta dos contents
	while ((*contents).proxima_secao != NULL) {
		if (strcmp(ini.buscar(contents, (*contents).secao, "opcao"), "1") == 0) {
			strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_contents")), BARRA), (*contents).secao);
			ini.inserir(contents, (*contents).secao, "pasta", temp);
		}
		contents = (*contents).proxima_secao;
	}

	// Define o caminho para o executável do SteamCMD
	strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steamcmd")), BARRA), cfg.steam.arquivo_steamcmd);
	strcpy(cfg.steam.arquivo_steamcmd, temp);

	// Define o caminho para o executável do SRCDS
	strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_servidor")), BARRA), cfg.steam.arquivo_srcds);
	strcpy(cfg.steam.arquivo_srcds, temp);

	// Define o caminho para o arquivo de bibliotecas
	if (strcmp(ini.buscar(cfg.parametros, "", "pasta_steam"), cfg.steam.aviso_nao_encontrado) != 0) { /* O Steam foi encontrado anteriormente? Se sim, alterar o valor */
		if (strcmp(SISTEMA,"Linux") == 0)
			strcat(strcat(strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steam")), BARRA), "steam/steamapps"), BARRA), cfg.steam.arquivo_bibliotecas);
		else if (strcmp(SISTEMA,"Windows") == 0)
			strcat(strcat(strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steam")), BARRA), "steamapps"), BARRA), cfg.steam.arquivo_bibliotecas);
		strcpy(cfg.steam.arquivo_bibliotecas, temp);
	}

	// Define o caminho para o arquivo de montagem de contents do GMod
	strcat(strcat(strcat(strcat(strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_servidor")), BARRA), "garrysmod"), BARRA), "cfg"), BARRA), cfg.gmod.arquivo_mount);
	strcpy(cfg.gmod.arquivo_mount, temp);

	printf("[GMBR DS] Variaveis reajustadas\n");
}

int buscarBibliotecas() {
	// Lê o arquivo de bibliotecas de jogos do Steam
	// Retorno:
	// 1 = Carregou as pastas, 0 = Erro

	// Checa se o Steam foi encontrado anteriormente
	if (strcmp(ini.buscar(cfg.parametros, "", "pasta_steam"), cfg.steam.aviso_nao_encontrado) != 0) {
		// Sim

		FILE *fp;
		int i = 0, j = 0;
		char c, caracteres[MAX_CHAR_DIR];
		char *prt;

		fp = fopen(cfg.steam.arquivo_bibliotecas, "r");

		// Verifica se houve erro ao abrir o arquivo
		if (fp == NULL) {
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("ERRO AO ABRIR O ARQUIVO DE BIBLIOTECAS DO STEAM!\n");
			printf("'%s'\n", cfg.steam.arquivo_bibliotecas);
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;
		}

		// Começo a ler o arquivo em busca de bibliotecas
		while ((c = getc(fp)) != EOF) {
			// Busco a primeira aspa
			if (c == '"') {
				while ((c = fgetc(fp)) != EOF) {
					// Capturo os caracteres em um array até achar outra aspa
					if (c != '"')
						caracteres[j++] = c;
					// Verifico se o array é um número
					else {
						caracteres[j] = '\0';
						// Se for um número: guardo o valor entre as práximas aspas, pois é uma pasta de biblioteca do Steam
						if (strtol(caracteres, &prt, 10) != 0) {
							// Verifica o limite de bibliotecas do GMBR DS antes de prosseguir
							if (i > MAX_LIBS) {
								printf("\n[GMBR DS]--------------------------------------------------------------\n");
								printf("O NUMERO DE BIBLIOTECAS DO STEAM EXCEDEU O LIMITE DO GMBR DS, QUE E %d!\n", MAX_LIBS + 1);
								printf("POR FAVOR, CONTACTE ALGUM MEMBRO DO GMBR PARA QUE ESSE LIMITE SEJA AUMENTADO\n");
								printf("OU TENTE RECOMPILAR O PROJETO VOCE MESMO. TER MENOS BIBLIOTECAS TAMBEM E UMA\n");
								printf("OPCAO.\n");
								printf("-----------------------------------------------------------------------\n\n");
								geral.pausar();

								return 0;
							}

							// Ok. Vamos pegar a pasta
							j = 0;
							while ((c = fgetc(fp)) != EOF) {
								if (c == '"') {
									while ((c = fgetc(fp)) != EOF) {
										if (c != '"')
											caracteres[j++] = c;
										else {
											caracteres[j] = '\0';
											j = 0;
											// Salvo o diretório
											strcpy(cfg.steam.libs.biblioteca[i++], caracteres);
										}
									}
									break;
								}
							}
						// Se não for um número: busco o fim de linha, zero o j e continuo em busca de outro caso válido
						} else {
							while ((c = fgetc(fp)) != EOF) {
								if (c == '\n') {
									j = 0;
									break;
								}
							}
						}
						break;
					}
				}
			}
		}

		fclose(fp);

		// Guardo a quantidade de bibliotecas encontrada
		cfg.steam.libs.quantidade = i;
	}

	printf("[GMBR DS] Bibliotecas de jogos do Steam carregadas\n");

	return 1;
}

int validarContents() {
	// Verifica se os contents estão presentes nos caminhos informados
	// Retorno:
	// 1 = Leitura bem sucedida, 0 = Erro.

	int i = 0, j, contagem_de_contents = 0;
	char temp[MAX_CHAR_DIR], opcao[1];
	no* contents = cfg.contents;

	// Checo os contents um por um
	while ((*contents).proxima_secao != NULL) {
		i++;

		// Opção
		strcpy(opcao, ini.buscar(cfg.contents, (*contents).secao, "opcao"));

		// Validação de contents do tipo 1
		if (strcmp(opcao, "1") == 0) {
			char login[30];

			strcpy(login, ini.buscar(cfg.contents, (*contents).secao, "login"));

			contagem_de_contents++;

			// Checa se o content foi marcado para download mesmo não podendo ser baixado
			if (strcmp(login, "Indisponivel") == 0) {
				printf("\n[GMBR DS]--------------------------------------------------------------\n");
				printf("O CONTENT '%s' E INVALIDO!\n", (*contents).secao);
				printf("VOCE MARCOU PARA DOWNLOAD UM CONTENT QUE NAO SUPORTA TAL OPERACAO. VA\n");
				printf("EM 'CONTENTS.INI' CORRIGIR O PROBLEMA.\n");
				printf("-----------------------------------------------------------------------\n\n");
				geral.pausar();

				return 0;
			}

			// Checa se o content necessita de login e se o login foi configurado
			if (strcmp(login, "Usuario") == 0) {
				if (strcmp(ini.buscar(cfg.parametros, "", "login_steam"), "") == 0) {
					printf("\n[GMBR DS]--------------------------------------------------------------\n");
					printf("ERRO NO CONTENT '%s'!\n", (*contents).secao);
					printf("ESSE CONTENT REQUER LOGIN PARA SER BAIXADO, POREM O CAMPO \"login_steam\"\n");
					printf("NAO FOI CONFIGURADO EM 'CFG.INI'.\n");
					printf("-----------------------------------------------------------------------\n\n");
					geral.pausar();

					return 0;
				}
			}

		// Validação de contents do tipo 2
		} else if (strcmp(opcao, "2") == 0) {
			contagem_de_contents++;

			// Verifica se "Caminho do content" não é um endereço válido
			if ( ! geral.verificarExistencia(ini.buscar(cfg.contents, (*contents).secao, "steam"))) {
				// Verifica se a pasta do Steam está configurada
				if (strcmp(ini.buscar(cfg.parametros, "", "pasta_steam"), cfg.steam.aviso_nao_encontrado) == 0) {
					printf("\n[GMBR DS]--------------------------------------------------------------\n");
					printf("O GMBR DS NAO FOI CAPAZ DE ENCONTRAR A PASTA DO STEAM AUTOMATICMENTE!\n");
					printf("PARA PODER USAR SEUS CONTENTS QUE FORAM MARCADOS COM A OPCAO 2, INSIRA-A\n");
					printf("NO ARQUIVO 'CFG.INI' !OU! ESCREVA O ENDERECO DE DIRETORIO COMPLETO DELES\n");
					printf("EM 'CONTENTS.INI'.\n");
					printf("-----------------------------------------------------------------------\n\n");
					geral.pausar();

					return 0;

				// Verifica se existe "Steam + Caminho content"
				} else if (geral.verificarExistencia(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steam")), ini.buscar(cfg.contents, (*contents).secao, "steam"))))
					ini.alterar(cfg.contents, (*contents).secao, "steam", temp);

				// Verifica se existe "Steam + BARRA + Caminho content"
				else if (geral.verificarExistencia(strcat(strcat(strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steam")), BARRA), ini.buscar(cfg.contents, (*contents).secao, "steam"))))
					ini.alterar(cfg.contents, (*contents).secao, "steam", temp);
				else
					for (j = 0; j < MAX_LIBS; j++) {
						// Verifica se existe "Biblioteca + Caminho content"
						if ((geral.verificarExistencia(strcat(strcpy(temp, cfg.steam.libs.biblioteca[j]), ini.buscar(cfg.contents, (*contents).secao, "steam")))))
							ini.alterar(cfg.contents, (*contents).secao, "steam", temp);

						// Verifica se existe "Biblioteca + BARRA + Caminho content"
						else if ((geral.verificarExistencia(strcat(strcat(strcpy(temp, cfg.steam.libs.biblioteca[j]), BARRA), ini.buscar(cfg.contents, (*contents).secao, "steam")))))
							ini.alterar(cfg.contents, (*contents).secao, "steam", temp);
					}

				// Se o content ainda não for válido, retorno erro
				if ( ! geral.verificarExistencia(ini.buscar(cfg.contents, (*contents).secao, "steam"))) {
					printf("\n[GMBR DS]--------------------------------------------------------------\n");
					printf("A PASTA DO CONTENT '%s' E INVALIDA!\n", (*contents).secao);
					printf("CHEQUE SE ELA EXISTE EM STEAMAPPS OU SE VOCE ESCREVEU CORRETAMENTE:\n");
					printf("'%s'\n", ini.buscar(cfg.contents, (*contents).secao, "steam"));
					printf("-----------------------------------------------------------------------\n\n");
					geral.pausar();

					return 0;
				}
			}
		}
		contents = (*contents).proxima_secao;
	}

	// Sucesso, todos os contents são válidos
	if (contagem_de_contents > 0) {
		printf("[GMBR DS] %d contents foram validados com sucesso\n", contagem_de_contents);

		return 1;

	// Sucesso, não há contents
	} else {
		printf("[GMBR DS] Nao ha contents configurados\n");

		return 1;
	}
}

int exibirTelaInicial() {
	// Apresentação e menu principal
	// Retorno:
	// Escolha (válida) do menu

	int numero_real, numero_exibido;
	int quantidade_de_opcoes = 6, opcao[quantidade_de_opcoes], escolha, escolha_ruim;
	char *endptr, temp[MAX_CHAR_DIR];
	no *contents;

	// Elimina o lixo do array
	for (numero_real=0; numero_real<=quantidade_de_opcoes; numero_real++)
		opcao[numero_real] = -1;

	// Exibição do menu
	while (1) {
		CLRSCR;
		numero_exibido = 1;
		printf("__.d8888b.__888b_____d888_888888b.___8888888b._______8888888b.___.d8888b.\n");
		printf("_d88P__Y88b_8888b___d8888_888___88b__888___Y88b______888___Y88b_d88P__Y88b\n");
		printf("_888____888_88888b.d88888_888__d88P__888____888______888____888_Y88b.\n");
		printf("_888________888Y88888P888_8888888K.__888___d88P______888____888___Y888b.\n");
		printf("_888__88888_888_Y888P_888_888___Y88b_8888888P8_______888____888_______Y88b.\n");
		printf("_888____888_888__Y8P__888_888____888_888_T88b________888____888+_______888\n");
		printf("_Y88b__d88P_888___8___888_888___d88P_888__T88b_______888__.d88P_Y88b__d88P\n");
		printf("__Y8888P88__888_______888_8888888P8__888___T88b______8888888P_____Y8888P\n");
		printf("\n");
		printf("Garry's Mod Brasil Dedicated Server - GMBR DS\n");
		printf("Feito por Xalalau - Garry s Mod Brasil, GMBR\n");
		printf("http://www.gmbrblog.blogspot.com.br/\n");
		printf("http://mrxalalau.blogspot.com.br/\n");
		printf("___________________________________________________________________[v2.8.5]\n");
		printf("\n");

		/* MENU PRINCIPAL
		 *
		 * Opção do tipo:
		 * (A) É exibida se houver contents configurados;
		 * (B) É exibida se o servidor de GMod estiver instalado.
		 *
		 * Escolhas de 0 a 6:
		 *
		 * 1 - Definições
		 * 2 - Instalar server de GMod
		 * 3 - Instalar contents (A)
		 * 4 - Instalar server de GMod e contents (A)
		 * 5 - Ligar o servidor (B)
		 * 6 - Montar os contents (A)(B)
		 * 0 - Sair
		 *
		 * A ideia é exibir esse limite de 7 opções em sequência com "numero_exibido" e depois
		 * convertê-las para "numero_real", que é sempre o mesmo para cada opção. Apenas o zero
		 * aparece fora de ordem e não requer ser transformado.
		*/

		// Imprimir opções em sequência
		printf("%d: Ver as configuracoes carregadas\n", numero_exibido);
		opcao[0] = numero_exibido++;
		printf("%d: Instalar/atualizar o servidor de GMod\n", numero_exibido);
		opcao[1] = numero_exibido++;
		contents = cfg.contents;
		while ((*contents).proxima_secao != NULL) {
			if (strcmp(ini.buscar(contents, (*contents).secao, "opcao"), "1") == 0) {
				printf("%d: Instalar/atualizar os contents\n", numero_exibido);
				opcao[2] = numero_exibido++;
				printf("%d: Instalar/atualizar o servidor de GMod e os contents\n", numero_exibido);
				opcao[3] = numero_exibido++;
				break;
			}
			contents = (*contents).proxima_secao;
		}
		if (geral.verificarExistencia(cfg.steam.arquivo_srcds)) {
			printf("%d: Iniciar o servidor\n", numero_exibido);
			opcao[4] = numero_exibido++;
			printf("%d: Forcar montagem de contents\n", numero_exibido);
			opcao[5] = numero_exibido++;
		}
		printf("0: Sair\n");
		printf("\n");
		printf("Escolha: ");
		scanf("%s", temp);
		printf("\n");

		// Converter o string para int
		escolha = strtol(temp, &endptr, 10);

		// Se houver lixo na entrada, reinicia o menu
		if (strcmp(endptr, "") != 0)
			continue;

		// Converter "numero_exibido" para "numero_real"
		// Obs: opção Zero não precisa ser convertida
		escolha_ruim = 1;
		for (numero_real = 0; numero_real <= quantidade_de_opcoes; numero_real++)
			if (opcao[numero_real] == escolha) {
				escolha = numero_real + 1;
				escolha_ruim = 0;
				break;
			}

		// Se a opção nao tiver sido convertida no passo anterior e não for 0, reinicia o menu
		if ((escolha_ruim == 1) && (escolha != 0))
			continue;

		// Retorna uma opção válida para processá-la
		return escolha;
	}
}

void exibirConfiguracoes() {
	// Mostra todas as opções carregadas

	int i;
	no *contents;

	// Limpo a tela
	CLRSCR;

	printf("______________________________________________________________________________\n");
	printf("\n");
	printf("CONFIGURACOES CARREGADAS:\n");
	printf("______________________________________________________________________________\n");
	printf("\n");


	// Pastas, no geral
	printf(" ______\n");
	printf("[PASTAS]\n\n");
	printf("  Base:\n");
	printf("    ");
	for (i = 0; i < strlen(cfg.diretorio_base) - 1; i++) /* Tiro a barra no final */
		printf("%c", cfg.diretorio_base[i]);
	printf("\n  Servidor de GMod:\n    %s\n",ini.buscar(cfg.parametros, "", "pasta_servidor"));
	printf("  Contents:\n    %s\n",ini.buscar(cfg.parametros, "", "pasta_contents"));
	printf("  Steam:\n    %s\n", ini.buscar(cfg.parametros, "", "pasta_steam"));
	printf("  SteamCMD:\n    %s\n", ini.buscar(cfg.parametros, "", "pasta_steamcmd"));

	// Bibliotecas do Steam (Caso existam)
	if (cfg.steam.libs.quantidade > 0) {
		printf(" ____________________\n");
		printf("[BIBLIOTECAS DO STEAM]\n\n");
		for (i = 0; i < cfg.steam.libs.quantidade; i++)
			printf("  [%d] %s\n", i + 1, cfg.steam.libs.biblioteca[i]);
	}

	// Arquivos que serão baixados
	printf(" ________\n");
	printf("[DOWNLOAD]\n\n");
	printf("  SteamCMD:\n    %s\n", ini.buscar(cfg.parametros, "", "download_steamcmd"));

	// Arquivos importantes para o GMBR DS
	printf(" ________\n");
	printf("[ARQUIVOS]\n\n");
	printf("  Configuracoes gerais do GMBR DS:\n    %s\n", cfg.caminho_ini_geral);
	printf("  Configuracao de contents do GMBR DS:\n    %s\n", cfg.caminho_ini_contents);
	printf("  Montagem de contents do GMod:\n    %s\n", cfg.gmod.arquivo_mount);
	printf("  Executavel do SteamCMD:\n    %s\n", cfg.steam.arquivo_steamcmd);
	printf("  Executavel do SRCDS:\n    %s\n", cfg.steam.arquivo_srcds);

	// GMod + Informações sobre cada content
	printf(" ________________________________________________\n");
	printf("[INFORMACAO DO SERVIDOR E DOS CONTENTS CARREGADOS]\n\n");
	printf("    ___________\n");
	printf("  + %s\n", cfg.gmod.nome );
	printf("    pasta = %s\n", ini.buscar(cfg.parametros, "", "pasta_servidor"));
	printf("    id = %s\n\n", cfg.gmod.id);
	contents = cfg.contents;
	while ((*contents).proxima_secao != NULL) {
		if (strcmp(ini.buscar(contents, (*contents).secao, "opcao"), "3") != 0) {
			printf("\n");
			if (strcmp(ini.buscar(contents, (*contents).secao, "opcao"), "1") == 0) {
				printf("  + %s\n", (*contents).secao);
				printf("    pasta = %s\n", ini.buscar(contents, (*contents).secao, "pasta"));
				printf("    login = %s\n", ini.buscar(contents, (*contents).secao, "login"));
				printf("    id    = %s\n", ini.buscar(contents, (*contents).secao, "id"));
				printf("    opcao = %s\n", ini.buscar(contents, (*contents).secao, "opcao"));
			} else if (strcmp(ini.buscar(contents, (*contents).secao, "opcao"), "2") == 0) {
				printf("  + %s\n", (*contents).secao);
				printf("    steam = %s\n", ini.buscar(contents, (*contents).secao, "steam"));
				printf("    opcao = %s\n", ini.buscar(contents, (*contents).secao,"opcao"));
			}
		}
		contents = (*contents).proxima_secao;
	}

	// O comando para abrir o servidor via terminal (por completo)
	printf(" ___________________\n");
	printf("[COMANDO DO SERVIDOR]\n\n");
	printf("    '%s' %s\n", cfg.steam.arquivo_srcds, ini.buscar(cfg.parametros, "", "comando_sv"));
	printf("\n");

	// Fim
	printf("______________________________________________________________________________\n");
	printf("\n");
	printf("\n");

	geral.pausar();
}

int criarPastasBase() {
	// Cria as pastas essenciais do GMBR DS
	// Retorno:
	// 1 = OK, 0 = Erro

	int j, retorno;
	char temp[MAX_CHAR_DIR];

	for (j=1;j<=4;j++) {
		// Pasta base
		if (j == 1)
			strcpy(temp, cfg.diretorio_base);
		// Pasta do SteamCMD
		else if (j == 2)
			strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_steamcmd"));
		// Pasta do GMod
		else if (j == 3)
			strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_servidor"));
		// Pasta dos contents
		else if (j == 4)
			strcpy(temp, ini.buscar(cfg.parametros, "", "pasta_contents"));

		// Tenta criar a pasta
		if ((retorno = geral.criarPasta(temp)) == 1)
			printf("[GMBR DS] Pasta criada: '%s'\n", temp);

		// Mensagem de erro geral na criação de pastas
		else if (retorno == 2) {
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("ERRO AO CRIAR PASTA '%s'\n", temp);
			printf("-----------------------------------------------------------------------\n");
			geral.pausar();

			return 0;
		}
	}

	return 1;
}

int criarPastasDosContents() {
	// Cria a pasta de cada content configurado
	// Retorno:
	// 1 = OK, 0 = Erro

	int retorno;
	char temp[MAX_CHAR_DIR];
	no* contentsAux = cfg.contents;

	// Checo os contents um por um
	while ((*contentsAux).proxima_secao != NULL) {
		// Está marcado para instalação?
		if (strcmp(ini.buscar(contentsAux, (*contentsAux).secao, "opcao"), "1") == 0) {
			// Sim

			// Pego a pasta do content
			strcpy(temp, ini.buscar(contentsAux, (*contentsAux).secao, "pasta"));

			// Tento criá-la
			if ((retorno = geral.criarPasta(temp)) == 1)
				printf("[GMBR DS] Pasta criada: '%s'\n", temp);
			else if (retorno == 2) {
				printf("\n[GMBR DS]--------------------------------------------------------------\n");
				printf("ERRO AO CRIAR PASTA '%s'\n", temp);
				printf("-----------------------------------------------------------------------\n");
				geral.pausar();

				return 0;
			}
		}

		// Próximo content
		contentsAux = (*contentsAux).proxima_secao;
	}

	return 1;
}

int instalarSteamCMD() {
	// Instala o SteamCMD
	// Retorno:
	// 1 = OK, 0 = Erro

	// Verifica se existe o executável do SteamCMD
	if ( ! geral.verificarExistencia(cfg.steam.arquivo_steamcmd)) {
		// Não

		char array[300], *partes, *parte_certa="", barra[2]="/";
		char arquivo[30], comando_download[300], comando_extrair[300];

		printf("\n[GMBR DS] Baixando SteamCMD em '%s'...\n\n", ini.buscar(cfg.parametros, "", "pasta_steamcmd"));

		// Pego o nome do arquivo que será baixado (a partir do link de download)
		//-----------------------------------------
		// Copio para "temp" o link de download
		strcpy(array, ini.buscar(cfg.parametros, "", "download_steamcmd"));
		// Explodo "temp" nas barras
		partes = strtok(array, barra);
		// Pego o nome do arquivo e salvo em "parte_certa" (obs: o loop vai até o final e "partes" fica nulo)
		while(partes != NULL) {
			parte_certa = partes;
			partes = strtok(NULL, barra);
		}
		//-----------------------------------------

		// Defino o caminho absoluto do arquivo na pasta do SteamCMD do GMBR DS
		strcat(strcat(strcpy(arquivo, ini.buscar(cfg.parametros, "", "pasta_steamcmd")), BARRA), parte_certa);

		// Removo algum download antigo do SteamCMD que possa existir nessa pasta
		if (geral.verificarExistencia(arquivo))
			remove(arquivo);

		// Faz o download do SteamCMD
		if (strcmp(SISTEMA,"Linux") == 0)
			strcpy(comando_download, "wget ");
		else if (strcmp(SISTEMA,"Windows") == 0)
			strcpy(comando_download, "bin\\wget ");
		strcat(strcat(strcat(strcat(comando_download, ini.buscar(cfg.parametros, "", "download_steamcmd")), " -P \""), ini.buscar(cfg.parametros, "", "pasta_steamcmd")), "\"");
		if ((system(comando_download)) != 0) {
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("ERRO AO BAIXAR O STEAMCMD\n");
			printf("O LINK DE DOWNLOAD ESTA FUNCIONANDO?\n");
			if (strcmp(SISTEMA,"Windows") == 0)
				printf("O PROGRAMA 'WGET.EXE' ESTA NA PASTA CFG?\n");
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;
		}

		// Extrai o zip
		if (strcmp(SISTEMA,"Linux") == 0)
			strcat(strcat(strcat(strcat(strcat(strcat(strcpy(comando_extrair, "tar -xvzf \""), arquivo), "\""), " -C "), "\""), ini.buscar(cfg.parametros, "", "pasta_steamcmd")), "\"");
		else if (strcmp(SISTEMA,"Windows") == 0)
			strcat(strcat(strcat(strcat(strcat(strcat(strcpy(comando_extrair, "bin\\unzip.exe \""), arquivo), "\""), " -d "), "\""), ini.buscar(cfg.parametros, "", "pasta_steamcmd")), "\"");
		if ((system(comando_extrair)) != 0) {
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("ERRO AO EXTRAIR O STEAMCMD\n");
			if (strcmp(SISTEMA,"Windows") == 0)
				printf("O PROGRAMA 'UNZIP.EXE' ESTA NA PASTA CFG?\n");
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;
		}
	}

	printf("\n[GMBR DS] O SteamCMD esta instalado\n");

	return 1;
 }

int instalarAtualizarAux(char id[], char pasta[], char login[]) {
	// Instala o GMod e os contents
	// Retorno:
	// 1 = Instalação bem sucedida, 0 = Erro

	char parte[12], comando[300];

	// Crio o comando para instalar/atualizar o servidor/contents
	if (strcmp(SISTEMA,"Linux") == 0)
		strcpy(parte, "\" && \"");
	else if (strcmp(SISTEMA,"Windows") == 0)
		strcpy(parte, "\" & \"");
	strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcpy(
	comando, "cd \""), ini.buscar(cfg.parametros, "", "pasta_steamcmd")), parte), cfg.steam.arquivo_steamcmd),
	"\" +login "), login), " +force_install_dir "), "\""), pasta), "\""), " +app_update "), id), " validate +quit");

	// Mostro o comando completo para o usuário
	printf("[GMBR DS] %s\n\n", comando);

	// Inicio o processo
	if ((system(comando)) != 0) {
		printf("\n[GMBR DS] Erro! Verifique as ultimas mensagens de problemas e o contents.ini\n");

		return 0;
	}

	return 1;
}

int instalarAtualizarSv() {
	// Instalação/Atualização do servidor de GMod
	// Retorno:
	// 1 = Instalação bem sucedida, 0 = Erro

	char pasta[MAX_CHAR_DIR];

	printf("[GMBR DS] Instalando/atualizando '%s'...\n\n", cfg.gmod.nome);

	// Pasta
	strcpy(pasta, ini.buscar(cfg.parametros, "", "pasta_servidor"));

	// Instalação
	if ( ! instalarAtualizarAux(cfg.gmod.id, pasta, "anonymous"))
		return 0;

	return 1;
}

int instalarAtualizarContents() {
	// Instalação/Atualização dos contents com opção em 1
	// Retorno:
	// 1 = Instalação bem sucedida, 0 = Erro

	no* contentsAux = cfg.contents;
	char login[30], pasta[MAX_CHAR_DIR], id[10];

	while ((*contentsAux).proxima_secao != NULL) {
		if (strcmp(ini.buscar(contentsAux, (*contentsAux).secao, "opcao"), "1") == 0) {

			printf("\n[GMBR DS] Instalando/atualizando '%s'...\n\n", (*contentsAux).secao);

			// Login
			if (strcmp(ini.buscar(cfg.contents, (*contentsAux).secao, "login"), "Usuario") == 0) {
				printf("[GMBR DS] Para que esse download de content funcione, a \"login_steam\" do cfg.ini deve estar com login outomatico no Steam ou ja logada no Steam\n\n");
				strcpy(login, ini.buscar(cfg.parametros, "", "login_steam"));
			} else
				strcpy(login, "anonymous");

			// Pasta
			strcpy(pasta, ini.buscar(contentsAux, (*contentsAux).secao, "pasta"));

			// ID
			strcpy(id, ini.buscar(contentsAux, (*contentsAux).secao, "id"));

			// Instalação
			if ( ! instalarAtualizarAux(id, pasta, login))
				return 0;
		}

		contentsAux = (*contentsAux).proxima_secao;
	}

	return 1;
}

int montarContents() {
	// Escreve as informações dos contents no arquivo de montagem do GMod
	// Retorno:
	// 1 = OK, 0 = Erro

	FILE *fp;
	no* contentsAux = cfg.contents;

	// Verifica se o arquivo de montagem existe
	if ( ! geral.verificarExistencia(cfg.gmod.arquivo_mount)) {
		// Não
		printf("\n[GMBR DS]--------------------------------------------------------------\n");
		printf("NAO FOI POSSIVEL MONTAR OS CONTENTS! ARQUIVO NAO ENCONTRADO:\n");
		printf("'%s'\n", cfg.gmod.arquivo_mount);
		printf("-----------------------------------------------------------------------\n\n");
		geral.pausar();

		return 0;
	} else {
		// Sim

		// Removo o arquivo de montagem
		remove(cfg.gmod.arquivo_mount);

		// Vou criar um novo
		fp = fopen(cfg.gmod.arquivo_mount, "w");

		// Verifica se houve erro ao rodar o "fopen", logo acima
		if (fp == NULL) {
			printf("\n[GMBR DS]--------------------------------------------------------------\n");
			printf("NAO FOI POSSIVEL MONTAR OS CONTENTS! ERRO AO ABRIR O ARQUIVO:\n");
			printf("'%s'\n", cfg.gmod.arquivo_mount);
			printf("-----------------------------------------------------------------------\n\n");
			geral.pausar();

			return 0;

		// Verifica se há contents para montar
		} else if ((*contentsAux).proxima_secao == NULL) {
			// Não
			printf("\n[GMBR DS] Nao ha contents para montar\n");

			return 1;
		}
			// Sim

		// Montagem de contents (opção em 1 e 2)
		fprintf(fp, "\"mountcfg\"%s{%s", PULO, PULO);
		while ((*contentsAux).proxima_secao != NULL) {
			if (strcmp(ini.buscar(contentsAux, (*contentsAux).secao, "opcao"), "1") == 0)
				fprintf(fp, "\t\"%s\" \"%s\"%s", (*contentsAux).secao, ini.buscar(contentsAux, (*contentsAux).secao, "pasta"), PULO);
			else if (strcmp(ini.buscar(contentsAux, (*contentsAux).secao, "opcao"), "2") == 0)
				fprintf(fp, "\t\"%s\" \"%s\"%s", (*contentsAux).secao, ini.buscar(contentsAux, (*contentsAux).secao, "steam"), PULO);
			contentsAux = (*contentsAux).proxima_secao;
		}
		fprintf(fp, "}");
	}

	fclose(fp);

	printf("\n[GMBR DS] Contents montados\n");

	return 1;
}

void ligarServer() {
	// Envio o comando de abertura do servidor
	// Retorno:
	// 1 = OK, 0 = Erro

	char temp[MAX_CHAR_DIR];

	CLRSCR;
	printf("[GMBR DS] Ligando o servidor...\n");

	// Mostro o comando completo para o usuário
	printf("[GMBR DS] \"%s\" %s\n\n", cfg.steam.arquivo_srcds, ini.buscar(cfg.parametros, "", "comando_sv"));

	// Abro o servidor de GMod
	if (strcmp(SISTEMA,"Linux") == 0) {
		strcpy(temp, strcat(strcat(strcat(strcat(strcpy(temp, "\""), cfg.steam.arquivo_srcds), "\""), " "), ini.buscar(cfg.parametros, "", "comando_sv")));
		system(temp);
	} else if (strcmp(SISTEMA,"Windows") == 0)
		ShellExecute(NULL, "open", cfg.steam.arquivo_srcds, ini.buscar(cfg.parametros, "", "comando_sv"), NULL, SW_SHOWDEFAULT);
}

void finalizarOpcao() {
	// Finalização genérica de opções do menu
	printf("\n");
	printf("\n");
	printf(" ___________________________________\n");
	printf("|                                   |\n");
	printf("|    [GMBR DS] Opcao finalizada.    |\n");
	printf("|                                   |\n\n");

	geral.pausar();
}

void sair() {
	// Sair

	// Removo as as leituras dos arquivos ini da memória
	ini.limpar(cfg.parametros);
	ini.limpar(cfg.contents);

	exit(0);
}

int main() {
	// Limpar a tela
	CLRSCR;

	// Pegar pasta onde está localizado o executável do GMBR DS
	if ( ! obterPastaCorrente())
		sair();

	// Ajustes básicos
	ajustarConfiguracoesBasicas();

	// Pegar as configurações gerais em arquivo .ini
	if (( ! carregarCfgIni()) ||
	// Pegar as configurações de contents em arquivo .ini
		( ! carregarContentsIni()) ||
	// Achar o Steam
		( ! encontrarSteam()))
		sair();

	// Dar às variáveis dos arquivos .ini os seus valores completos
	ajustarVariaveis();

	// Buscar/Salvar caminhos de bibliotecas de jogos do Steam
	if (( ! buscarBibliotecas()) ||
	// Verificar se os contents são válidos
		( ! validarContents()))
		sair();

	// Menu e procedimentos
	while (1) {
		// Apresentação, menu e tela de exibição das configurações carregadas
		int escolha = exibirTelaInicial();

		// Escolhas
		switch(escolha)
		{
			// Sair
			case 0:
				sair();
			break;
			// Exibir as configurações carregadas
			case 1:
				exibirConfiguracoes();
			// Instalar/atualizar servidor de GMod
			break;
			case 2:
				if (( ! criarPastasBase()) ||
					( ! instalarSteamCMD()) ||
					( ! instalarAtualizarSv()) ||
					( ! montarContents()))
					finalizarOpcao();
				else
					finalizarOpcao();
			break;
			// Instalar/atualizar contents
			case 3:
				if (( ! criarPastasBase()) ||
					( ! criarPastasDosContents()) ||
					( ! instalarSteamCMD()) ||
					( ! instalarAtualizarContents()) ||
					( ! montarContents()))
					finalizarOpcao();
				else
					finalizarOpcao();
			break;
			// Instalar/atualizar servidor de GMod e contents
			case 4:
				if (( ! criarPastasBase()) ||
					( ! criarPastasDosContents()) ||
					( ! instalarSteamCMD()) ||
					( ! instalarAtualizarSv()) ||
					( ! instalarAtualizarContents()) ||
					( ! montarContents()))
					finalizarOpcao();
				else
					finalizarOpcao();
			break;
			// Ligar servidor
			case 5:
				ligarServer();
				if (strcmp(SISTEMA,"Windows") == 0) // Ruim. Seria melhor se voltasse para o GMBR DS.
					sair();
			break;
			// Forçar montagem de contents
			case 6:
				montarContents();
				finalizarOpcao();
			break;
		}

		// Reiniciando ciclo da escolha
		escolha = -1;
	}

	return 0;
}
