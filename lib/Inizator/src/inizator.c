#include "inizator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if __linux__
	#define SISTEMA "Linux"
#elif _WIN32
	#define SISTEMA "Windows"
#endif

static int limpar(no* no_atual);

static no* alocar() {
	/*
	 * Aloca um novo nó para uso na estrutura de leitura do ini
	 *
	 * RETORNO:
	 * no* = Sucesso,
	 * NULL = Falhou
	 *
	 */

	no* alocacao;

	if ((alocacao = malloc(sizeof(no))) == NULL) {
		printf("\n[Inizator] Erro ao tentar alocar memoria para um no. (Ela esta muito cheia?)\n");
		return NULL;
	}

	return alocacao;
}

static int ler_fix(int c, FILE *fp) {
	/*
	 * HACK - Isso serve para limpar um lixo que aparece no início da leitura do arquivo!
	 * A função procura por um '#', descartando tudo que vem antes e sua linha.
	 * Nunca mais vi esse problema mas vou deixar a função aqui. Não entendo a causa.
	 * Adicionar os parâmetros de uma seção
	 *
	 * ENTRADA:
	 * c = Caractere que procuraremos para começar a descartar a linha inteira;
	 * fp = Ponteiro do arquivo ini que está sendo lido,
	 *
	 * RETORNO:
	 * 0 = Não foi fornecido o caractere #, portanto a limpeza não foi feita
	 * 1 = Documento "limpo"
	 *
	 */

	if (c == '#')
		while ((c = fgetc(fp)) != EOF)
			if (c == '\n')
				return 1;

	return 0;
}

static void ler_comentario(FILE *fp) {
	/*
	 * Ignorar linhas que iniciaram com o caractere de comentário
	 *
	 * ENTRADA:
	 * fp = Ponteiro do arquivo ini que está sendo lido,
	 *
	 */

	int c;

	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n')
			break;
	}
}

static int ler_secao(FILE *fp, no **inicio, no **topo, no **ponteiro1, no **ponteiro2) {
	/*
	 * Adicionar os parâmetros de uma seção
	 *
	 * ENTRADA:
	 * fp = Ponteiro do arquivo ini que está sendo lido,
	 * inicio = Início da estrutura ini,
	 * topo = Topo da atual seção,
	 * ponteiro1 = Anda pelos parâmetros,
	 * ponteiro2 = Anda pelos parâmetros junto com o ponteiro 1,
	 *
	 * RETORNO:
	 * -1 = Erro,
	 * 0 = Seção lida sem sucesso, parâmetros não podem entrar,
	 * 1 = Seção lida com sucesso, parâmetros podem entrar
	 *
	 */

	int tamanho = 0, ler_parametros = 1, c;
	char caracteres[MAX_CHAR_PARAMETRO_VALOR];

	// Nome da seção
	while ((c = fgetc(fp)) != EOF) {
		// Coleto o nome da seção
		if (c != ']') {
			caracteres[tamanho++] = c;

			// Se "tamanho" ficar grande demais, sou obrigado a cortar a linha
			if (tamanho == MAX_CHAR_SECAO) {
				caracteres[tamanho--] = '\0';
				printf("\n[Inizator] A seguinte secao passou o limite de '%d' caracteres:\n%s\n\n", MAX_CHAR_SECAO, caracteres);
				while ((c = fgetc(fp)) != EOF)
					if (c == '\n')
						break;
				c = ']';
			}

			// Erro! Cheguou no final da linha sem fechar a seção
			if (c == '\n')
				return -1;
		} else {
			// Fecho o nome da seção
			caracteres[tamanho] = '\0';

			// Se for a primeira seção, insiro dessa forma
			if (strcmp((**ponteiro1).secao, "inicio") == 0) {
				strcpy((**ponteiro1).parametro, "");
				strcpy((**ponteiro1).valor, "");
				(**ponteiro1).proximo_parametro = NULL;
				(**ponteiro1).proxima_secao = NULL;
			}

			// Para as demais seções:
			if (strcmp((**ponteiro1).secao, "inicio") != 0) {

				// Verifico se a seção já existe. Se sim, ignora ela e todos os seus parâmetros/valores
				int duplicata = 0;
				no* verificar = *inicio;
				while (1) {
					if (strcmp((*verificar).secao, caracteres) == 0) {
						printf("\n[Inizator] Detectada secao '%s' duplicada. Ignorando-a...\n", caracteres);
						ler_parametros = 0;
						duplicata = 1;
						tamanho = 0;
						break;
					}
					if ((*verificar).proxima_secao != NULL)
						verificar = (*verificar).proxima_secao;
					else
						break;
				}
				if (duplicata)
					break;

				// Crio um nó para finalizar seção anterior
				if((*ponteiro1 = alocar()) == NULL) {
					limpar(*inicio);
					return -1;
				}
				(**ponteiro2).proximo_parametro = *ponteiro1;
				(**ponteiro1).proximo_parametro = NULL;
				(**ponteiro1).proxima_secao = NULL;

				// Crio o primeiro nó da seção atual
				if((*ponteiro1 = alocar()) == NULL) {
					limpar(*inicio);
					return -1;
				}
				(**topo).proxima_secao = *ponteiro1;
				(**ponteiro1).proximo_parametro = NULL;
				(**ponteiro1).proxima_secao = NULL;
				*ponteiro2 = *ponteiro1;
				*topo = *ponteiro1;
			}

			// Guardo o nome da seção
			strcpy((**ponteiro1).secao, caracteres);
			tamanho = 0;
			break;
		}
	}

	return ler_parametros;
}

