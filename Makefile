# Variáveis
VERSAO_DO_PROGRAMA = v2.8.5

# Nosso conversor
WRAPPER = comp-wrapper

# Pastas
DIR_BUILD = build
DIR_RELEASE = release
DIR_BIBLIOTECAS = lib
DIR_RECURSOS = res
DIR_SOURCE = src
DIR_SERVIDOR = Servidor
DIR_CONFIGURACOES = ${DIR_RECURSOS}/cfg
DIR_ICONE = ${DIR_RECURSOS}/icon
DIR_EXECUTAVEL = ${DIR_RECURSOS}/windows_exes
DIR_BINARIOS = ${DIR_RELEASE}/bin
DIR_COMPRESSAO = ${DIR_RELEASE}/zip
DIR_BIB_GERAL = ${DIR_BIBLIOTECAS}/Geral
DIR_BIB_INIZATOR = ${DIR_BIBLIOTECAS}/Inizator
DIR_BIB_CWRAPPER = ${DIR_BIBLIOTECAS}/Comp-Wrapper

# Arquivos source
SRC_GMBRDS = ${DIR_SOURCE}/GMBR_DS.c
SRC_BIB_GERAL = ${DIR_BIB_GERAL}/src/geral.c
SRC_BIB_INIZATOR = ${DIR_BIB_INIZATOR}/src/inizator.c

