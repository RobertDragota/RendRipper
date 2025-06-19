#!/usr/bin/env python3
import json
import sys
from pathlib import Path

def load_def(path: Path):
    with open(path) as f:
        data = json.load(f)
    if 'inherits' in data:
        parent = path.parent / f"{data['inherits']}.def.json"
        if parent.exists():
            base = load_def(parent)
            # merge overrides dictionaries
            base_overrides = base.get('overrides', {})
            child_overrides = data.get('overrides', {})
            base_overrides.update(child_overrides)
            base['overrides'] = base_overrides
            base.update({k: v for k, v in data.items() if k != 'overrides'})
            return base
    return data

def main():
    if len(sys.argv) != 2:
        print('Usage: check_printer_config.py <printer_def.json>')
        return
    conf = load_def(Path(sys.argv[1]))
    ov = conf.get('overrides', {})
    def get_val(key):
        item = ov.get(key, {})
        return item.get('value', item.get('default_value'))
    print('machine_width:', get_val('machine_width'))
    print('machine_depth:', get_val('machine_depth'))
    print('machine_height:', get_val('machine_height'))
    print('machine_center_is_zero:', get_val('machine_center_is_zero'))
    plat = conf.get('metadata', {}).get('platform_offset')
    print('platform_offset:', plat)

if __name__ == '__main__':
    main()
