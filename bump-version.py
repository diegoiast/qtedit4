#! /usr/bin/python3

import re
import json
import argparse
import subprocess

def get_iss_version(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    match = re.search(r'#define\s+VersionString\s+"([^"]+)"', content)
    return match.group(1) if match else None

def update_iss_version(file_path, new_version):
    with open(file_path, 'r+', encoding='utf-8') as f:
        content = f.read()
        content_new = re.sub(
            r'(#define\s+VersionString\s+")([^"]+)(")',
            fr'\g<1>{new_version}\g<3>',
            content
        )
        f.seek(0)
        f.write(content_new)
        f.truncate()

def get_cpp_version(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    match = re.search(r'QCoreApplication::setApplicationVersion\("([^"]+)"\)', content)
    return match.group(1) if match else None

def update_cpp_version(file_path, new_version):
    with open(file_path, 'r+', encoding='utf-8') as f:
        content = f.read()
        content_new = re.sub(
            r'(QCoreApplication::setApplicationVersion\(")([^"]+)("\))',
            fr'\g<1>{new_version}\g<3>',
            content
        )
        f.seek(0)
        f.write(content_new)
        f.truncate()

def get_json_versions(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    versions = {env: v['latest-version'] for env, v in data['updates'].items()}
    return versions

def update_json_versions(file_path, new_version, update_all=False, qt_version=None):
    with open(file_path, 'r+', encoding='utf-8') as f:
        data = json.load(f)

        environments = data['updates'].keys()
        if not update_all:
            environments = [k for k in environments if 'testing' in k]

        for env in environments:
            env_data = data['updates'][env]
            env_data['latest-version'] = new_version
            # Update open-url version
            if 'open-url' in env_data:
                env_data['open-url'] = re.sub(
                    r'/tag/v[\d\w\.\-]+',
                    f'/tag/v{new_version}',
                    env_data['open-url']
                )
            # Update download-url version and qt version, preserving file extension
            if 'download-url' in env_data:
                # Replace v<version> in the path
                env_data['download-url'] = re.sub(
                    r'/download/v[\d\w\.\-]+/',
                    f'/download/v{new_version}/',
                    env_data['download-url']
                )
                # Replace the entire filename segment before the extension
                env_data['download-url'] = re.sub(
                    r'qtedit4-qt[\d\.]+-v[\d\w\.\-]+-x86_64',
                    f'qtedit4-qt{qt_version}-v{new_version}-x86_64' if qt_version else f'qtedit4-qt6.8.3-v{new_version}-x86_64',
                    env_data['download-url']
                )

        f.seek(0)
        json.dump(data, f, indent=3)
        f.truncate()

def print_versions(title, iss_file, cpp_file, json_file):
    print(f"\n{title.center(40, '-')}")
    iss_version = get_iss_version(iss_file)
    cpp_version = get_cpp_version(cpp_file)
    json_versions = get_json_versions(json_file)
    print(f"{iss_file}: {iss_version}")
    print(f"{cpp_file}: {cpp_version}")
    print(f"{json_file}:")
    for env, ver in json_versions.items():
        print(f"  {env}: {ver}")

def git_add(files):
    try:
        subprocess.check_call(['git', 'add'] + files)
        print(f"\nStaged files for commit: {', '.join(files)}")
    except Exception as e:
        print(f"Error staging files for git: {e}")

def reformat_json(json_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        data = json.load(f)
    with open(json_file, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=3)
    print(f"Reformatted {json_file}")

def main():
    parser = argparse.ArgumentParser(description='Update version numbers across project files')
    parser.add_argument('new_version', nargs='?', help='New version number to set')
    parser.add_argument('--all', action='store_true', help='Update all environments in updates.json')
    parser.add_argument('--git', action='store_true', help='Stage files for git commit')
    parser.add_argument('--qt-version', help='Set Qt version in updates.json URLs (e.g. 6.8.3)')
    parser.add_argument('--reformat-json', action='store_true', help='Only reformat (pretty print) the JSON file and do nothing else')
    parser.add_argument('--iss', default='setup_script.iss', help='Path to .iss file')
    parser.add_argument('--cpp', default='src/main.cpp', help='Path to main.cpp file')
    parser.add_argument('--json', default='updates.json', help='Path to updates.json file')
    args = parser.parse_args()

    iss_file = args.iss
    cpp_file = args.cpp
    json_file = args.json

    # Only reformat JSON if flag is set
    if args.reformat_json:
        reformat_json(json_file)
        return

    # new_version is required unless --reformat-json is used
    if not args.new_version:
        parser.error("the following arguments are required: new_version (unless --reformat-json is used)")

    # Print versions before update
    print_versions('Current Versions', iss_file, cpp_file, json_file)

    # Update files
    update_iss_version(iss_file, args.new_version)
    update_cpp_version(cpp_file, args.new_version)
    update_json_versions(json_file, args.new_version, update_all=args.all, qt_version=args.qt_version)

    # Print versions after update
    print_versions('Updated Versions', iss_file, cpp_file, json_file)

    # Stage files for git only if --git is specified
    if args.git:
        git_add([iss_file, cpp_file, json_file])

if __name__ == "__main__":
    main()
