from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import subprocess, time, os, sys

WATCH_EXTENSIONS = ('.png', '.bmp', '.gif', '.toml', '.json')
_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CONVERTER = os.path.join(_ROOT, 'run_asset_converter.py')

class Handler(FileSystemEventHandler):
    def on_any_event(self, event):
        if event.is_directory:
            return
        if any(event.src_path.lower().endswith(ext) for ext in WATCH_EXTENSIONS):
            print(f"[watcher] change detected: {event.src_path}")
            subprocess.run([sys.executable, CONVERTER])

if __name__ == '__main__':
    watch_paths = [
        os.path.join(_ROOT, 'Images'),
    ]
    obs = Observer()
    for p in watch_paths:
        if os.path.isdir(p):
            obs.schedule(Handler(), p, recursive=True)
    obs.start()
    print("[watcher] watching Images/ for asset changes — Ctrl+C to stop")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        obs.stop()
    obs.join()
