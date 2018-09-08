# GARRY'S MOD BRASIL DEDICATED SERVER

Programa para instalação de servidores dedicados de Garry's Mod ([Página oficial](http://gmbrblog.blogspot.com.br/2012/07/garrys-mod-brasil-dedicated-server-gmbr.html)).

# COMPILAÇÃO


## LINUX

*Testado na base do Ubuntu 18.04*

Para compilar no seu sistema, clone o repositório, entre na pasta do projeto e dê build pelo make:

```sh
$ git clone https://github.com/xalalau/GMBR_DS.git
$ cd GMBR_DS
$ make
````

Mas nós temos uma série de opções para escolher. Veja as combinações de comando para o makefile:

```sh
$ make REGRA SYSTEM=VALOR
````

REGRA:
- "64bits"  = Gera o executável de 64 bits;
- "32bits"  = Gera o executável de 32 bits;
- "deploy"  = Gera a pasta "Servidor" completa com os executáveis 64 e 32 bits e seu arquivo zip;
- "clean"   = Remove as pastas "build", "release" e "lib".

VALOR de SYSTEM:
- "Windows" = Gera o executável para Windows;
- "Linux"   = Gera o executável para Linux;
- "LW"   = Gera o executável para Linux e Windows.

Notas:
1) A omissão da REGRA fará o make escolher "deploy";
2) A omissão do SYSTEM fará o make criar executáveis para o seu sistema atual.

Exemplo:

```sh
$ make 64bits SYSTEM=Windows
````
Sairá o executável de 64 bits para Windows na pasta "release/bin".


## WINDOWS

*Infelizmente eu fiz o GMBR DS para ser totalmente compilado no Linux, então o método aqui acaba sendo altamente manual.*

No Windows eu uso o Git CMD para baixar os repositórios, o Code::Blocks com mingw para compilar e depois pego o restante dos arquivos os copiando mesmo. Fica assim:

- Instale o [Git](https://git-scm.com/download/win);
- Abra o Git CMD;
- Coloque:

```sh
cd C:\
git clone https://github.com/xalalau/GMBR_DS.git
cd GMBR_DS
git submodule update --init
```

- Baixe e instale o [Code::Blocks](http://www.codeblocks.org/downloads/26) versão **mingw**;
- No Code::Blocks crie um novo projeto **Console application** em C (dados de nome e local genéricos) (modo Degub e Release ativados);
- Passe o "Build target" para **Release**;
- Na árvore de arquivos, delete o "main.c" e mande **incluir recursivamente** os arquivos de "C:\GMBR_DS" (Aceitar os **.c**, **.h** e **.rc**);
- Na árvore de arquivos, abra o GMBR_DS.c;
- Mande compilar em "Build -> Build";
- O executável vai aparecer na pasta **release** do seu projeto. Renomeie-o corretamente;
- Agora, para compilar em 64 bits, faça esses [passos](https://medium.com/@yzhong.cs/code-blocks-compile-64-bit-under-windows-with-mingw-w64-79101f5bbc02) e repita o Build;
- Coloque os executáveis 32 e 64 bits dentro da pasta do GMBR DS;
- Pronto! Uma bela volta.