static int ler_parametrosDaSecao(FILE *fp, char letra_inicial, no **inicio, no **topo, no **ponteiro1, no **ponteiro2) {
	/*
	 * Adicionar os parâmetros de uma seção
	 *
	 * ENTRADA:
	 * fp = Ponteiro do arquivo ini que está sendo lido,
	 * letra_inicial = Letra que começa o nome da seção (é mais fácil pegar ela logo assim),
	 * inicio = Início da estrutura ini,
	 * topo = Topo da atual seção,
	 * ponteiro1 = Anda pelos parâmetros,
	 * ponteiro2 = Anda pelos parâmetros junto com o ponteiro 1,
	 *
	 * RETORNO:
	 * 1 = Sucesso.
	 * 0 = Falhou
	 *
	 */

	int tamanho = 0, c;
	char caracteres[MAX_CHAR_PARAMETRO_VALOR];

	// Coloco a primeira letra do parâmetro
	caracteres[tamanho] = letra_inicial;

	while ((c = fgetc(fp)) != EOF) {
		// Vou coletando o resto do parâmetro
		if (c != '=')
			caracteres[++tamanho] = c;

		// Se "tamanho" ficar grande demais, sou obrigado a cortar a linha
		if (tamanho == MAX_CHAR_PARAMETRO_NOME) {
			caracteres[tamanho--] = '\0';
			printf("\n[Inizator] O seguinte parametro passou o limite de '%d' caracteres:\n%s\n\n",
					MAX_CHAR_PARAMETRO_NOME, caracteres);
			while ((c = fgetc(fp)) != EOF)
				if (c == '\n')
					break;
			c = '=';
		}

		if (c == '=') {
			// Removo os espacos em branco no final do nome do parâmetro (caso existam)
			if (caracteres[tamanho] == ' ')
				while (caracteres[tamanho - 1] == ' ')
					tamanho--;

			// Fecho o array
			caracteres[tamanho++] = '\0';

			// Verifico se o parâmetro já existe na seção. Se sim, ignoro ele e seu valor
			int duplicata = 0;
			no* verificar = *topo;
			while (1) {
				if (strcmp((*verificar).parametro, caracteres) == 0) {
					printf("\n[Inizator] Detectado parametro '%s' duplicado na secao '%s'. Ignorando-o...\n\n",
							caracteres, (*verificar).secao);
					duplicata = 1;
					tamanho = 0;
					break;
				}
				if ((*verificar).proximo_parametro != NULL)
					verificar = (*verificar).proximo_parametro;
				else
					break;
			}
			if (duplicata)
				break;

			// Alocação de novos nós de parâmetro
			if (strcmp((**ponteiro1).parametro, "") != 0) {
				if((*ponteiro1 = alocar()) == NULL) {
					limpar(*inicio);
					return 0;
				}
				(**ponteiro2).proximo_parametro = *ponteiro1;
				strcpy((**ponteiro1).secao, (**topo).secao);
				(**ponteiro1).proxima_secao = NULL;
				(**ponteiro1).proximo_parametro = NULL;
				*ponteiro2 = *ponteiro1;
			}

			// Guardo o parâmetro
			strcpy((**ponteiro1).parametro, caracteres);
			tamanho = 0;

			// Valor do parâmetro
			while ((c = fgetc(fp)) != EOF) {
				if (c != ' ') {
					// Coleto a primeira letra do valor do parâmetro
					caracteres[tamanho] = c;

					// Se não tiver um valor, copio uma string vazia para ele e vou embora
					if (c == '\n') {
						strcpy((**ponteiro1).valor, "");
						tamanho = 0;
						break;
					} else {
						while ((c = fgetc(fp)) != EOF) {
							// Vou coletando o resto do valor do parâmetro
							if (c != '\n')
								caracteres[++tamanho] = c;

							// Se "tamanho" ficar grande demais, sou obrigado a cortar a linha
							if (tamanho == MAX_CHAR_PARAMETRO_VALOR) {
								caracteres[--tamanho] = '\0';
								printf("\n[Inizator] O seguinte valor de parametro passou o limite de '%d' caracteres:\n%s\n",
										MAX_CHAR_PARAMETRO_VALOR, caracteres);
								while ((c = fgetc(fp)) != EOF)
									if (c == '\n')
										break;
							}
							if (c == '\n') {
								// Removo os espaços em branco no final do valor do parâmetro (caso existam)
								if ((caracteres[tamanho - 1] == ' ') || (caracteres[tamanho] == ' '))
									while ((caracteres[tamanho - 1] == ' ') || (caracteres[tamanho] == ' '))
										tamanho--;
								if (strcmp(SISTEMA,"Linux") == 0)
									caracteres[tamanho] = '\0';
								else if (strcmp(SISTEMA,"Windows") == 0)
									caracteres[tamanho+1] = '\0';

								// Guardo o valor do parâmetro
								strcpy((**ponteiro1).valor, caracteres);
								tamanho = 0;
								break;
							}
						}
						break;
					}
				}
			}
		}

		if (c == '\n') {
			tamanho = 0; // Preciso disso aqui para evitar erros por parte do usuário
			break;
		}
	}

	return 1;
}

