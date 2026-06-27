from __future__ import annotations

import configparser
import tempfile
import unittest
from pathlib import Path

import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from gmbr_ds import GmbrDS, GmodInfo, IniFile


class IniFileTests(unittest.TestCase):
    def test_loads_ini_with_comments_and_cross_section_lookup(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "cfg.ini"
            path.write_text(
                """
; comentario
[Pastas]
pasta_base =
pasta_servidor = gmodds

[Servidor]
comando_sv = -game garrysmod +map gm_flatgrass
""".strip(),
                encoding="utf-8",
            )

            ini = IniFile.load(path)

        self.assertEqual(ini.get_any("pasta_servidor"), "gmodds")
        self.assertEqual(ini.get_any("comando_sv"), "-game garrysmod +map gm_flatgrass")


class SteamLibraryTests(unittest.TestCase):
    def test_reads_libraryfolders_paths_without_external_vdf_dependency(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "libraryfolders.vdf"
            path.write_text(
                '''
"libraryfolders"
{
    "0"
    {
        "path" "C:\\\\Program Files (x86)\\\\Steam"
    }
    "1"
    {
        "path" "D:\\\\SteamLibrary"
    }
}
'''.strip(),
                encoding="utf-8",
            )
            app = GmbrDS(base_dir=Path(temp_dir))

            libraries = app._read_steam_libraries(path)

        self.assertEqual(libraries[0], r"C:\Program Files (x86)\Steam")
        self.assertEqual(libraries[1], r"D:\SteamLibrary")


class MountContentsTests(unittest.TestCase):
    def test_mount_contents_uses_configparser_data(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            mount_cfg = root / "garrysmod" / "cfg" / "mount.cfg"
            mount_cfg.parent.mkdir(parents=True)
            mount_cfg.write_text("", encoding="utf-8")

            params_parser = configparser.RawConfigParser(interpolation=None)
            params_parser.add_section("Pastas")
            params_parser.set("Pastas", "pasta_servidor", str(root))

            contents_parser = configparser.RawConfigParser(interpolation=None)
            contents_parser.add_section("Counter-Strike: Source")
            contents_parser.set("Counter-Strike: Source", "opcao", "1")
            contents_parser.set("Counter-Strike: Source", "pasta", str(root / "contents" / "css"))

            app = GmbrDS(base_dir=root)
            app.params = IniFile(root / "cfg.ini", params_parser)
            app.contents = IniFile(root / "contents.ini", contents_parser)
            app.gmod = GmodInfo(mount_cfg=mount_cfg)

            self.assertTrue(app.mount_contents())

            text = mount_cfg.read_text(encoding="utf-8")

        self.assertIn('"mountcfg"', text)
        self.assertIn('"Counter-Strike: Source"', text)
        self.assertIn("contents", text)


if __name__ == "__main__":
    unittest.main()
