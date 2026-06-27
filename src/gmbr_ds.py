#!/usr/bin/env python3
from __future__ import annotations

import argparse
import configparser
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import tarfile
import urllib.request
import zipfile
from dataclasses import dataclass, field
from pathlib import Path, PurePosixPath
from typing import Iterable
from urllib.parse import urlparse

try:
    import vdf as vdf_lib
except ImportError:  # The app still works with a small fallback parser.
    vdf_lib = None


VERSION = "v.2.8.6"
NOT_FOUND = "NAO ENCONTRADA!"
GMOD_NAME = "Garry's Mod"
GMOD_APP_ID = "4020"


class GmbrError(RuntimeError):
    pass


class IniFile:
    def __init__(self, path: Path, parser: configparser.RawConfigParser) -> None:
        self.path = path
        self.parser = parser

    @classmethod
    def load(cls, path: Path) -> "IniFile":
        parser = configparser.RawConfigParser(
            interpolation=None,
            strict=False,
            empty_lines_in_values=False,
        )

        try:
            text = path.read_text(encoding="utf-8-sig")
        except UnicodeDecodeError:
            text = path.read_text(encoding="latin-1")

        parser.read_string(text, source=str(path))
        return cls(path, parser)

    def sections(self) -> list[str]:
        return self.parser.sections()

    def has_any(self, option: str) -> bool:
        return any(self.parser.has_option(section, option) for section in self.sections())

    def get(self, section: str, option: str, default: str = "") -> str:
        if not section:
            return self.get_any(option, default)
        if not self.parser.has_section(section):
            return default
        return self.parser.get(section, option, fallback=default).strip()

    def get_any(self, option: str, default: str = "") -> str:
        for section in self.sections():
            if self.parser.has_option(section, option):
                return self.parser.get(section, option, fallback=default).strip()
        return default

    def set(self, section: str, option: str, value: str | Path) -> None:
        if not self.parser.has_section(section):
            self.parser.add_section(section)
        self.parser.set(section, option, str(value))

    def set_any(self, option: str, value: str | Path) -> bool:
        for section in self.sections():
            if self.parser.has_option(section, option):
                self.parser.set(section, option, str(value))
                return True
        return False


@dataclass
class SteamInfo:
    marker_names: list[str]
    steamcmd_name: str
    srcds_name: str
    libraryfolders_name: str = "libraryfolders.vdf"
    steamcmd_path: Path | None = None
    srcds_path: Path | None = None
    libraryfolders_path: Path | None = None
    libraries: list[Path] = field(default_factory=list)


@dataclass
class GmodInfo:
    name: str = GMOD_NAME
    app_id: str = GMOD_APP_ID
    mount_cfg: Path | None = None