static int ler_finalizar(no **inicio, no **topo, no **ponteiro1, no **ponteiro2) {
	/*
	 * Finalizar seção atual adicionando parâmetro vazio e próxima seção vazia
	 *
	 * ENTRADA:
	 * inicio = Início da estrutura ini,
	 * topo = Topo da atual seção,
	 * ponteiro1 = Anda pelos parâmetros,
	 * ponteiro2 = Anda pelos parâmetros junto com o ponteiro 1,
	 *
	 * RETORNO:
	 * 0 = Falha,
	 * 1 = Sucesso.
	 *
	 */

	// Esse nó finaliza os nós criados na última seção
	if((*ponteiro1 = alocar()) == NULL) {
		limpar(*inicio);
		return 0;
	}
	(**ponteiro2).proximo_parametro = *ponteiro1;
	(**ponteiro1).proximo_parametro = NULL;
	(**ponteiro1).proxima_secao = NULL;

	// Esse nó finaliza a última seção
	if((*ponteiro1 = alocar()) == NULL) {
		limpar(*inicio);
		return 0;
	}
	(**topo).proxima_secao = *ponteiro1;
	(**ponteiro1).proximo_parametro = NULL;
	(**ponteiro1).proxima_secao = NULL;

	return 1;
}

static no* ler(char arquivo[]) {
	/*
	 * Lê o arquivo ini e gera uma esturuta de nós contendo todos os seus dados
	 *
	 * ENTRADA:
	 * arquivo = Nome do arquivo ini a ser aberto
	 *
	 * RETORNO:
	 * no* = Sucesso,
	 * NULL = Falha
	 *
	 */

	int c, ler_parametros = 0;
	//int fix = 0; /* Desativado */
	no *inicio, *topo, *ponteiro1, *ponteiro2;
	FILE * fp = fopen(arquivo, "r");

	if (fp == NULL)
		return NULL;

	// Inicialização dos ponteiros no* no primeiro nó
	if ((inicio = topo = ponteiro1 = ponteiro2 = alocar()) == NULL)
		return NULL;

	// Indicativo para reconhecer a primeira seção
	strcpy((*inicio).secao, "inicio");

	// Início da leitura
	while ((c = fgetc(fp)) != EOF) {
		/*
		// FIX - Isso limpa um lixo que aparece no começo do input
		if (fix == 0)
			fix = ler_fix(c, fp);
		*/

		// Pular comentários
		if (c == ';')
			ler_comentario(fp);

		// Criar/Fechar seção
		if (c == '[')
			if ((ler_parametros = ler_secao(fp, &inicio, &topo, &ponteiro1, &ponteiro2)) == -1)
				return NULL;

		// Pular caracteres inconvenientes
		if ((c == ' ') || (c == '\n') || (c == '\t'))
			continue;

		// Adicionar os parâmetros de uma seção
		if (ler_parametros == 1)
			if ( ! ler_parametrosDaSecao(fp, c, &inicio, &topo, &ponteiro1, &ponteiro2))
				return NULL;
	}

	fclose(fp);

	// Finalizar seção atual adicionando parâmetro vazio e próxima seção vazia
	if ( ! ler_finalizar(&inicio, &topo, &ponteiro1, &ponteiro2))
		return NULL;

	return inicio;
}

