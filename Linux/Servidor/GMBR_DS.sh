 #!/bin/bash

 # SCRIPT PARA INSTALAÇÃO DE SERVIDORES DEDICADOS DE GARRY'S MOD
 # Escrito por Xalalau - GMBR, Garry's Mod Brasil
 # http://www.gmbrblog.blogspot.com.br/
 
 clear
 echo __.d8888b.__888b_____d888_888888b.___8888888b._______8888888b.___.d8888b.  
 echo _d88P__Y88b_8888b___d8888_888__º88b__888___Y88b______888__ºY88b_d88P__Y88b 
 echo _888____888_88888b.d88888_888__.88P__888____888______888____888_Y88b.      
 echo _888________888Y88888P888_8888888K.__888___d88P______888____888__ºY888b.   
 echo _888__88888_888_Y888P_888_888__ºY88b_8888888Pº_______888____888______ºY88b. 
 echo _888____888_888__Y8P__888_888____888_888_T88b________888____888+______º888
 echo _Y88b__d88P_888___º___888_888___d88P_888__T88b_______888__.d88P_Y88b__d88P
 echo _ºY8888P88__888_______888_8888888Pº__888___T88b______8888888Pº___ºY8888Pº_v1.5
 echo
 echo O GMBR DS, Garry s Mod Brasil Dedicated Server, é um script capaz de 
 echo instalar/atualizar automaticamente um servidor dedicado de Garry s Mod.
 echo Ele também instala/atualiza e monta contents de TF2 e CSS.
 echo
 echo Para atualizar seu servidor e/ou contents, apenas reabra este script.
 echo 
 echo GMBR, Garry s Mod Brasil
 echo http://www.gmbrblog.blogspot.com.br/
 echo Script feito por Xalalau
 echo
 read -n1 -r -p "Aperte qualquer tecla para continuar..."
 clear


 # ------------------------------------------
 # Funções
 # ------------------------------------------


 decidir_carregar_opcoes(){
	if [ -f "$CONFIG_CONTENTS" ]; then
		echo Deseja alterar as configuracoes salvas? [S/N]
		while true; do
			read -n1 -r RESP
			if [ "$RESP" == "s" ] || [ "$RESP" == "n" ] || [ "$RESP" == "S" ] || [ "$RESP" == "N" ]; then
				break
			fi
		done
		clear
	else
		RESP=s
	fi
 }
 
 carregar_opcoes(){
	count=0
	while read line; do
		count=$(($count+1));
		if [ "$count" == "1" ]; then DIR_STEAM="$line"; fi
		if [ "$count" == "2" ]; then OPCAO="$line"; fi
		if [ "$count" == "3" ]; then OPCAO2="$line"; fi
	done < "$CONFIG_CONTENTS"
	if [ -f "$DIR_STEAM/steam.pid" ]; then TEST1=1; else REMOVER=1; fi
	if [ "$OPCAO" != "1" ] && [ "$OPCAO" != "2" ] && [ "$OPCAO" != "3" ]; then REMOVER=1; fi
	if [ "$OPCAO2" != "1" ] && [ "$OPCAO2" != "2" ] && [ "$OPCAO2" != "3" ]; then REMOVER=1; fi
	if [ "$REMOVER" == "1" ]; then
		echo "GMBR DS: Erro nas configurações carregadas. Reconfiguração necessária."
		read -n1 -r -p "Aperte qualquer tecla para continuar..."
		echo
		RESP=s
	else
		echo "GMBR DS: As informações carregadas são válidas."
	fi
 }

 configurar_contents(){
	VAI=false
	VAI2=false
	while true;	do
		echo Escolha uma das opções para cada content:
		echo 1 - Baixar/atualizar [SteamCMD]
		echo 2 - Usar instalação do Steam [Steamapps/common]
		echo 3 - Não fazer nada
		echo
		echo Counter-Strike: Source
		read -n1 -r -p "Escolha: " OPCAO
		echo
		echo
		echo Team Fortress 2
		read -n1 -r -p "Escolha: " OPCAO2
		echo
		echo
		if [ "$OPCAO" == "1" ] || [ "$OPCAO" == "2" ] || [ "$OPCAO" == "3" ]; then VAI=true; fi
		if [ "$OPCAO2" == "1" ] || [ "$OPCAO2" == "2" ] || [ "$OPCAO2" == "3" ]; then VAI2=true; fi
		if [ "$VAI" == "true" ] && [ "$VAI2" == "true" ]; then break; fi
		clear
	done
 }

 encontrar_steam(){
	DIR_STEAM="$HOME/.steam"
	while true;	do
		if [ ! -e "$DIR_STEAM/steam.pid" ]; then
			echo "GMBR DS: O steam não foi encontrado."
			read -n200 -r -p "GMBR DS: Escreva o diretório do Steam: " DIR_STEAM
			echo
			echo
		else
			echo "GMBR DS: O Steam foi detectado em '$DIR_STEAM'."
			break
		fi
	done
 }

 salvar_opcoes(){
	if [ -e "$CONFIG_CONTENTS" ]; then rm "$CONFIG_CONTENTS"; fi
	echo -e "$DIR_STEAM" >> "$CONFIG_CONTENTS"
	echo -e $OPCAO >> "$CONFIG_CONTENTS"
	echo -e $OPCAO2 >> "$CONFIG_CONTENTS"
	echo "GMBR DS: Configuração dos contents salva."	 
 }
 
 decidir_atualizar_contents(){
	echo
	echo "Deseja instalar/atualizar os contents nessa execução do script? [S/N]"
	while true; do
		read -n1 -r ATUALIZAR_CONTENTS
		if [ "$ATUALIZAR_CONTENTS" == "S" ] ||  [ "$ATUALIZAR_CONTENTS" == "s" ]; then
			ATUALIZAR_CONTENTS=s
		fi
		if [ "$ATUALIZAR_CONTENTS" == "n" ] || [ "$ATUALIZAR_CONTENTS" == "N" ]; then
			break
		fi
	done
	clear
 }
 
 criar_pasta(){
 	PASTA=$1
	if ( [ ! -d "$DIR_BASE/$PASTA" ]; ) then
		DIR="$DIR_BASE/$PASTA"
		mkdir "$DIR"
		chmod 777 "$DIR"
		echo "GMBR DS: Criando o diretório $DIR..."
	fi
 }

 instalar_steamcmd(){
	if [ ! -e steamcmd.sh ]; then
		echo "GMBR DS: Baixando e instalando o SteamCMD em $DIR_BASE/steamcmd..."
		if [ -e steamcmd_linux.tar.gz ]; then rm steamcmd_linux.tar.gz; fi
		wget http://media.steampowered.com/client/steamcmd_linux.tar.gz
		(tar -xvzf steamcmd_linux.tar.gz)
	fi
 }

 atualizar_servidor(){
	JOGO=$1
 	APP_ID=$2
	DIR=$3
	if [ "$APP_ID" == "4020" ]; then
		echo "GMBR DS: Instalando/atualizando dedicado de $JOGO.."
	else
		echo "GMBR DS: Instalando/atualizando content de $JOGO.."
	fi
	./steamcmd.sh +login anonymous +force_install_dir "$DIR" +app_update $APP_ID validate +quit
 }

 montar_contents(){
	ARQ="$DIR_BASE/gmodds/garrysmod/cfg/mount.cfg"
	if [ ! -e "$ARQ" ]; then
		echo "GMBR DS: O diretório $ARQ não foi encontrado."
		break
	fi
	rm "$ARQ"
	echo -e "\n\"mountcfg\"" >> "$ARQ"
	echo -e "{" >> "$ARQ"
	if [ "$OPCAO" == "1" ]; then echo -e "\t\"cstrike\"\t\"$DIR_BASE/contents/css/cstrike\"" >> "$ARQ"; fi
	if [ "$OPCAO" == "2" ]; then echo -e "\t\"cstrike\"\t\"$DIR_STEAM/steam/SteamApps/common/Counter-Strike Source/cstrike\"" >> "$ARQ"; fi
	if [ "$OPCAO2" == "1" ]; then echo -e "\t\"tf\"\t\t\"$DIR_BASE/contents/tf2/tf\"" >> "$ARQ"; fi
	if [ "$OPCAO2" == "2" ]; then echo -e "\t\"tf\"\t\t\"$DIR_STEAM/steam/SteamApps/common/Team Fortress 2/tf\"" >> "$ARQ"; fi
	echo -e "}" >> "$ARQ"
	echo "GMBR DS: O arquivo mount.cfg foi configurado."
}


 # ------------------------------------------
 # Principal
 # ------------------------------------------


 DIR_BASE="$(cd "${0%/*}" && echo $PWD)" 
 CONFIG_CONTENTS="$DIR_BASE/opt.txt"
 MONTAR_CONTENTS=n
 ATUALIZAR_CONTENTS=n

 decidir_carregar_opcoes

 if [ "$RESP" == "n" ] || [ "$RESP" == "N" ]; then
	carregar_opcoes
 fi
 
 if [ "$RESP" == "s" ] || [ "$RESP" == "S" ]; then #if necessário!
	configurar_contents
	encontrar_steam
	salvar_opcoes
	
	MONTAR_CONTENTS=s
 fi

 if [ "$OPCAO" == "1" ] || [ "$OPCAO2" == "1" ]; then
	decidir_atualizar_contents
 fi

 criar_pasta steamcmd
 criar_pasta gmodds

 if [ "$ATUALIZAR_CONTENTS" == "s" ]; then
	criar_pasta contents
	if [ "$OPCAO" == "1" ]; then criar_pasta contents/css; fi
	if [ "$OPCAO2" == "1" ]; then criar_pasta contents/tf2; fi
 fi

 cd "$DIR_BASE/steamcmd" 
 instalar_steamcmd
 
 atualizar_servidor "Garry's Mod" 4020 "../gmodds" 
 if [ "$ATUALIZAR_CONTENTS" == "s" ]; then
	if [ "$OPCAO" == "1" ]; then atualizar_servidor "Counter-Strike: Source" 232330 "../contents/css"; fi
	if [ "$OPCAO2" == "1" ]; then atualizar_servidor "Team Fortress 2" 232250 "../contents/tf2"; fi
 fi
 
 if [ "$MONTAR_CONTENTS" == "s" ]; then
	montar_contents
 fi
 
 cd $DIR_BASE

 echo
 echo 888888 888888 888888 888888 888888 888888 888888 888888 888888
 echo "GMBR DS: Fim do script."
 echo
