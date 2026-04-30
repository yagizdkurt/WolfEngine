import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'tools'))

import run_asset_converter
import watch_and_run

_ROOT = os.path.dirname(os.path.abspath(__file__))

if __name__ == '__main__':
    print("[WEAP] Running asset converter...")
    run_asset_converter.run(project_dir=_ROOT, python=sys.executable)
    print("[WEAP] Starting watcher...")
    watch_and_run.start_watcher()
