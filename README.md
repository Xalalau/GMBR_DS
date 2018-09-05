# GARRY'S MOD BRASIL DEDICATED SERVER

Programa para instalação de servidores dedicados de Garry's Mod ([Página oficial](http://gmbrblog.blogspot.com.br/2012/07/garrys-mod-brasil-dedicated-server-gmbr.html)).

### COMPILAÇÃO


**LINUX** (Testado na base do Ubuntu 18.04)

Para compilar no seu sistema, clone o repositório, entre na pasta do projeto e dê build pelo make:

```sh
$ git clone https://github.com/xalalau/GMod.git
$ cd ~/GMod/GMBR_DS/
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

Dois exemplos:

```sh
$ make 64bits SYSTEM=Windows
````
Sairá o executável de 64 bits para Windows na pasta "release/bin".


**WINDOWS**

Aqui eu uso o CodeBlocks para compilar e nesse ponto faço o posicionamento dos arquivos manualmente mesmo, fica assim:

- [Baixe o código do GMBR DS](https://github.com/xalalau/GMod/archive/master.zip) e extraia ele em um local qualquer (Vamos usar **C:\GMBR-DS**);
- Crie a pasta lib na pasta do GMBR DS (**C:\GMBR-DS\lib**);
- Baixe a [Xalateca](https://github.com/xalalau/Xalateca) e a extraia na pasta "lib" do GMBR DS (**C:\GMBR-DS\lib\Xalateca**);
- Baixe e instale o [CodeBlocks](http://www.codeblocks.org/downloads/26) versão **mingw**;
- Abra o CodeBlocks e crie um novo **Pojeto de Console**;
- Passe o "Build target" do projeto para **Release** e cheque nas configurações se o GCC está configurado para **32 bits**;
- Na árvore de arquivos, delete o "main.c" e mande inlcuir recursivamente os arquivos do GMBR DS a partir de **C:\GMBR-DS** marcando para inclusão apenas os arquivos importantes, os **.c** e o **.rc**;
- Na árvore de arquivos, abra o GMBR_DS.c;
- Mande compilar;
- Pronto! O executável vai aparecer na pasta **release** do seu projeto.

Me desculpem por isso ainda não ser automatizado. Eu programo o GMBR DS no Linux.
