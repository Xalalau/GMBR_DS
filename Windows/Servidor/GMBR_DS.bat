
:: SCRIPT PARA INSTALAÇÃO DE SERVIDORES DEDICADOS DE GARRY'S MOD
:: Escrito por Xalalau - GMBR, Garry's Mod Brasil
:: http://www.gmbrblog.blogspot.com.br/ 

setlocal enableextensions enabledelayedexpansion

@echo off
cls
echo  .d8888b.  888b     d888 888888b.   8888888b.       8888888b.   .d8888b.  
echo d88P  Y88b 8888b   d8888 888  "88b  888   Y88b      888  "Y88b d88P  Y88b 
echo 888    888 88888b.d88888 888  .88P  888    888      888    888 Y88b.      
echo 888        888Y88888P888 8888888K.  888   d88P      888    888  "Y888b.   
echo 888  88888 888 Y888P 888 888  "Y88b 8888888P"       888    888     "Y88b. 
echo 888    888 888  Y8P  888 888    888 888 T88b        888    888       "888 
echo Y88b  d88P 888   "   888 888   d88P 888  T88b       888  .d88P Y88b  d88P 
echo  "Y8888P88 888       888 8888888P"  888   T88b      8888888P"   "Y8888P" v1.5
echo.
echo O GMBR DS, Garry's Mod Brasil Dedicated Server, e um script capaz de 
echo instalar/atualizar automaticamente um servidor dedicado de Garry's Mod.
echo Ele tambem instala/atualiza e monta contents de TF2 e CSS.
echo.
echo Para atualizar seu servidor e/ou contents, apenas reabra este script.
echo. 
echo GMBR, Garry's Mod Brasil
echo http://www.gmbrblog.blogspot.com.br/
echo Script feito por Xalalau
echo.
echo.
echo.
pause
cls
goto principal


 :: ------------------------------------------
 :: Funções
 :: ------------------------------------------
 
 
 :decidir_carregar_opcoes
	if not exist "%CONFIG_CONTENTS%" (
		set RESP=s
		goto:eof
	)
	echo Deseja alterar as configuracoes salvas? [S/N]
	set RESP=nada
	:loop
		set /p RESP=Escolha:
		if %RESP%==n goto:eof
		if %RESP%==N (
			set RESP=n
			goto:eof
		)
		if %RESP%==s  goto:eof
		if %RESP%==S (
			set RESP=s
			goto:eof
		)
		goto loop

 :carregar_opcoes
	cls
	for /f "useback tokens=*" %%a in ("%CONFIG_CONTENTS%") do (
		set /a N+=1
		set v!N!=%%a
	)
	set DIR_STEAM=!v1:~0,-1!
	set ESCOLHA1=!v2:~0,-1!
	set ESCOLHA2=!v3:~0,-1!
	set DIR_STEAM_Acertar=!v1:~0,-2!
	if exist "%DIR_STEAM%\Steam.exe" ( set TEST1=1 ) else ( set TEST1=ruim )
	if exist "%DIR_STEAM_Acertar%Steam.exe" ( set TEST2=1 ) else ( set TEST2=ruim )
	if not %TEST1%==1 if not %TEST2%==1 goto reconfigurar
	if %TEST2%==1 set DIR_STEAM=!v3:~0,-2!
	if not %ESCOLHA1%==1 if not %ESCOLHA1%==2 if not %ESCOLHA1%==3 goto reconfigurar
	if not %ESCOLHA2%==1 if not %ESCOLHA2%==2 if not %ESCOLHA2%==3 goto reconfigurar
	echo GMBR DS: As informacoes carregadas sao validas.
	goto:eof
	:reconfigurar
	echo.
	echo GMBR DS: Erro nas configuracoes carregadas. Reconfiguracao necessaria.
	set RESP=s
	echo.
	pause
 goto:eof
 
 :configurar_contents
	set ESCOLHA1=nada
	set ESCOLHA2=nada
	:escolha
	cls
	echo.
	echo Escolha uma das opcoes para cada content:
	echo 1 - Baixar/atualizar [SteamCMD]
	echo 2 - Usar instalacao do Steam [Steamapps/common]
	echo 3 - Nao fazer nada
	echo.
	echo Counter-Strike: Source
	set /p ESCOLHA1=Escolha: 
	echo.
	echo Team-Fortress 2
	set /p ESCOLHA2=Escolha:
	set VAI1=false
	set VAI2=false
	set VAI3=false
	if %ESCOLHA1%==1 set VAI1=true
	if %ESCOLHA1%==2 set VAI1=true
	if %ESCOLHA1%==3 set VAI1=true
	if %ESCOLHA2%==1 set VAI2=true
	if %ESCOLHA2%==2 set VAI2=true
	if %ESCOLHA2%==3 set VAI2=true
	if %VAI1%==true if %VAI2%==true set VAI3=true
	if %VAI3%==false goto escolha
 goto:eof
 
 :encontrar_steam
	cls
	if exist "C:\Program Files (x86)\Steam" set "DIR_STEAM=C:\Program Files (x86)\Steam"
	if exist "C:\Program Files\Steam" set "DIR_STEAM=C:\Program Files\Steam"
	if exist "C:\Arquivos de Programas (x86)\Steam" set "DIR_STEAM=C:\Arquivos de Programas (x86)\Steam"
	if exist "C:\Arquivos de Programas\Steam" set "DIR_STEAM=C:\Arquivos de Programas\Steam"
	if exist "C:\Steam" set "DIR_STEAM=C:\Steam"
	if exist "D:\Program Files (x86)\Steam" set "DIR_STEAM=D:\Program Files (x86)\Steam"
	if exist "D:\Program Files\Steam" set "DIR_STEAM=D:\Program Files\Steam"
	if exist "D:\Arquivos de Programas (x86)\Steam" set "DIR_STEAM=D:\Arquivos de Programas (x86)\Steam"
	if exist "D:\Arquivos de Programas\Steam" set "DIR_STEAM=D:\Arquivos de Programas\Steam"
	if exist "D:\Steam" set "DIR_STEAM=D:\Steam"
	if exist "E:\Program Files (x86)\Steam" set "DIR_STEAM=E:\Program Files (x86)\Steam"
	if exist "E:\Program Files\Steam" set "DIR_STEAM=E:\Program Files\Steam"
	if exist "E:\Arquivos de Programas (x86)\Steam" set "DIR_STEAM=E:\Arquivos de Programas (x86)\Steam"
	if exist "E:\Arquivos de Programas\Steam" set "DIR_STEAM=E:\Arquivos de Programas\Steam"
	if exist "E:\Steam" set "DIR_STEAM=E:\Steam"
	if exist "G:\Program Files (x86)\Steam" set "DIR_STEAM=G:\Program Files (x86)\Steam"
	if exist "G:\Program Files\Steam" set "DIR_STEAM=G:\Program Files\Steam"
	if exist "G:\Arquivos de Programas (x86)\Steam" set "DIR_STEAM=G:\Arquivos de Programas (x86)\Steam"
	if exist "G:\Arquivos de Programas\Steam" set "DIR_STEAM=G:\Arquivos de Programas\Steam"
	if exist "G:\Steam" set "DIR_STEAM=G:\Steam"
	if not exist "%DIR_STEAM%\Steam.exe" goto inserir_diretorio
	goto steam_encontrado
	:inserir_diretorio
	echo GMBR DS: O Steam nao foi encontrado.
	set /p DIR_STEAM=GMBR DS: Escreva o diretorio do Steam: 
	if not exist "%DIR_STEAM%\Steam.exe" goto inserir_diretorio
	:steam_encontrado
	echo GMBR DS: O Steam foi detectado em "%DIR_STEAM%".
 goto:eof
 
 :salvar_opcoes
	if exist "%CONFIG_CONTENTS%" del "%CONFIG_CONTENTS%"
	echo %DIR_STEAM% >> "%CONFIG_CONTENTS%"
	echo %ESCOLHA1% >> "%CONFIG_CONTENTS%"
	echo %ESCOLHA2% >> "%CONFIG_CONTENTS%"
	echo GMBR DS: Configuracoes salvas.
 goto:eof
 
 :decidir_atualizar_contents
	echo.
	echo Deseja instalar/atualizar os contents nessa execucao do script? [S/N]
	:loop2
		set /p ATUALIZAR_CONTENTS=Escolha:
		if %ATUALIZAR_CONTENTS%==n (
			echo.
			goto:eof
		)
		if %ATUALIZAR_CONTENTS%==N (
			echo.
			goto:eof
		)
		if %ATUALIZAR_CONTENTS%==s (
			echo.
			set ATUALIZAR_CONTENTS=s
			goto:eof
		)
		if %ATUALIZAR_CONTENTS%==S (
			echo.
			set ATUALIZAR_CONTENTS=s
			goto:eof
		)
		goto loop2
 goto:eof
 
