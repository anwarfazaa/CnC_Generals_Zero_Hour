#!/usr/bin/env python3
"""
Fix Windows-style path separators in #include statements for Unix/Linux builds
"""

import os
import re
import sys

def fix_include_paths(file_path):
    """Fix backslash path separators in include statements"""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return False

    original_content = content

    # Pattern to match #include statements with backslashes
    # Matches: #include "path\with\backslashes.h"
    pattern = r'(#include\s+")([^"]*\\[^"]*)(")'

    def replace_backslashes(match):
        prefix = match.group(1)
        path = match.group(2)
        suffix = match.group(3)
        # Replace backslashes with forward slashes
        fixed_path = path.replace('\\', '/')
        return prefix + fixed_path + suffix

    content = re.sub(pattern, replace_backslashes, content)

    # If content changed, write it back
    if content != original_content:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Fixed: {file_path}")
            return True
        except Exception as e:
            print(f"Error writing {file_path}: {e}")
            return False

    return False

def process_directory(directory):
    """Process all C/C++ files in directory recursively"""
    fixed_count = 0

    for root, dirs, files in os.walk(directory):
        # Skip build directories and git
        dirs[:] = [d for d in dirs if d not in ['.git', 'build', 'Build', 'BUILD']]

        for file in files:
            if file.endswith(('.h', '.hpp', '.c', '.cpp', '.cxx')):
                file_path = os.path.join(root, file)
                if fix_include_paths(file_path):
                    fixed_count += 1

    return fixed_count

if __name__ == '__main__':
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = 'Generals/Code'

    print(f"Fixing include paths in: {directory}")
    count = process_directory(directory)
    print(f"\nFixed {count} files")
