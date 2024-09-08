# Biblioteca ini.*

Criada para ler e manipular informações de arquivos .ini.

Plataformas: Windows e Linux.


## Estrutura do arquivo .ini

É da seguinte forma que o programa espera o texto e o transforma:

```ini
[Seção 1]

; Isso é um comentário, vai sumir
; Outro comentário

País = Brasil
Logradouro_1 = Rua Imaginária, 7
Cor favorita = Azul


[Seção 2]

; Comentário 3

p1 = NOPE
p2 = NOPE2



[Seção 3]

; Outro comentário

link = http://site.com
PASTA = /home/teste
estado = Roraima


[Seção -1]

Hu3-Life = Half-Life 3

[Ultima seção]

parametro1 = true
parametro final = false
```

Estrutura:

![Imagem da estrutura](https://i.imgur.com/ls86FWf.png)

## Funções disponíveis

ini* **ler** (char arquivo[])
```
	Lê o arquivo ini e gera uma esturuta de nós contendo todos os seus dados

	ENTRADA:
	arquivo = Nome do arquivo ini a ser aberto

	RETORNO:
	ini* = Sucesso,
	NULL = Falha
```

void **imprimir** (ini* inicio, char secao[])
```
	Imprime as informações coletadas no arquivo ini

	ENTRADA:
	inicio = Nó a partir do qual a impressão iniciará,
	secao = Nome da seção a imprimir (entrar com "" para imprimir todas as seções)
```

char* **buscar** (ini* inicio, char secao[], char termo[])
```
	Checa se um dado parâmetro está expresso no arquivo ini

	ENTRADA:
	inicio = Primeiro nó de uma estrutura de arquivo ini,
	secao = Nome da seção onde haverá a busca (entrar com "" para buscar em todas as seções)
	termo = Parâmetro buscado,
	 
	RETORNO:
	char* = Valor do termo encontrado,
	NULL = Termo não encontrado
```

int **alterar** (ini* inicio, char secao[], char parametro[], char valor[])
```
	Busco por um parâmetro e altero o valor dele

	ENTRADA:
	inicio = Primeiro nó de uma estrutura de arquivo ini,
	secao = Seção do parâmetro a ser alterado (entrar com "" em secao[] para alterar toda ocorrência desse parâmetro),
	parametro = Nome do parâmetro que será alterado,
	valor = Novo valor do parâmetro

	RETORNO:
	Valor positivo qualquer = Sucesso (é a quantidade de valores alterados),
	0 = Nada foi alterado,
	-1 = o valor de entrada é grande demais
```
  
int **inserir** (ini* inicio, char secao[], char parametro[], char valor[])
```
	Insere um novo parâmetro em dada seção

	ENTRADA:
	inicio = Primeiro nó de uma estrutura de arquivo ini,
	secao = Seção do parâmetro a ser inserido,
	parametro = Nome do parâmetro que será inserido,
	valor = Valor do parâmetro.

	RETORNO:
	0 = Falha,
	1 = Sucesso.
```

int **limpar** (ini* no_atual)
```
	Libera todos os nós que foram alocados na leitura do ini

	ENTRADA:
	no_atual = Entrar com primeiro nó de uma estrutura de arquivo ini. Recursavamente ele vai sendo limpo.

	RETORNO:
	0 = Falha,
	1 = Sucesso.
```

## Utilização

1) Clone o repositório e inicialize os submódulos:

```sh
$ git clone https://github.com/xalalau/Inizator.git
$ cd Inizator
$ git submodule init
$ git submodule update
```

2) Importe o arquivo **inizator.h**;
3) Crie o ponteiro de **no** com **ini.ler()**;
4) Faça as operações que quiser com o tempo termo **ini.** e com os #define;
5) Antes de fechar o seu programa, desaloque a leitura do ini com **ini.limpar()**.

Assim, por exemplo:

```C
#include "inizator.h"
#include <stdio.h>

int main() {
	// Leitura
	no* cfgs;
	if((cfgs = ini.ler("./meu_arquivo.ini")) == NULL) {
		printf("ERRO DE LEITURA!");
		return 0;
	}
	
	// Operação de impressão
	ini.imprimir(cfgs, "");
	
	// Limpeza
	ini.limpar(cfgs);
	
	return 1;
}
```

