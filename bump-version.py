#! /usr/bin/python3

# This script is used to update versions of the main app
# and Qt dependencies. We have the same app version in several places:
#
# 1) setup_script.iss - which creates the EXE installer on windows
# 2) build.sh - script which creates a local appimage
# 3) main.cpp - runtime version
# 4) the updates.json file
#
# Qt versions is also spread:
# 1) build.bat
# 2) build.sh
# 3) github actions
#
# This script is vibe coded to the death. I just ask chatgpt to add features, and paste
# the whole script. Seems to be working for such simple task. No need to be logged it, and
# it gives good results.

import re
import json
import argparse
import subprocess

def detect_line_ending(text):
    if '\r\n' in text:
        return '\r\n'
    elif '\r' in text:
        return '\r'
    return '\n'

def get_iss_version(file_path):
    with open(file_path, 'rb') as f:
        content = f.read().decode('utf-8')
    match = re.search(r'#define\s+VersionString\s+"([^"]+)"', content)
    return match.group(1) if match else None

def update_iss_version(file_path, new_version):
    with open(file_path, 'rb+') as f:
        content = f.read().decode('utf-8')
        line_ending = detect_line_ending(content)
        content_new = re.sub(
            r'(#define\s+VersionString\s+")([^"]+)(")',
            fr'\g<1>{new_version}\g<3>',
            content
        )
        f.seek(0)
        f.write(content_new.replace('\n', line_ending).encode('utf-8'))
        f.truncate()

def get_cpp_version(file_path):
    with open(file_path, 'rb') as f:
        content = f.read().decode('utf-8')
    match = re.search(r'QCoreApplication::setApplicationVersion\("([^"]+)"\)', content)
    return match.group(1) if match else None

def update_cpp_version(file_path, new_version):
    with open(file_path, 'rb+') as f:
        content = f.read().decode('utf-8')
        line_ending = detect_line_ending(content)
        content_new = re.sub(
            r'(QCoreApplication::setApplicationVersion\(")([^"]+)("\))',
            fr'\g<1>{new_version}\g<3>',
            content
        )
        f.seek(0)
        f.write(content_new.replace('\n', line_ending).encode('utf-8'))
        f.truncate()

