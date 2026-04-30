try:
    Import("env")
    _pio_mode = True
except NameError:
    _pio_mode = False

import os
import subprocess
import sys


def run(project_dir, python):
    # Ensure required packages are available in this Python environment.
    # PlatformIO runs in its own venv which may be missing them.
    try:
        import PIL
        if sys.version_info < (3, 11):
            import tomli  # noqa: F401
    except ImportError:
        reqs = os.path.join(project_dir, "tools", "requirements.txt")
        print("Asset converter: installing required packages...")
        subprocess.check_call([python, "-m", "pip", "install", "-r", reqs])

    # Step 1 -- generate palette headers
    result = subprocess.run(
        [
            python,
            os.path.join(project_dir, "tools", "generate_palettes.py"),
            os.path.join(project_dir, "tools", "palettes"),
            os.path.join(project_dir, "src", "GeneratedAssets", "palettes"),
        ],
        cwd=project_dir,
    )
    if result.returncode != 0:
        print("ERROR: Palette generator failed (see output above)")
        sys.exit(result.returncode)

    # Step 2 -- convert image assets
    result = subprocess.run(
        [
            python,
            os.path.join(project_dir, "tools", "asset_converter.py"),
            os.path.join(project_dir, "Images"),
            os.path.join(project_dir, "src", "GeneratedAssets", "sprites"),
            os.path.join(project_dir, "tools", "palettes"),
        ],
        cwd=project_dir,
    )
    if result.returncode != 0:
        print("ERROR: Asset converter failed (see output above)")
        sys.exit(result.returncode)


if _pio_mode:
    run(env.subst("$PROJECT_DIR"), sys.executable)
elif __name__ == '__main__':
    _project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    run(_project_dir, sys.executable)