static void imprimir(no *inicio, char secao[]) {
	/*
	 * Imprime as informações coletadas no arquivo ini
	 *
	 * ENTRADA:
	 * inicio = Nó a partir do qual a impressão iniciará,
	 * secao = Nome da seção a imprimir (entrar com "" para imprimir todas as seções)
	 *
	 */

	// Evito que rode a função em um nó nulo
	if (inicio == NULL)
		return;

	no* no_secao = inicio;
	no* no_parametro = inicio;

	// Enquanto houver seções...
	while ((*no_secao).proxima_secao != NULL) {
		// Se eu estiver na seção buscada ou ela não importar...
		if ((strcmp((*no_parametro).secao, secao) == 0) || (strcmp(secao, "") == 0)) {
			// Imprimo a seção
			printf("\n[%s]\n", (*no_secao).secao);

			// Enquanto houver parâmetros...
			while ((*no_parametro).proximo_parametro != NULL) {
				// Imprimo os parâmetros e seus valores
				printf("	%s = %s\n", (*no_parametro).parametro, (*no_parametro).valor);
				no_parametro = (*no_parametro).proximo_parametro;
			}

			// Ao acabar a seção, paro tudo se secao[] não for ""
			if (strcmp(secao, "") != 0)
				break;
		}

		// Continuo a ler as seções se secao[] for ""
		no_secao = (*no_secao).proxima_secao;
		no_parametro = no_secao;
	}
}

static char* buscar(no *inicio, char secao[], char termo[]) {
	/*
	 * Checa se um dado parâmetro está expresso no arquivo ini
	 *
	 * ENTRADA:
	 * inicio = Primeiro nó de uma estrutura de arquivo ini,
	 * secao = Nome da seção onde haverá a busca (entrar com "" para buscar em todas as seções)
	 * termo = Parâmetro buscado,
	 *
	 * RETORNO:
	 * char* = Valor do termo encontrado,
	 * NULL = Termo não encontrado
	 *
	 */

	// Evito que rode a função em um nó nulo
	if (inicio == NULL)
		return NULL;

	no* no_secao = inicio;
	no* no_parametro = inicio;

	// Enquanto houver seções...printf("[GMBR DS]Aqui, oh!\n");
	while ((*no_secao).proxima_secao != NULL) {
		// Se eu estiver na seção buscada ou ela não importar...
		if ((strcmp((*no_parametro).secao, secao) == 0) || (strcmp(secao, "") == 0)) {
			// Enquanto houver parâmetros...
			while ((*no_parametro).proximo_parametro != NULL) {
				// Se estivermos no parâmetro certo, retornamos ele
				if (strcmp((*no_parametro).parametro, termo) == 0)
					return (*no_parametro).valor;

				// Se não, vamos continuar buscando
				no_parametro = (*no_parametro).proximo_parametro;
			}

			// Ao acabar a seção, paro tudo se secao[] não for ""
			if (strcmp(secao, "") != 0)
				break;
		}

		// Continuo a ler as seções se secao[] for ""
		no_secao = (*no_secao).proxima_secao;
		no_parametro = no_secao;
	}

	return NULL;
}