:criar_pasta
	set PASTA=%~1
	set "DIR=%DIR_BASE%%PASTA%"
	if not exist "%DIR%" (
		mkdir "%DIR%"
		echo GMBR DS: Criando diretorio %DIR%
	)
goto:eof

:instalar_steamcmd
	if not exist "%CD%\steamcmd.exe" (
		echo GMBR DS: Baixando e instalando o SteamCMD em %CD%...
		%MYFILES%\wget.exe --output-document "%CD%\steamcmd.zip" http://media.steampowered.com/installer/steamcmd.zip
		%MYFILES%\unzip.exe "%CD%\steamcmd.zip"
	)
goto:eof

:instalar_servidor
	set JOGO=%~1
	set APP_ID=%~2
	set "DIR=%~3"
	echo GMBR DS: Atualizando %JOGO%...
	if %APP_ID%==4020 (
		echo GMBR DS: Instalando/atualizando dedicado de %JOGO%...
	) else (
	if not %APP_ID%==4020 echo GMBR DS: Instalando/atualizando content de %$JOGO%...
	)
	"%CD%\steamcmd.exe" +login anonymous +force_install_dir "%DIR%" +app_update %APP_ID% validate +quit
goto:eof

:montar_contents
	if not exist "%DIR_BASE%gmodds\garrysmod\cfg" (
		echo GMBR DS: O diretorio ...\garrysmod\cfg nao foi encontrado.
		pause
		exit
	)
	set "ARQ=%DIR_BASE%gmodds\garrysmod\cfg\mount.cfg"
	del "%ARQ%"
	echo "mountcfg" >> "%ARQ%"
	echo { >> "%ARQ%"
	if %ESCOLHA1%==1 echo 	"cstrike"	"%DIR_BASE%contents\css\cstrike" >> "%ARQ%"
	if %ESCOLHA1%==2 echo 	"cstrike"	"%DIR_STEAM%\steamapps\common\counter-strike source\cstrike" >> "%ARQ%"
	if %ESCOLHA2%==1 echo 	"tf"		"%DIR_BASE%contents\tf2\tf" >> "%ARQ%"
	if %ESCOLHA2%==2 echo 	"tf"		"%DIR_STEAM%\steamapps\common\team fortress 2\tf" >> "%ARQ%"
	echo } >> "%ARQ%"
	echo GMBR DS: O arquivo mount.cfg foi configurado.
 goto:eof
 
 
 :: ------------------------------------------
 :: Principal
 :: ------------------------------------------
 
 
 :principal
 set "DIR_BASE=%~dp0"
 set "CONFIG_CONTENTS=%DIR_BASE%opt.txt"
 set MONTAR_CONTENTS=n
 set ATUALIZAR_CONTENTS=n
 
 call:decidir_carregar_opcoes
 
 if %RESP%==n call:carregar_opcoes
 :: if abaixo é necessário!
 if %RESP%==s (
	call:configurar_contents
	call:encontrar_steam
	call:salvar_opcoes
 
	set MONTAR_CONTENTS=s
 )

 set LIB1=false
 if %ESCOLHA1%==1 set LIB1=true
 if %ESCOLHA2%==1 set LIB1=true
 if %LIB1%==true call:decidir_atualizar_contents
 call:criar_pasta "steamcmd"
 call:criar_pasta "gmodds"

 if %ATUALIZAR_CONTENTS%==s (
	call:criar_pasta "contents"
	if %ESCOLHA1%==1 call:criar_pasta "contents\css"
	if %ESCOLHA2%==1 call:criar_pasta "contents\tf2"
 )
 
 cd "%DIR_BASE%steamcmd"
 call:instalar_steamcmd
 
 call:instalar_servidor "Garry's Mod", 4020, "../gmodds"
 if %ATUALIZAR_CONTENTS%==s (
	if %ESCOLHA1%==1 call:instalar_servidor "Counter-Strike: Source", 232330, "../contents/css"
	if %ESCOLHA2%==1 call:instalar_servidor "Team Fortress 2", 232250, "../contents/tf2"
 )
 
if %MONTAR_CONTENTS%==s call:montar_contents

cd "%DIR_BASE%"

echo.
echo 888888 888888 888888 888888 888888 888888 888888 888888
echo GMBR DS: Fim do script.
echo.
pause
exit
