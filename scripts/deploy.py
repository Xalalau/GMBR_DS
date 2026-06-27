#!/usr/bin/env python3
from __future__ import annotations

import argparse
import platform
import shutil
import zipfile
from pathlib import Path


VERSION = "v.2.8.6"


def copy_tree(system: str, root: Path) -> Path:
    release_dir = root / "release"
    server_dir = release_dir / "Servidor"
    cfg_dir = server_dir / "cfg"

    if server_dir.exists():
        shutil.rmtree(server_dir)

    cfg_dir.mkdir(parents=True)
    shutil.copy2(root / "src" / "gmbr_ds.py", server_dir / "GMBR_DS.py")
    shutil.copy2(root / "requirements.txt", server_dir / "requirements.txt")
    shutil.copy2(root / "res" / "LEIAME.txt", server_dir / "LEIAME.txt")
    shutil.copy2(root / "res" / "cfg" / f"cfg{system}.ini", cfg_dir / "cfg.ini")
    shutil.copy2(root / "res" / "cfg" / f"contents{system}.ini", cfg_dir / "contents.ini")
    return server_dir


def zip_server(system: str, root: Path, server_dir: Path) -> Path:
    zip_dir = root / "release" / "zip"
    zip_dir.mkdir(parents=True, exist_ok=True)
    zip_path = zip_dir / f"{system}_GMBR_DS_{VERSION}.zip"

    if zip_path.exists():
        zip_path.unlink()

    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for path in server_dir.rglob("*"):
            archive.write(path, path.relative_to(server_dir.parent))

    return zip_path


def deploy(system: str, root: Path) -> Path:
    server_dir = copy_tree(system, root)
    zip_path = zip_server(system, root, server_dir)
    shutil.rmtree(server_dir)
    return zip_path


def main() -> int:
    parser = argparse.ArgumentParser(description="Gera pacote portavel do GMBR DS.")
    parser.add_argument(
        "--system",
        choices=["Windows", "Linux", "all"],
        default=platform.system() if platform.system() in {"Windows", "Linux"} else "all",
        help="Plataforma do cfg que sera empacotado.",
    )
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    systems = ["Windows", "Linux"] if args.system == "all" else [args.system]

    for system in systems:
        zip_path = deploy(system, root)
        print(f"[GMBR DS] Pacote gerado: {zip_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
