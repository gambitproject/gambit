import io
import os
import signal
import sys
from pathlib import Path

import pygambit as gbt

CATALOG_DIR = Path(__file__).resolve().parent.parent.parent / "catalog"


def generate_openspiel_games():
    """Generate NFG/EFG files for OpenSpiel games."""
    try:
        import pyspiel
        from open_spiel.python.algorithms.gambit import export_gambit
    except ImportError:
        print("open_spiel not installed, skipping generation of openspiel games.")
        return

    class TimeoutException(Exception):
        pass

    def timeout_handler(signum, frame):
        raise TimeoutException()

    signal.signal(signal.SIGALRM, timeout_handler)

    openspiel_dir = CATALOG_DIR / "open_spiel"
    openspiel_dir.mkdir(parents=True, exist_ok=True)

    # Suppress C++ stderr from OpenSpiel
    old_stderr = os.dup(sys.stderr.fileno())
    devnull = os.open(os.devnull, os.O_WRONLY)
    os.dup2(devnull, sys.stderr.fileno())

    try:
        games = pyspiel.registered_games()
        for game_type in games:
            name = game_type.short_name
            title = getattr(game_type, "long_name", None)
            if not title:
                title = "[INSERT TITLE]"

            print(f"Attempting to generate {title}...", end=" ", flush=True)

            try:
                game = pyspiel.load_game(name)
            except Exception:
                print("Fail")
                continue

            success = False
            # Try NFG first
            try:
                signal.alarm(2)  # 2 second timeout per game
                nfg = pyspiel.game_to_nfg_string(game)
                signal.alarm(0)

                # Verify it can be loaded
                with io.StringIO(nfg) as f:
                    game_obj = gbt.read_nfg(f)

                game_obj.title = title
                game_obj.description = "[INSERT DESCRIPTION]"

                with open(openspiel_dir / f"{name}.nfg", "w") as f:
                    f.write(game_obj.to_nfg())

                print("Success (NFG)")
                success = True
            except Exception:
                signal.alarm(0)

            if success:
                continue

            # Try EFG
            try:
                signal.alarm(2)
                export_str = export_gambit(game)
                signal.alarm(0)

                with io.StringIO(export_str) as f:
                    game_obj = gbt.read_efg(f)

                game_obj.title = title
                game_obj.description = "[INSERT DESCRIPTION]"

                with open(openspiel_dir / f"{name}.efg", "w") as f:
                    f.write(game_obj.to_efg())

                print("Success (EFG)")
                success = True
            except Exception:
                signal.alarm(0)

            if not success:
                print("Fail")
    finally:
        os.dup2(old_stderr, sys.stderr.fileno())
        os.close(devnull)
        os.close(old_stderr)


if __name__ == "__main__":
    generate_openspiel_games()