static int alterar(no *inicio, char secao[], char parametro[], char valor[]) {
	/*
	 * Busco por um parâmetro e altero o valor dele
	 *
	 * ENTRADA:
	 * inicio = Primeiro nó de uma estrutura de arquivo ini,
	 * secao = Seção do parâmetro a ser alterado (entrar com "" em secao[] para alterar toda ocorrência desse parâmetro),
	 * parametro = Nome do parâmetro que será alterado,
	 * valor = Novo valor do parâmetro
	 *
	 * RETORNO:
	 * Valor positivo qualquer = Sucesso (é a quantidade de valores alterados),
	 * 0 = Nada foi alterado,
	 * -1 = o valor de entrada é grande demais
	 *
	 */

	// Evito que rode a função em um nó nulo
	if (inicio == NULL)
		return 0;

	no* no_secao = inicio;
	no* no_parametro = inicio;
	int i = 0;

	// Checo se parametro_valor[] não é grande demais
	while (valor[i] != '\0')
		i++;
	if (i >= MAX_CHAR_PARAMETRO_VALOR)
		return -1;

	i = 0;
	// Enquanto houver seções...
	while ((*no_secao).proxima_secao != NULL) {
		// Se eu estiver na seção buscada ou ela não importar...
		if ((strcmp((*no_parametro).secao, secao) == 0) || (strcmp(secao, "") == 0)) {
			// Enquanto houver parâmetros...
			while ((*no_parametro).proximo_parametro != NULL) {
				// Se for o parâmetro que eu busco...
				if (strcmp((*no_parametro).parametro, parametro) == 0) {
					// Mudo o valor dele e paro de checar essa sequência de parâmetros
					strcpy((*no_parametro).valor, valor);
					i ++;
					break;
				}

				// Se não for o parâmetro que eu busco, vejo se é o próximo
				no_parametro = (*no_parametro).proximo_parametro;
			}

			// Se "" não estiver marcado, paro de ler as seções
			if (strcmp(secao, "") != 0)
				break;
		}

		// Se eu não estiver na seção certa ou ela não importar, vou para a próxima seção
		no_secao = (*no_secao).proxima_secao;
		no_parametro = no_secao;
	}
	return i;
}

static int inserir(no *inicio, char secao[], char parametro[], char valor[]) {
	/*
	 * Insere um novo parâmetro em dada seção
	 *
	 * ENTRADA:
	 * inicio = Primeiro nó de uma estrutura de arquivo ini,
	 * secao = Seção do parâmetro a ser inserido,
	 * parametro = Nome do parâmetro que será inserido,
	 * valor = Valor do parâmetro.
	 *
	 * RETORNO:
	 * 0 = Falha,
	 * 1 = Sucesso.
	 *
	 */

	// Evito que rode a função em um nó nulo
	if (inicio == NULL)
		return 0;

	no *no_secao = inicio;
	no *no_parametro = inicio;
	no *secao_anterior, *no_anterior, *novo;

	// Crio o nó que será inserido
	if((novo = alocar()) == NULL)
		return 0;

	// Guardo as informações no novo nó
	strcpy((*novo).secao, secao);
	strcpy((*novo).parametro, parametro);
	strcpy((*novo).valor, valor);
	(*novo).proxima_secao = NULL;
	(*novo).proximo_parametro = NULL;

	// Tento inserir o novo nó em uma seção existente:

	while ((*no_secao).proxima_secao != NULL) {
		// Se eu estiver na seção dita
		if (strcmp((*no_parametro).secao, secao) == 0) {
			// Busco o nó final e o nó nulo
			while ((*no_parametro).proximo_parametro != NULL) {
				no_anterior = no_parametro;
				no_parametro = (*no_parametro).proximo_parametro;
			}

			// Conecto o nó final com o novo nó
			(*no_anterior).proximo_parametro = novo;

			// Conecto o novo nó com o nó nulo
			(*novo).proximo_parametro = no_parametro;

			return 1;
		}

		// Continuo a ler as seções se secao[] for ""
		secao_anterior = no_secao;
		no_secao = (*no_secao).proxima_secao;
		no_parametro = no_secao;
	}

	// Insiro o nó em uma nova seção:

	// Conecto a seção anterior com o novo nó
	(*secao_anterior).proxima_secao = novo;

	// Conecto o novo nó com a seção nula
	(*novo).proxima_secao = no_secao;

	return 1;
}

static int limpar(no* no_atual) {
	/* Libera todos os nós que foram alocados na leitura do ini
	 *
	 * ENTRADA:
	 * no_atual = O primeiro nó de uma estrutura de arquivo ini - que recursavamente irá sendo limpo.
	 *
	 * RETORNO:
	 * 0 = Falha,
	 * 1 = Sucesso.
	 *
	 */

	// Evito que rode a função em um nó nulo
	if (no_atual == NULL)
		return 0;

	// Mudo para próximos parâmetros
	if ((*no_atual).proximo_parametro != NULL)
		limpar((*no_atual).proximo_parametro);

	// Mudo para próximas seções
	if ((*no_atual).proxima_secao != NULL)
		limpar((*no_atual).proxima_secao);

	// Limpo o que estiver aberto
	free(no_atual);

	return 1;
}

// [Classe] Exposição das funções necessárias para os usuários
struct inizator_h ini = {
	.ler = &ler,
	.imprimir = &imprimir,
	.buscar = &buscar,
	.alterar = &alterar,
	.inserir = &inserir,
	.limpar = &limpar
};