def get_json_versions(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    versions = {env: v['latest-version'] for env, v in data['updates'].items()}
    return versions

def update_json_versions(file_path, new_version, update_all=False, qt_version=None):
    with open(file_path, 'r', encoding='utf-8') as f:
        original_content = f.read()
        line_ending = detect_line_ending(original_content)
        data = json.loads(original_content)

    environments = data['updates'].keys()
    if not update_all:
        environments = [k for k in environments if 'testing' in k]

    for env in environments:
        env_data = data['updates'][env]
        env_data['latest-version'] = new_version
        if 'open-url' in env_data:
            env_data['open-url'] = re.sub(
                r'/tag/v[\d\w\.\-]+',
                f'/tag/v{new_version}',
                env_data['open-url']
            )
        if 'download-url' in env_data:
            env_data['download-url'] = re.sub(
                r'/download/v[\d\w\.\-]+/',
                f'/download/v{new_version}/',
                env_data['download-url']
            )
            env_data['download-url'] = re.sub(
                r'qtedit4-qt[\d\.]+-v[\d\w\.\-]+-x86_64',
                f'qtedit4-qt{qt_version}-v{new_version}-x86_64' if qt_version else f'qtedit4-qt6.8.3-v{new_version}-x86_64',
                env_data['download-url']
            )

    # Dump to JSON string, then apply original line endings
    new_json = json.dumps(data, indent=4)
    new_json = new_json.replace('\n', line_ending)

    with open(file_path, 'w', encoding='utf-8', newline='') as f:
        f.write(new_json)

def get_github_qt_versions(yaml_file_path):
    try:
        with open(yaml_file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        return None

    match = re.search(r'qt_version:\s*\n((?:\s*-\s*"[0-9.]+"\s*\n?)+)', content)
    if not match:
        return None

    versions_block = match.group(1)
    versions = re.findall(r'"([\d\.]+)"', versions_block)
    return versions

def get_qt_version_from_build_sh(path):
    try:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        return None
    match = re.search(r'^QT_VERSION="([^"]+)"', content, re.MULTILINE)
    return match.group(1) if match else None

def get_qt_version_from_build_bat(path):
    try:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        return None
    match = re.search(r'c:\\Qt\\([\d.]+)\\', content, re.IGNORECASE)
    return match.group(1) if match else None

def print_versions(title, iss_file, cpp_file, json_file, workflow_file=None, build_sh=None, build_bat=None):
    print(f"\n{title.center(40, '-')}")
    iss_version = get_iss_version(iss_file)
    cpp_version = get_cpp_version(cpp_file)
    json_versions = get_json_versions(json_file)
    print(f"{iss_file}: {iss_version}")
    print(f"{cpp_file}: {cpp_version}")
    print(f"{json_file}:")
    for env, ver in json_versions.items():
        print(f"  {env}: {ver}")

    if workflow_file:
        qt_versions = get_github_qt_versions(workflow_file)
        if qt_versions:
            print(f"{workflow_file}:")
            for v in qt_versions:
                print(f"  Qt: {v}")
        else:
            print(f"{workflow_file}: Qt version not found.")

    if build_sh:
        qt_sh = get_qt_version_from_build_sh(build_sh)
        print(f"{build_sh}: Qt {qt_sh or 'version not found'}")

    if build_bat:
        qt_bat = get_qt_version_from_build_bat(build_bat)
        print(f"{build_bat}: Qt {qt_bat or 'version not found'}")


def git_add(files):
    try:
        subprocess.check_call(['git', 'add'] + files)
        print(f"\nStaged files for commit: {', '.join(files)}")
    except Exception as e:
        print(f"Error staging files for git: {e}")

def reformat_json(json_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        original_content = f.read()
        line_ending = detect_line_ending(original_content)
        data = json.loads(original_content)

    formatted = json.dumps(data, indent=4).replace('\n', line_ending)

    with open(json_file, 'w', encoding='utf-8', newline='') as f:
        f.write(formatted)

    print(f"Reformatted {json_file}")

def update_build_bat(build_bat_path, qt_version):
    if not qt_version:
        return
    with open(build_bat_path, 'rb') as f:
        content = f.read().decode('utf-8')
        line_ending = detect_line_ending(content)
        lines = content.splitlines()

    new_lines = []
    for line in lines:
        if line.strip().startswith('SET PATH='):
            new_line = re.sub(
                r'(c:\\Qt\\)([\d\.]+)(\\[^;\\]+)',
                rf'\g<1>{qt_version}\g<3>',
                line
            )
            new_lines.append(new_line)
        else:
            new_lines.append(line)

    new_content = line_ending.join(new_lines) + line_ending
    with open(build_bat_path, 'wb') as f:
        f.write(new_content.encode('utf-8'))
    print(f"Updated Qt version in {build_bat_path}")

def update_build_sh(build_sh_path, app_version=None, qt_version=None):
    with open(build_sh_path, 'rb') as f:
        content = f.read().decode('utf-8')
        line_ending = detect_line_ending(content)
        lines = content.splitlines()
    new_lines = []
    for line in lines:
        if app_version and line.strip().startswith('APP_VERSION='):
            new_lines.append(f'APP_VERSION="{app_version}"')
        elif qt_version and line.strip().startswith('QT_VERSION='):
            new_lines.append(f'QT_VERSION="{qt_version}"')
        else:
            new_lines.append(line)
    new_content = line_ending.join(new_lines) + line_ending
    with open(build_sh_path, 'wb') as f:
        f.write(new_content.encode('utf-8'))

def update_github_workflow_qt_version(yaml_file_path, qt_version):
    if not qt_version:
        return
    with open(yaml_file_path, 'r', encoding='utf-8') as f:
        original_content = f.read()
        line_ending = detect_line_ending(original_content)

    pattern = re.compile(r'(qt_version:\s*\n(?:\s*-\s*"[0-9.]+"\s*\n)+)', re.MULTILINE)

    def replace_versions(match):
        indent = re.search(r'^(\s*)-', match.group(1), re.MULTILINE).group(1)
        return f'qt_version:\n{indent}- "{qt_version}"\n'

    updated_content = pattern.sub(replace_versions, original_content)
    with open(yaml_file_path, 'w', encoding='utf-8') as f:
        f.write(updated_content.replace('\n', line_ending))
    print(f"Updated Qt version in {yaml_file_path}")

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
    parser.add_argument('--build-sh', default='build.sh', help='Path to build.sh file')
    parser.add_argument('--build-bat', default='build.bat', help='Path to build.bat file')
    args = parser.parse_args()

    iss_file = args.iss
    cpp_file = args.cpp
    json_file = args.json
    workflow_file = '.github/workflows/build.yml'

    if args.reformat_json:
        reformat_json(json_file)
        return

    print_versions('Current Versions', iss_file, cpp_file, json_file, workflow_file, build_sh=args.build_sh, build_bat=args.build_bat)
    if not args.new_version:
        return

    update_iss_version(iss_file, args.new_version)
    update_cpp_version(cpp_file, args.new_version)
    update_json_versions(json_file, args.new_version, update_all=args.all, qt_version=args.qt_version)

    print_versions('Updated Versions', iss_file, cpp_file, json_file, workflow_file, build_sh=args.build_sh, build_bat=args.build_bat)

    update_build_sh(args.build_sh, app_version=args.new_version, qt_version=args.qt_version)
    update_build_bat(args.build_bat, qt_version=args.qt_version)
    update_github_workflow_qt_version(workflow_file, qt_version=args.qt_version)

    if args.git:
        git_add([iss_file, cpp_file, json_file, args.build_sh, args.build_bat, workflow_file])

if __name__ == "__main__":
    main()