## Exemplo de uso mais amplo

Copie o código logo abaixo em um arquivo **teste.c** dentro da pasta **src** e teste:

```C
#include "inizator.h"
#include <stdio.h>

int main() {
	/*
	 * Exemplos de uso do Inizator.
	 * 
	 */

	no* leitura = NULL, *leitura2 = NULL;
	char* resultado_busca;
	char termo1[30] = "parametro final";
	char secao1[30] = "Ultima seção";
	char termo2[30] = "parametro2";
	char secao2[30] = "Seção 3";
	char arquivo[100] = "../cfg_exemplo.ini";
	//char arquivo[100] = "../cfg_exemplo2.ini";

	// Leitura
	if((leitura = ini.ler(arquivo)) == NULL) {
		printf("ERRO DE LEITURA!");
		return 0;
	}

	// Impressões

	printf("\n\n####### Realizando impressões:\n");

	printf("\nImpressão geral:\n");
	ini.imprimir(leitura, "");
	printf("\n------------------\n\nImprimindo apenas a Seção 2:\n");
	ini.imprimir(leitura, "Seção 2");

	printf("\n_____________________________________________\n");

	// Buscas

	printf("\n\n####### Realizando buscas:\n");

	if ((resultado_busca = ini.buscar(leitura, secao1, termo1)) == NULL)
		printf("\n'%s' nao foi encontrado pela busca em '%s'!\n", termo1, secao1);
	else
		printf("\n\n'%s' foi encontrado pela busca em '%s'!\n VALOR: \"%s\"\n", termo1, secao1, resultado_busca);

	if ((resultado_busca = ini.buscar(leitura, "", termo1)) == NULL)
		printf("\n'%s' nao foi encontrado pela busca geral!\n\n", termo1);
	else
		printf("\n'%s' foi encontrado pela busca geral em alguma secao!\n VALOR: \'%s\'\n\n", termo1, resultado_busca);

	if ((resultado_busca = ini.buscar(leitura, secao2, termo2)) == NULL)
		printf("'%s' nao foi encontrado pela busca em '%s'!\n", termo2, secao2);
	else
		printf("'%s' foi encontrado pela busca em '%s'!\n VALOR: \"%s\"\n", termo2, secao2, resultado_busca);

	if ((resultado_busca = ini.buscar(leitura, "", termo2)) == NULL)
		printf("\n'%s' nao foi encontrado pela busca geral!\n\n", termo2);
	else
		printf("\n'%s' foi encontrado pela busca geral em alguma secao!\n VALOR: \'%s\'\n", termo2, resultado_busca);

	printf("\n_____________________________________________\n");

	// Alterações

	printf("\n\n####### Realizando alterações:\n");

	printf("\n\nO valor de '%s' e: '%s'\n", termo2, ini.buscar(leitura, "Seção 1", termo2));
	printf("\nAlterando...\n");
	ini.alterar(leitura, "Seção 1", termo2, "OUTRO VALOR AÍ");
	printf("\nO novo valor de '%s' e: '%s'\n", termo2, ini.buscar(leitura, "Seção 1", termo2));
	printf("\nImprimindo a Seção 1:\n");

	ini.imprimir(leitura, "Seção 1");

	printf("\nAlterando todos os \"parametros1\" com o valor \"Eu mexi aqui!!!!!!\"\n");
	ini.alterar(leitura, "", "parametro1", "Eu mexi aqui!!!!!!");
	printf("\nSituacao geral:\n");
	ini.imprimir(leitura, "");
	printf("\n");

	printf("\n_____________________________________________\n");

	// Inserção

	printf("\n\n####### Realizando inserções:\n");

	printf("\n\nInserindo novo parametro na Secao1...\n");
	ini.inserir(leitura, "Seção 1", "eu_nao_existia", "tenho um valor novo tambem");
	printf("\nImprimindo a Seção 1:\n");

	ini.imprimir(leitura, "Seção 1");

	printf("\n\n");

	// Limpeza
	ini.limpar(leitura);

	return 1;
}
```

Se estiver no Linux, rode os seguintes comandos para compilar e testar:

```sh
cd caminho_da_pasta_src
gcc inizator.c teste.c -o teste
./teste
```
