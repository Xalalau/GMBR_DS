# GARRY'S MOD BRASIL DEDICATED SERVER

Porta em Python do Garry's Mod Brasil Dedicated Server ([pagina oficial](http://gmbrblog.blogspot.com.br/2012/07/garrys-mod-brasil-dedicated-server-gmbr.html)).

A versao original em C foi preservada na branch local `backup/c-original`.

## O Que Mudou

- Leitura de `.ini` com `configparser`, no lugar da biblioteca local `Inizator`.
- Caminhos e pastas com `pathlib` e `shutil`, no lugar da biblioteca local `Geral`.
- Download do SteamCMD com `urllib.request`, sem `wget.exe`.
- Extracao de `.zip` e `.tar.gz` com `zipfile` e `tarfile`, sem `unzip.exe`/`tar` externo.
- Execucao do SteamCMD e SRCDS com `subprocess`.
- Leitura de `libraryfolders.vdf` com a biblioteca `vdf` quando instalada, com fallback interno simples.

## Requisitos

- Python 3.10 ou superior.
- Opcional: instalar o parser VDF recomendado.

```sh
python -m pip install -r requirements.txt
```

Sem o pacote `vdf`, o programa ainda tenta ler `libraryfolders.vdf` com um parser simples para o campo `path`.

## Uso

Menu interativo:

```sh
python src/gmbr_ds.py
```

Executar uma acao diretamente:

```sh
python src/gmbr_ds.py --action show-config
python src/gmbr_ds.py --action install-server
python src/gmbr_ds.py --action install-contents
python src/gmbr_ds.py --action install-all
python src/gmbr_ds.py --action mount-contents
python src/gmbr_ds.py --action start-server
```

Durante o desenvolvimento, se `cfg/cfg.ini` e `cfg/contents.ini` nao existirem, o programa usa os modelos de `res/cfg` conforme a plataforma atual:

- `cfgWindows.ini` e `contentsWindows.ini`
- `cfgLinux.ini` e `contentsLinux.ini`

Para usar uma pasta de configuracao especifica:

```sh
python src/gmbr_ds.py --config-dir caminho/para/cfg
```

## Contents

O arquivo `contents.ini` usa estes campos:

- `mount`: chave escrita no `mount.cfg`, como `cstrike`, `tf` ou `dod`.
- `steam`: pasta base do jogo dentro da biblioteca Steam.
- `subpasta`: pasta dentro do jogo que contem `gameinfo.txt`; em branco usa a propria pasta base.
- `opcao`: `1` baixa/atualiza via SteamCMD, `2` monta uma instalacao local do Steam, `3` ignora.

Quando `opcao = 1`, o GMBR DS baixa o content para `pasta_contents/<nome>` e monta `subpasta` dentro desse download. Quando `opcao = 2`, ele procura `steam + subpasta` nas bibliotecas Steam encontradas.

## Makefile

O `Makefile` agora e apenas um atalho para comandos Python:

```sh
make run
make show-config
make install-server
make install-contents
make install-all
make mount-contents
make start-server
make test
```

## Deploy

Gerar pacote da plataforma atual:

```sh
python scripts/deploy.py
```

Gerar pacote de uma plataforma especifica:

```sh
python scripts/deploy.py --system Windows
python scripts/deploy.py --system Linux
python scripts/deploy.py --system all
```

Os zips saem em `release/zip`.

## Testes

```sh
python -m unittest discover -s tests
```
