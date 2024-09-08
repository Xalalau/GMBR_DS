# Biblioteca geral.*

Essa biblioteca é um aglomerado de funções para usos diversos.

Plataformas: Windows e Linux.

## Funções

void **erro** ()
```
	Imprime uma mensagem de erro genérica
	Nota: eu queria fazer um uso decente dessa função, mas ela está essa coisa genérica e inútil no momento
```

void **pausar** ()
```
	Uma pausa para ver as mensagens no terminal
```

int **pegarPastaCorrente** (char dir_base[])
```
	Retorna o diretório corrente

	ENTRADA:
	dir_base = Variável onde será salvo o endereço do diretório corrente

	RETORNO:
	1 = Pasta corrente obtida com sucesso,
	0 = Falha
```

int **verificarExistencia** (char arquivo[])
```
	Verifica a exitência de arquivos

	ENTRADA:
	arquivo = Caminho do arquivo

	RETORNO:
	1 = Pasta ou arquivo existe,
	0 = Não existe
```

int **criarPasta** (char pasta[])
```
	Cria pastas

	ENTRADA:
	pasta = Caminho de pasta

	RETORNO:
	1 = Criou a pasta,
	2 = Falhou,
	3 = Já existe
```

int **existeCharXNaStringY** (char ch, char array[])
```
	Verifica se existe o char X no array Y

	ENTRADA:
	ch = Caracter a ser procurado
	array[] = Texto onde procuraremos o caracter

	RETORNO:
	1 = Caracter existe,
	0 = Não existe
```

## Utilização

1) Clone o repositório:

```sh
$ git clone https://github.com/xalalau/Geral.git
```

2) Importe o arquivo **geral.h**;
3) Faça as operações que quiser com o tempo termo **geral.** e com os #define;

Assim, por exemplo:

```C
#include "geral.h"
#include <stdio.h>

int main() {
	printf("Estou no: %s\n", SISTEMA);
	
	char dir_base[MAX_CHAR_DIR];
	
	if( ! geral.pegarPastaCorrente(dir_base))
		return 0;

	printf("Essa é a pasta atual: %s\n", dir_base);

	PAUSA;

	return 1;
}
```