# Quantidade de arquivos dentro de DIR_BIB_CWRAPPER (Se for maior que 0, não há porque rodar a MODULOS)
BIB_ATUALIZAR_AUX := $(wildcard ${DIR_BIB_CWRAPPER}/*.md)
BIB_ATUALIZAR := $(words $(BIB_ATUALIZAR_AUX))

# Arquivos objeto
O_GERAL = geral.o
O_INIZATOR = inizator.o
O_GMBRDS = GMBR_DS.o

# Definindo SYSTEM se ele estiver vazio
ifeq ($(SYSTEM),)
	# Windows
	ifeq ($(OS),Windows_NT)
		SYSTEM = Windows
	else
		# Linux
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Linux)
			SYSTEM = Linux
		endif
	endif
	# Outros sistemas = sem suporte
	ifeq ($(SYSTEM),)
$(error "Seu sistema nao e suportado pelo GMBR DS")
	endif
endif

# Lidando com variáveis de acordo com os sistemas
ifeq ($(SYSTEM),LW)
	# Compilação de Linux e Windows
	# Para sumir warnings alguns warnings:
	DIR_OBJETOS64 = a
	DIR_OBJETOS32 = b
else
	# Linux
	ifeq ($(SYSTEM),Linux)
		# Compiladores
		CC64=gcc
		CC32=gcc
		FLAG32=-m32
		# Pastas de objetos
		DIR_OBJETOS64 = ${DIR_BUILD}/obj64Lin
		DIR_OBJETOS32 = ${DIR_BUILD}/obj32Lin
	else
		# Windows
		ifeq ($(SYSTEM),Windows)
			# Compiladores
			CC64=x86_64-w64-mingw32-gcc
			CC32=i686-w64-mingw32-gcc
			CCRES64=x86_64-w64-mingw32-windres
			CCRES32=i686-w64-mingw32-windres
			FLAG32=
			# Pastas de objetos
			DIR_OBJETOS64 = ${DIR_BUILD}/obj64Win
			DIR_OBJETOS32 = ${DIR_BUILD}/obj32Win
			# Binários extras requisitados
			WGET=${DIR_EXECUTAVEL}/wget.exe
			UZIP=${DIR_EXECUTAVEL}/unzip.exe
			# Extensão
			EXT=.exe
			# Recursos de arquivo
			RECURSOS = ${DIR_ICONE}/gmbrds-res.rc
			O_RECURSOS = gmbrds-res.o
			O_RECURSOS_64 = ${DIR_OBJETOS64}/${O_RECURSOS}
			O_RECURSOS_32 = ${DIR_OBJETOS32}/${O_RECURSOS}
		else
			# SYSTEM não suportado
			ifneq ($(SYSTEM),LW)
$(error "Entrada SYSTEM invalida. Escolha Linux, Windows ou LW para os dois")
			endif
		endif
	endif
endif

# Checagens de instalação
WRAPPER_CHECK := $(shell command -v comp-wrapper 2> /dev/null)
APT_CHECK := $(shell command -v apt 2> /dev/null)
GIT_CHECK := $(shell command -v git 2> /dev/null)
MINGW_CHECK := $(shell command -v ${CC32} 2> /dev/null)

# Pastas de objetos
O_GERAL_64 = ${DIR_OBJETOS64}/${O_GERAL}
O_INIZATOR_64 = ${DIR_OBJETOS64}/${O_INIZATOR}
O_GMBRDS_64 = ${DIR_OBJETOS64}/${O_GMBRDS}
O_GERAL_32 = ${DIR_OBJETOS32}/${O_GERAL}
O_INIZATOR_32 = ${DIR_OBJETOS32}/${O_INIZATOR}
O_GMBRDS_32 = ${DIR_OBJETOS32}/${O_GMBRDS}

# Nome dos binários + extensão
DS64 = ${DIR_BINARIOS}/GMBR_DS_64bits${EXT}
DS32 = ${DIR_BINARIOS}/GMBR_DS_32bits${EXT}


# -------------------
# Chamadas principais
# -------------------


# Compilar tudo e fazer o deploy
default: MINGW MODULOS COMPWRAPPER 64bits 32bits
ifeq ($(SYSTEM),LW)
	@echo "\n[!] Despachando comando..."
	$(MAKE) deploy SYSTEM=Linux
	@echo "\n[!] Despachando comando..."
	$(MAKE) deploy SYSTEM=Windows
else
	$(MAKE) deploy SYSTEM=$(SYSTEM)
endif

# Criar executável de 64 bits
64bits: MINGW MODULOS COMPWRAPPER MENSAGEM_64 ${O_GERAL_64} ${O_INIZATOR_64} ${O_GMBRDS_64} ${O_RECURSOS_64}
ifeq ($(SYSTEM),LW)
	@echo "\n[!] Despachando comando..."
	$(MAKE) 64bits SYSTEM=Windows
	@echo "\n[!] Despachando comando..."
	$(MAKE) 64bits SYSTEM=Linux
else
	${WRAPPER} ${CC64} ${O_GERAL_64} ${O_INIZATOR_64} ${O_GMBRDS_64} ${O_RECURSOS_64} -o ${DS64}
endif

# Criar executável de 32 bits
32bits: MINGW MODULOS COMPWRAPPER MENSAGEM_32 ${O_GERAL_32} ${O_INIZATOR_32} ${O_GMBRDS_32} ${O_RECURSOS_32}
ifeq ($(SYSTEM),LW)
	@echo "\n[!] Despachando comando..."
	$(MAKE) 32bits SYSTEM=Windows
	@echo "\n[!] Despachando comando..."
	$(MAKE) 32bits SYSTEM=Linux
else
	${WRAPPER} ${CC32} ${O_GERAL_32} ${O_INIZATOR_32} ${O_GMBRDS_32} ${O_RECURSOS_32} -o ${DS32} ${FLAG32}
endif

# Limpar todos os arquivos produzidos na compilação
clean:
ifeq "$(wildcard $(DIR_BIB_GERAL)/src )" "$(DIR_BIB_GERAL)/src"
	rm -r ${DIR_BIB_GERAL}/*
	rm -r ${DIR_BIB_GERAL}/.git
endif
ifeq "$(wildcard $(DIR_BIB_INIZATOR)/src )" "$(DIR_BIB_INIZATOR)/src"
	rm -r ${DIR_BIB_INIZATOR}/*
	rm -r ${DIR_BIB_INIZATOR}/.git
endif
ifeq "$(wildcard $(DIR_BIB_CWRAPPER)/src )" "$(DIR_BIB_CWRAPPER)/src"
	rm -r ${DIR_BIB_CWRAPPER}/*
	rm -r ${DIR_BIB_CWRAPPER}/.git
endif
ifeq "$(wildcard $(DIR_BUILD) )" "$(DIR_BUILD)"
	rm -r ${DIR_BUILD}
endif
ifeq "$(wildcard $(DIR_RELEASE) )" "$(DIR_RELEASE)"
	rm -r ${DIR_RELEASE}
endif


# --------------------
# Chamadas secundárias
# --------------------
# Ignore toda essa seção


# Fechar o pacote do GMBR DS
deploy: 
	@echo "\n[!] Deploy na pasta \"${DIR_SERVIDOR}\" + arquivo \"${SYSTEM}_GMBR_DS_${VERSAO_DO_PROGRAMA}.zip\"..."
	mkdir -p ${DIR_SERVIDOR}/cfg
ifeq ($(SYSTEM),Windows)
	mkdir ${DIR_SERVIDOR}/bin
	cp ${WGET} ${UZIP} ${DIR_SERVIDOR}/bin;
	cp ${DS64} ${DIR_SERVIDOR}/GMBR_DS_64.exe
	cp ${DS32} ${DIR_SERVIDOR}/GMBR_DS_32.exe
else
ifeq ($(SYSTEM),Linux)
	cp ${DS64} ${DIR_SERVIDOR}
	cp ${DS32} ${DIR_SERVIDOR}
endif
endif
	cp ${DIR_CONFIGURACOES}/cfg${SYSTEM}.ini ${DIR_SERVIDOR}/cfg/cfg.ini
	cp ${DIR_CONFIGURACOES}/contents${SYSTEM}.ini ${DIR_SERVIDOR}/cfg/contents.ini
	cp ${DIR_RECURSOS}/LEIAME.txt ${DIR_SERVIDOR}
	if [ ! -d ${DIR_COMPRESSAO} ]; then mkdir ${DIR_COMPRESSAO}; fi
	zip -r ${SYSTEM}_GMBR_DS_${VERSAO_DO_PROGRAMA}.zip ${DIR_SERVIDOR}
	rm -r ${DIR_SERVIDOR}
	mv ${SYSTEM}_GMBR_DS_${VERSAO_DO_PROGRAMA}.zip ${DIR_COMPRESSAO}

# Lidar com 64 bits
MENSAGEM_64:
ifneq ($(SYSTEM),LW)
	@echo "\n[!] Compilando GMBR DS 64 bits para ${SYSTEM}..."
endif

${O_RECURSOS_64}: ${RECURSOS}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CCRES64} ${RECURSOS} -O coff -o ${O_RECURSOS_64}
endif

${O_GERAL_64}: ${SRC_BIB_GERAL}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CC64} ${DIR_OBJETOS64} -c ${SRC_BIB_GERAL}
endif

${O_INIZATOR_64}: ${SRC_BIB_INIZATOR}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CC64} ${DIR_OBJETOS64} -c ${SRC_BIB_INIZATOR}
endif

${O_GMBRDS_64}: ${SRC_GMBRDS}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CC64} ${DIR_OBJETOS64} -c ${SRC_GMBRDS}
endif

# Lidar com 32 bits
MENSAGEM_32:
ifneq ($(SYSTEM),LW)
	@echo "\n[!] Compilando GMBR DS 32 bits para ${SYSTEM}..."
endif

${O_GERAL_32}: ${SRC_BIB_GERAL}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CC32} ${DIR_OBJETOS32} -c ${SRC_BIB_GERAL} ${FLAG32}
endif

${O_INIZATOR_32}: ${SRC_BIB_INIZATOR}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CC32} ${DIR_OBJETOS32} -c ${SRC_BIB_INIZATOR} ${FLAG32}
endif

${O_GMBRDS_32}: ${SRC_GMBRDS}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CC32} ${DIR_OBJETOS32} -c ${SRC_GMBRDS} ${FLAG32}
endif

${O_RECURSOS_32}: ${RECURSOS}
ifneq ($(SYSTEM),LW)
	${WRAPPER} ${CCRES32} ${RECURSOS} -O coff -o ${O_RECURSOS_32}
endif

# Crosscompiler
MINGW:
ifeq ($(SYSTEM),Windows)
ifndef MINGW_CHECK
ifndef APT_CHECK
	$(error "\nInstale o \"mingw-w64\" para poder usar esse makefile!\n")
endif
	@echo "\n[!] É necessário instalar o \"mingw-w64\" para compilar binários de Windows..."
	sudo apt install mingw-w64 -y
endif
endif

# Módulos do GMBR DS
MODULOS:
ifeq ($(BIB_ATUALIZAR), 0)
ifndef GIT_CHECK
ifndef APT_CHECK
	$(error "\nInstale o Git para poder usar esse makefile!\n")
endif
	@echo "\n[!] Necessário instalar \"Git\"..."
	sudo apt install git -y
endif
	@echo "\n[!] Baixando módulos..."
	git submodule update --init
endif

# Instalar o comp-wrapper
COMPWRAPPER:
ifndef WRAPPER_CHECK
	@echo "\n[!] Instalando script \"comp-wrapper\"..."
	sudo cp '${DIR_BIB_CWRAPPER}/src/comp-wrapper' /usr/bin
	sudo chmod 755 /usr/bin/comp-wrapper
endif