class GmbrDS:
    def __init__(self, base_dir: Path, config_dir: Path | None = None) -> None:
        self.start_dir = base_dir.expanduser().resolve()
        self.base_dir = self.start_dir
        self.config_dir_override = config_dir.expanduser().resolve() if config_dir else None
        self.project_dir = Path(__file__).resolve().parents[1]
        self.system = self._detect_system()
        self.line_sep = "\r\n" if self.system == "Windows" else "\n"
        self.cfg_path: Path | None = None
        self.contents_path: Path | None = None
        self.params: IniFile | None = None
        self.contents: IniFile | None = None
        self.gmod = GmodInfo()
        self.steam = self._default_steam_info()

    def bootstrap(self) -> bool:
        print("[GMBR DS] Diretorio corrente obtido")
        self._adjust_basic_config()
        if not self._load_cfg_ini():
            return False
        if not self._load_contents_ini():
            return False
        if not self._find_steam():
            return False
        self._adjust_variables()
        if not self._load_steam_libraries():
            return False
        return self._validate_contents()

    def run_menu(self) -> None:
        while True:
            choice = self._show_main_menu()
            if choice == 0:
                return
            if choice == 1:
                self.show_config(pause=True)
            elif choice == 2:
                self._run_action("install-server", pause=True)
            elif choice == 3:
                self._run_action("install-contents", pause=True)
            elif choice == 4:
                self._run_action("install-all", pause=True)
            elif choice == 5:
                self.start_server()
                if self.system == "Windows":
                    return
            elif choice == 6:
                self._run_action("mount-contents", pause=True)

    def run_action(self, action: str) -> bool:
        return self._run_action(action, pause=False)

    def show_config(self, pause: bool = False) -> None:
        self._clear_screen()
        params = self._params()
        contents = self._contents()

        print("______________________________________________________________________________")
        print()
        print("CONFIGURACOES CARREGADAS:")
        print("______________________________________________________________________________")
        print()

        print(" ______")
        print("[PASTAS]\n")
        print("  Base:")
        print(f"    {self.base_dir}")
        print(f"  Servidor de GMod:\n    {params.get_any('pasta_servidor')}")
        print(f"  Contents:\n    {params.get_any('pasta_contents')}")
        print(f"  Steam:\n    {params.get_any('pasta_steam')}")
        print(f"  SteamCMD:\n    {params.get_any('pasta_steamcmd')}")

        if self.steam.libraries:
            print(" ____________________")
            print("[BIBLIOTECAS DO STEAM]\n")
            for index, library in enumerate(self.steam.libraries, start=1):
                print(f"  [{index}] {library}")

        print(" ________")
        print("[DOWNLOAD]\n")
        print(f"  SteamCMD:\n    {params.get_any('download_steamcmd')}")

        print(" ________")
        print("[ARQUIVOS]\n")
        print(f"  Configuracoes gerais do GMBR DS:\n    {self.cfg_path}")
        print(f"  Configuracao de contents do GMBR DS:\n    {self.contents_path}")
        print(f"  Montagem de contents do GMod:\n    {self.gmod.mount_cfg}")
        print(f"  Executavel do SteamCMD:\n    {self.steam.steamcmd_path}")
        print(f"  Executavel do SRCDS:\n    {self.steam.srcds_path}")

        print(" ________________________________________________")
        print("[INFORMACAO DO SERVIDOR E DOS CONTENTS CARREGADOS]\n")
        print("    ___________")
        print(f"  + {self.gmod.name}")
        print(f"    pasta = {params.get_any('pasta_servidor')}")
        print(f"    id = {self.gmod.app_id}\n")

        for section in contents.sections():
            option = contents.get(section, "opcao")
            mount_key = self._content_mount_key(section)
            if option == "1":
                print()
                print(f"  + {section}")
                print(f"    mount = {mount_key}")
                print(f"    pasta = {contents.get(section, 'pasta')}")
                if contents.get(section, "subpasta"):
                    print(f"    subpasta = {contents.get(section, 'subpasta')}")
                print(f"    login = {contents.get(section, 'login')}")
                print(f"    id    = {contents.get(section, 'id')}")
                print(f"    opcao = {option}")
            elif option == "2":
                print()
                print(f"  + {section}")
                print(f"    mount = {mount_key}")
                print(f"    steam = {contents.get(section, 'steam')}")
                if contents.get(section, "subpasta"):
                    print(f"    subpasta = {contents.get(section, 'subpasta')}")
                print(f"    opcao = {option}")

        print(" ___________________")
        print("[COMANDO DO SERVIDOR]\n")
        print(f"    '{self.steam.srcds_path}' {params.get_any('comando_sv')}")
        print()
        print("______________________________________________________________________________")
        print()

        if pause:
            self._pause()

    def create_base_dirs(self) -> bool:
        params = self._params()
        paths = [
            self.base_dir,
            Path(params.get_any("pasta_steamcmd")),
            Path(params.get_any("pasta_servidor")),
            Path(params.get_any("pasta_contents")),
        ]
        return self._create_dirs(paths)

    def create_content_dirs(self) -> bool:
        contents = self._contents()
        paths = [
            Path(contents.get(section, "pasta"))
            for section in contents.sections()
            if contents.get(section, "opcao") == "1"
        ]
        return self._create_dirs(paths)

    def install_steamcmd(self) -> bool:
        params = self._params()
        steamcmd_path = self._require_path(self.steam.steamcmd_path, "SteamCMD")
        steamcmd_dir = Path(params.get_any("pasta_steamcmd"))

        if steamcmd_path.exists():
            print("\n[GMBR DS] O SteamCMD esta instalado")
            return True

        url = params.get_any("download_steamcmd")
        archive_name = PurePosixPath(urlparse(url).path).name
        if not archive_name:
            self._error_block("ERRO AO BAIXAR O STEAMCMD", "URL de download invalida.")
            return False

        archive_path = steamcmd_dir / archive_name
        print(f"\n[GMBR DS] Baixando SteamCMD em '{steamcmd_dir}'...\n")

        try:
            steamcmd_dir.mkdir(parents=True, exist_ok=True)
            if archive_path.exists():
                archive_path.unlink()
            with urllib.request.urlopen(url) as response, archive_path.open("wb") as output:
                shutil.copyfileobj(response, output)
        except OSError as exc:
            self._error_block("ERRO AO BAIXAR O STEAMCMD", str(exc))
            return False

        try:
            self._extract_archive(archive_path, steamcmd_dir)
        except (OSError, tarfile.TarError, zipfile.BadZipFile, ValueError) as exc:
            self._error_block("ERRO AO EXTRAIR O STEAMCMD", str(exc))
            return False

        if self.system == "Linux" and steamcmd_path.exists():
            mode = steamcmd_path.stat().st_mode
            steamcmd_path.chmod(mode | 0o111)

        print("\n[GMBR DS] O SteamCMD esta instalado")
        return True

    def install_update_server(self) -> bool:
        print(f"[GMBR DS] Instalando/atualizando '{self.gmod.name}'...\n")
        return self._steamcmd_app_update(
            app_id=self.gmod.app_id,
            install_dir=Path(self._params().get_any("pasta_servidor")),
            login="anonymous",
        )

    def install_update_contents(self) -> bool:
        contents = self._contents()
        params = self._params()

        for section in contents.sections():
            if contents.get(section, "opcao") != "1":
                continue

            print(f"\n[GMBR DS] Instalando/atualizando '{section}'...\n")
            if contents.get(section, "login") == "Usuario":
                print(
                    '[GMBR DS] Para que esse download de content funcione, '
                    'a "login_steam" do cfg.ini deve estar com login automatico '
                    "no Steam ou ja logada no Steam\n"
                )
                login = params.get_any("login_steam")
            else:
                login = "anonymous"

            if not self._steamcmd_app_update(
                app_id=contents.get(section, "id"),
                install_dir=Path(contents.get(section, "pasta")),
                login=login,
            ):
                return False

        return True

    def mount_contents(self) -> bool:
        mount_cfg = self._require_path(self.gmod.mount_cfg, "mount.cfg")
        contents = self._contents()
        mountable = [
            section
            for section in contents.sections()
            if contents.get(section, "opcao") in {"1", "2"}
        ]

        try:
            mount_cfg.parent.mkdir(parents=True, exist_ok=True)
            lines = ['"mountcfg"', "{"]
            for section in mountable:
                key = self._content_mount_key(section)
                path = self._content_mount_path(section)
                lines.append(f'\t"{key}" "{path}"')
            lines.append("}")
            mount_cfg.write_text(self.line_sep.join(lines), encoding="utf-8")
        except OSError as exc:
            self._error_block(
                "NAO FOI POSSIVEL MONTAR OS CONTENTS! ERRO AO ABRIR O ARQUIVO:",
                f"{mount_cfg}\n{exc}",
            )
            return False

        if mountable:
            print("\n[GMBR DS] Contents montados")
        else:
            print("\n[GMBR DS] Nao ha contents para montar")
        return True

    def start_server(self) -> bool:
        params = self._params()
        srcds_path = self._require_path(self.steam.srcds_path, "SRCDS")
        command_args = self._split_args(params.get_any("comando_sv"))
        args = [str(srcds_path), *command_args]

        if not srcds_path.exists():
            self._error_block("NAO FOI POSSIVEL INICIAR O SERVIDOR:", str(srcds_path))
            return False

        self._clear_screen()
        print("[GMBR DS] Ligando o servidor...")
        print(f"[GMBR DS] {self._format_command(args)}\n")

        try:
            if self.system == "Windows":
                subprocess.Popen(args, cwd=srcds_path.parent)
            else:
                subprocess.run(args, cwd=srcds_path.parent, check=False)
        except OSError as exc:
            self._error_block("ERRO AO INICIAR O SERVIDOR", str(exc))
            return False

        return True

    def _run_action(self, action: str, pause: bool) -> bool:
        if action == "show-config":
            self.show_config(pause=pause)
            return True
        if action == "install-server":
            ok = (
                self.create_base_dirs()
                and self.install_steamcmd()
                and self.install_update_server()
                and self.mount_contents()
            )
        elif action == "install-contents":
            ok = (
                self.create_base_dirs()
                and self.create_content_dirs()
                and self.install_steamcmd()
                and self.install_update_contents()
                and self.mount_contents()
            )
        elif action == "install-all":
            ok = (
                self.create_base_dirs()
                and self.create_content_dirs()
                and self.install_steamcmd()
                and self.install_update_server()
                and self.install_update_contents()
                and self.mount_contents()
            )
        elif action == "mount-contents":
            ok = self.mount_contents()
        elif action == "start-server":
            ok = self.start_server()
        else:
            raise GmbrError(f"Acao invalida: {action}")

        if pause:
            self._finish_option()
        return ok

    def _adjust_basic_config(self) -> None:
        self.cfg_path, self.contents_path = self._resolve_config_paths()

    def _load_cfg_ini(self) -> bool:
        path = self._require_path(self.cfg_path, "cfg.ini")
        if not path.exists():
            self._error_block(
                "NAO FOI POSSIVEL CARREGAR O ARQUIVO",
                str(path),
                "1) VERIFIQUE SE ELE ESTA PRESENTE NA PASTA 'CFG'",
                "2) VERIFIQUE SE TODOS OS PARAMETROS ESTAO PRESENTES E BEM FORMATADOS",
            )
            return False

        try:
            self.params = IniFile.load(path)
            self._require_cfg_options()
        except (OSError, configparser.Error, GmbrError) as exc:
            self._error_block(
                "ERRO AO CARREGAR AS OPCOES DO ARQUIVO",
                str(path),
                "VERIFIQUE SE TODOS OS PARAMETROS ESTAO PRESENTES E BEM FORMATADOS",
                str(exc),
            )
            return False

        print("[GMBR DS] Arquivo cfg.ini carregado")
        return True

    def _load_contents_ini(self) -> bool:
        path = self._require_path(self.contents_path, "contents.ini")
        if not path.exists():
            self._error_block("NAO FOI POSSIVEL CARREGAR O ARQUIVO", str(path))
            return False

        try:
            self.contents = IniFile.load(path)
        except (OSError, configparser.Error) as exc:
            self._error_block("ERRO AO CARREGAR O ARQUIVO", str(path), str(exc))
            return False

        print("[GMBR DS] Arquivo contents.ini carregado")
        return True

    def _find_steam(self) -> bool:
        params = self._params()
        configured = params.get_any("pasta_steam")

        if not configured:
            found = next((path for path in self._steam_candidates() if self._is_steam_dir(path)), None)
            params.set_any("pasta_steam", found if found else NOT_FOUND)
            return True

        steam_dir = Path(configured).expanduser()
        if self._is_steam_dir(steam_dir):
            params.set_any("pasta_steam", steam_dir)
            print("[GMBR DS] A pasta do Steam foi localizada")
            return True

        self._error_block("A PASTA DO STEAM E INVALIDA:", configured)
        return False

    def _adjust_variables(self) -> None:
        params = self._params()
        contents = self._contents()

        configured_base = params.get_any("pasta_base")
        if configured_base:
            self.base_dir = self._resolve_path(configured_base, self.start_dir)

        for option in ("pasta_contents", "pasta_steamcmd", "pasta_servidor"):
            params.set_any(option, self._resolve_path(params.get_any(option), self.base_dir))

        contents_root = Path(params.get_any("pasta_contents"))
        for section in contents.sections():
            if contents.get(section, "opcao") == "1":
                contents.set(section, "pasta", contents_root / section)

        self.steam.steamcmd_path = Path(params.get_any("pasta_steamcmd")) / self.steam.steamcmd_name
        self.steam.srcds_path = Path(params.get_any("pasta_servidor")) / self.steam.srcds_name

        steam_root = params.get_any("pasta_steam")
        if steam_root != NOT_FOUND:
            self.steam.libraryfolders_path = self._libraryfolders_path(Path(steam_root))

        self.gmod.mount_cfg = (
            Path(params.get_any("pasta_servidor")) / "garrysmod" / "cfg" / "mount.cfg"
        )

        print("[GMBR DS] Variaveis reajustadas")

    def _load_steam_libraries(self) -> bool:
        params = self._params()
        if params.get_any("pasta_steam") == NOT_FOUND:
            print("[GMBR DS] Bibliotecas de jogos do Steam carregadas")
            return True

        libraryfolders = self._require_path(self.steam.libraryfolders_path, "libraryfolders.vdf")
        if not libraryfolders.exists():
            self._error_block(
                "ERRO AO ABRIR O ARQUIVO DE BIBLIOTECAS DO STEAM!",
                str(libraryfolders),
            )
            return False

        try:
            self.steam.libraries = [Path(path) for path in self._read_steam_libraries(libraryfolders)]
        except (OSError, ValueError) as exc:
            self._error_block(
                "ERRO AO LER O ARQUIVO DE BIBLIOTECAS DO STEAM!",
                str(libraryfolders),
                str(exc),
            )
            return False

        print("[GMBR DS] Bibliotecas de jogos do Steam carregadas")
        return True

    def _validate_contents(self) -> bool:
        contents = self._contents()
        params = self._params()
        configured_count = 0

        for section in contents.sections():
            option = contents.get(section, "opcao")
            if option == "1":
                configured_count += 1
                login = contents.get(section, "login")

                if login == "Indisponivel":
                    self._error_block(
                        f"O CONTENT '{section}' E INVALIDO!",
                        "VOCE MARCOU PARA DOWNLOAD UM CONTENT QUE NAO SUPORTA TAL OPERACAO.",
                        "VA EM 'CONTENTS.INI' CORRIGIR O PROBLEMA.",
                    )
                    return False

                if login == "Usuario" and not params.get_any("login_steam"):
                    self._error_block(
                        f"ERRO NO CONTENT '{section}'!",
                        'ESSE CONTENT REQUER LOGIN PARA SER BAIXADO, POREM O CAMPO "login_steam"',
                        "NAO FOI CONFIGURADO EM 'CFG.INI'.",
                    )
                    return False

                if not contents.get(section, "id"):
                    self._error_block(f"ERRO NO CONTENT '{section}'!", "O campo 'id' nao foi configurado.")
                    return False

            elif option == "2":
                configured_count += 1
                if not self._validate_steam_content_path(section):
                    return False

        if configured_count:
            print(f"[GMBR DS] {configured_count} contents foram validados com sucesso")
        else:
            print("[GMBR DS] Nao ha contents configurados")
        return True

    def _validate_steam_content_path(self, section: str) -> bool:
        contents = self._contents()
        params = self._params()
        configured = contents.get(section, "steam")

        if Path(configured).expanduser().exists():
            base = Path(configured).expanduser()
            if self._content_mount_path_from_base(section, base).exists():
                return True

        if params.get_any("pasta_steam") == NOT_FOUND:
            self._error_block(
                "O GMBR DS NAO FOI CAPAZ DE ENCONTRAR A PASTA DO STEAM AUTOMATICMENTE!",
                "PARA PODER USAR SEUS CONTENTS MARCADOS COM A OPCAO 2, INSIRA-A",
                "NO ARQUIVO 'CFG.INI' OU ESCREVA O ENDERECO COMPLETO EM 'CONTENTS.INI'.",
            )
            return False

        for candidate in self._steam_content_candidates(configured):
            if self._content_mount_path_from_base(section, candidate).exists():
                contents.set(section, "steam", candidate)
                return True

        self._error_block(
            f"A PASTA DO CONTENT '{section}' E INVALIDA!",
            "CHEQUE SE ELA EXISTE EM STEAMAPPS OU SE VOCE ESCREVEU CORRETAMENTE:",
            configured,
        )
        return False

    def _content_mount_key(self, section: str) -> str:
        contents = self._contents()
        return contents.get(section, "mount") or section

    def _content_mount_path(self, section: str) -> Path:
        contents = self._contents()
        option = contents.get(section, "opcao")
        if option == "1":
            base = Path(contents.get(section, "pasta"))
        else:
            base = Path(contents.get(section, "steam"))
        return self._content_mount_path_from_base(section, base)

    def _content_mount_path_from_base(self, section: str, base: Path) -> Path:
        subdir = self._contents().get(section, "subpasta")
        if not subdir:
            return base
        if base.name.casefold() == subdir.casefold():
            return base
        return base / subdir

    def _show_main_menu(self) -> int:
        while True:
            self._clear_screen()
            print("__.d8888b.__888b_____d888_888888b.___8888888b._______8888888b.___.d8888b.")
            print("_d88P__Y88b_8888b___d8888_888___88b__888___Y88b______888___Y88b_d88P__Y88b")
            print("_888____888_88888b.d88888_888__d88P__888____888______888____888_Y88b.")
            print("_888________888Y88888P888_8888888K.__888___d88P______888____888___Y888b.")
            print("_888__88888_888_Y888P_888_888___Y88b_8888888P8_______888____888_______Y88b.")
            print("_888____888_888__Y8P__888_888____888_888_T88b________888____888+_______888")
            print("_Y88b__d88P_888___8___888_888___d88P_888__T88b_______888__.d88P_Y88b__d88P")
            print("__Y8888P88__888_______888_8888888P8__888___T88b______8888888P_____Y8888P")
            print()
            print("Garry's Mod Brasil Dedicated Server - GMBR DS")
            print("Feito por Xalalau - Garry s Mod Brasil, GMBR")
            print("http://www.gmbrblog.blogspot.com.br/")
            print("http://mrxalalau.blogspot.com.br/")
            print(f"___________________________________________________________________[{VERSION}]")
            print()

            visible_to_action: dict[int, int] = {}
            displayed = 1

            print(f"{displayed}: Ver as configuracoes carregadas")
            visible_to_action[displayed] = 1
            displayed += 1

            print(f"{displayed}: Instalar/atualizar o servidor de GMod")
            visible_to_action[displayed] = 2
            displayed += 1

            if self._has_installable_contents():
                print(f"{displayed}: Instalar/atualizar os contents")
                visible_to_action[displayed] = 3
                displayed += 1
                print(f"{displayed}: Instalar/atualizar o servidor de GMod e os contents")
                visible_to_action[displayed] = 4
                displayed += 1

            srcds_path = self._require_path(self.steam.srcds_path, "SRCDS")
            if srcds_path.exists():
                print(f"{displayed}: Iniciar o servidor")
                visible_to_action[displayed] = 5
                displayed += 1
                print(f"{displayed}: Forcar montagem de contents")
                visible_to_action[displayed] = 6

            print("0: Sair")
            print()

            choice = input("Escolha: ").strip()
            print()
            if not choice.isdigit():
                continue

            selected = int(choice)
            if selected == 0:
                return 0
            if selected in visible_to_action:
                return visible_to_action[selected]

    def _has_installable_contents(self) -> bool:
        return any(self._contents().get(section, "opcao") == "1" for section in self._contents().sections())

    def _create_dirs(self, paths: Iterable[Path]) -> bool:
        for path in paths:
            try:
                already_exists = path.exists()
                path.mkdir(parents=True, exist_ok=True)
                if not already_exists:
                    print(f"[GMBR DS] Pasta criada: '{path}'")
            except OSError:
                self._error_block("ERRO AO CRIAR PASTA", str(path))
                return False
        return True

    def _steamcmd_app_update(self, app_id: str, install_dir: Path, login: str) -> bool:
        steamcmd_path = self._require_path(self.steam.steamcmd_path, "SteamCMD")
        args = [
            str(steamcmd_path),
            "+login",
            login,
            "+force_install_dir",
            str(install_dir),
            "+app_update",
            app_id,
            "validate",
            "+quit",
        ]

        print(f"[GMBR DS] {self._format_command(args)}\n")

        try:
            result = subprocess.run(args, cwd=steamcmd_path.parent, check=False)
        except OSError as exc:
            self._error_block("ERRO AO EXECUTAR O STEAMCMD", str(exc))
            return False

        if result.returncode != 0:
            print("\n[GMBR DS] Erro! Verifique as ultimas mensagens de problemas e o contents.ini\n")
            return False
        return True

    def _resolve_config_paths(self) -> tuple[Path, Path]:
        if self.config_dir_override:
            return self.config_dir_override / "cfg.ini", self.config_dir_override / "contents.ini"

        deployed_cfg_dir = self.base_dir / "cfg"
        deployed_cfg = deployed_cfg_dir / "cfg.ini"
        deployed_contents = deployed_cfg_dir / "contents.ini"
        if deployed_cfg.exists() or deployed_contents.exists():
            return deployed_cfg, deployed_contents

        filename_suffix = self.system
        resource_dirs = [self.base_dir / "res" / "cfg", self.project_dir / "res" / "cfg"]
        for resource_dir in resource_dirs:
            cfg = resource_dir / f"cfg{filename_suffix}.ini"
            contents = resource_dir / f"contents{filename_suffix}.ini"
            if cfg.exists() and contents.exists():
                return cfg, contents

        return deployed_cfg, deployed_contents

    def _require_cfg_options(self) -> None:
        params = self._params()
        required = [
            "pasta_base",
            "pasta_steam",
            "pasta_servidor",
            "pasta_contents",
            "pasta_steamcmd",
            "login_steam",
            "download_steamcmd",
            "comando_sv",
        ]
        missing = [option for option in required if not params.has_any(option)]
        if missing:
            raise GmbrError("Parametros ausentes: " + ", ".join(missing))

    def _resolve_path(self, value: str, base: Path) -> Path:
        path = Path(value).expanduser()
        if path.is_absolute():
            return path
        return (base / path).resolve()

    def _steam_content_candidates(self, fragment: str) -> Iterable[Path]:
        params = self._params()
        raw = Path(fragment).expanduser()
        yield raw

        steam_root = params.get_any("pasta_steam")
        if steam_root != NOT_FOUND:
            yield self._append_fragment(Path(steam_root), fragment)

        for library in self.steam.libraries:
            yield self._append_fragment(library, fragment)

    def _append_fragment(self, base: Path, fragment: str) -> Path:
        parts = [part for part in re.split(r"[\\/]+", fragment.strip("/\\")) if part]
        return base.joinpath(*parts)

    def _read_steam_libraries(self, libraryfolders: Path) -> list[str]:
        if vdf_lib is not None:
            with libraryfolders.open(encoding="utf-8", errors="replace") as handle:
                data = vdf_lib.load(handle)
            paths = self._find_vdf_paths(data)
        else:
            text = libraryfolders.read_text(encoding="utf-8", errors="replace")
            paths = [
                self._decode_vdf_string(match.group(1))
                for match in re.finditer(r'"path"\s+"((?:\\.|[^"\\])*)"', text)
            ]

        unique: list[str] = []
        for path in paths:
            if path and path not in unique:
                unique.append(path)
        return unique

    def _find_vdf_paths(self, node: object) -> list[str]:
        paths: list[str] = []
        if isinstance(node, dict):
            for key, value in node.items():
                if key == "path" and isinstance(value, str):
                    paths.append(value)
                else:
                    paths.extend(self._find_vdf_paths(value))
        elif isinstance(node, list):
            for item in node:
                paths.extend(self._find_vdf_paths(item))
        return paths

    def _decode_vdf_string(self, value: str) -> str:
        return value.replace(r"\\", "\\").replace(r"\"", '"')

    def _extract_archive(self, archive_path: Path, destination: Path) -> None:
        lower_name = archive_path.name.lower()
        if lower_name.endswith(".zip"):
            with zipfile.ZipFile(archive_path) as archive:
                self._safe_extract_zip(archive, destination)
        elif lower_name.endswith((".tar.gz", ".tgz", ".tar")):
            with tarfile.open(archive_path) as archive:
                self._safe_extract_tar(archive, destination)
        else:
            raise ValueError(f"Formato de arquivo nao suportado: {archive_path.name}")

    def _safe_extract_zip(self, archive: zipfile.ZipFile, destination: Path) -> None:
        root = destination.resolve()
        for member in archive.infolist():
            target = (destination / member.filename).resolve()
            if target != root and root not in target.parents:
                raise ValueError(f"Caminho inseguro no zip: {member.filename}")
        archive.extractall(destination)

    def _safe_extract_tar(self, archive: tarfile.TarFile, destination: Path) -> None:
        root = destination.resolve()
        for member in archive.getmembers():
            target = (destination / member.name).resolve()
            if target != root and root not in target.parents:
                raise ValueError(f"Caminho inseguro no tar: {member.name}")
        try:
            archive.extractall(destination, filter="data")
        except TypeError:
            archive.extractall(destination)

    def _steam_candidates(self) -> list[Path]:
        if self.system == "Linux":
            home = Path.home()
            candidates = [
                home / ".steam",
                home / ".steam" / "steam",
                home / ".local" / "share" / "Steam",
            ]
        else:
            folders = [
                "Steam",
                "Program Files (x86)/Steam",
                "Program Files/Steam",
                "Arquivos de Programas (x86)/Steam",
                "Arquivos de Programas/Steam",
            ]
            candidates = [Path(f"{drive}:/") / folder for drive in "CDEFG" for folder in folders]
            for env_var in ("ProgramFiles(x86)", "ProgramFiles"):
                env_path = os.environ.get(env_var)
                if env_path:
                    candidates.append(Path(env_path) / "Steam")

        deduped: list[Path] = []
        for candidate in candidates:
            if candidate not in deduped:
                deduped.append(candidate)
        return deduped

    def _is_steam_dir(self, path: Path) -> bool:
        return any((path / marker).exists() for marker in self.steam.marker_names)

    def _libraryfolders_path(self, steam_root: Path) -> Path:
        if self.system == "Windows":
            candidates = [steam_root / "steamapps" / self.steam.libraryfolders_name]
        else:
            candidates = [
                steam_root / "steam" / "steamapps" / self.steam.libraryfolders_name,
                steam_root / "steamapps" / self.steam.libraryfolders_name,
                Path.home() / ".local" / "share" / "Steam" / "steamapps" / self.steam.libraryfolders_name,
            ]
        return next((path for path in candidates if path.exists()), candidates[0])

    def _default_steam_info(self) -> SteamInfo:
        if self.system == "Linux":
            return SteamInfo(
                marker_names=["steam.pid", "steam.sh"],
                steamcmd_name="steamcmd.sh",
                srcds_name="srcds_run",
            )
        return SteamInfo(
            marker_names=["steam.dll", "steam.exe"],
            steamcmd_name="steamcmd.exe",
            srcds_name="srcds.exe",
        )

    def _detect_system(self) -> str:
        detected = platform.system()
        if detected in {"Linux", "Windows"}:
            return detected
        raise GmbrError(f"Sistema nao suportado pelo GMBR DS: {detected}")

    def _split_args(self, command: str) -> list[str]:
        if not command:
            return []
        return shlex.split(command, posix=self.system != "Windows")

    def _format_command(self, args: list[str]) -> str:
        if self.system == "Windows":
            return subprocess.list2cmdline(args)
        return shlex.join(args)

    def _clear_screen(self) -> None:
        os.system("cls" if self.system == "Windows" else "clear")

    def _pause(self) -> None:
        try:
            input('\nAperte "Enter" para continuar...')
        except EOFError:
            pass

    def _finish_option(self) -> None:
        print()
        print()
        print(" ___________________________________")
        print("|                                   |")
        print("|    [GMBR DS] Opcao finalizada.    |")
        print("|                                   |\n")
        self._pause()

    def _error_block(self, *lines: str) -> None:
        print("\n[GMBR DS]--------------------------------------------------------------")
        for line in lines:
            print(line)
        print("-----------------------------------------------------------------------\n")

    def _require_path(self, path: Path | None, label: str) -> Path:
        if path is None:
            raise GmbrError(f"Caminho nao inicializado: {label}")
        return path

    def _params(self) -> IniFile:
        if self.params is None:
            raise GmbrError("cfg.ini ainda nao foi carregado")
        return self.params

    def _contents(self) -> IniFile:
        if self.contents is None:
            raise GmbrError("contents.ini ainda nao foi carregado")
        return self.contents


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Garry's Mod Brasil Dedicated Server")
    parser.add_argument(
        "--base-dir",
        type=Path,
        default=Path.cwd(),
        help="Pasta base usada para cfg/, steamcmd, gmodds e contents.",
    )
    parser.add_argument(
        "--config-dir",
        type=Path,
        help="Pasta contendo cfg.ini e contents.ini. Padrao: cfg/ ou res/cfg/ da plataforma.",
    )
    parser.add_argument(
        "--action",
        choices=[
            "menu",
            "show-config",
            "install-server",
            "install-contents",
            "install-all",
            "mount-contents",
            "start-server",
        ],
        default="menu",
        help="Executa uma acao sem passar pelo menu interativo.",
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)

    try:
        app = GmbrDS(base_dir=args.base_dir, config_dir=args.config_dir)
        if not app.bootstrap():
            return 1
        if args.action == "menu":
            app.run_menu()
            return 0
        return 0 if app.run_action(args.action) else 1
    except KeyboardInterrupt:
        print("\n[GMBR DS] Saindo...")
        return 130
    except GmbrError as exc:
        print(f"\n[GMBR DS] {exc}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
