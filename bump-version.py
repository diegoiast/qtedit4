#! /usr/bin/python3

import re
import json
import argparse
import subprocess
import xml.etree.ElementTree as ET # Added for XML (plist) handling

def detect_line_ending(text):
    """Detects the line ending style of a given text."""
    if '\r\n' in text:
        return '\r\n'
    elif '\r' in text:
        return '\r'
    return '\n'

def get_iss_version(file_path):
    """Reads the version string from an Inno Setup (.iss) file."""
    with open(file_path, 'rb') as f:
        content = f.read().decode('utf-8')
    match = re.search(r'#define\s+VersionString\s+"([^"]+)"', content)
    return match.group(1) if match else None

def update_iss_version(file_path, new_version):
    """Updates the version string in an Inno Setup (.iss) file."""
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
    print(f"Updated VersionString in {file_path} to {new_version}")

def get_cpp_version(file_path):
    """Reads the application version from a C++ source file."""
    with open(file_path, 'rb') as f:
        content = f.read().decode('utf-8')
    match = re.search(r'QCoreApplication::setApplicationVersion\("([^"]+)"\)', content)
    return match.group(1) if match else None

def update_cpp_version(file_path, new_version):
    """Updates the application version in a C++ source file."""
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
    print(f"Updated QCoreApplication::setApplicationVersion in {file_path} to {new_version}")

def get_json_versions(file_path):
    """Reads all 'latest-version' entries from a JSON updates file."""
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    versions = {env: v['latest-version'] for env, v in data['updates'].items()}
    return versions

def update_json_versions(file_path, new_version, update_all=False, qt_version=None):
    """Updates version numbers and URLs in a JSON updates file."""
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
            # This regex needs to be careful not to match other parts of the URL
            # Assuming the format is 'qtedit4-qtX.Y.Z-vVERSION-x86_64'
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
    print(f"Updated versions and URLs in {file_path} to {new_version}")

def get_plist_version(file_path):
    """Reads CFBundleVersion and CFBundleShortVersionString from a .plist file."""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the actual <plist> root element start to handle XML declaration/DOCTYPE
    plist_start_index = content.find('<plist')
    if plist_start_index == -1:
        print(f"Error: <plist> tag not found in {file_path}")
        return None, None

    xml_content_only = content[plist_start_index:]
    root = ET.fromstring(xml_content_only)

    dict_elem = root.find('dict')
    if dict_elem is None:
        return None, None

    version = None
    short_version = None
    # Iterate through dict children to find keys and their string values
    children = list(dict_elem)
    for i in range(len(children)):
        if children[i].tag == 'key':
            if children[i].text == 'CFBundleVersion' and i + 1 < len(children) and children[i+1].tag == 'string':
                version = children[i+1].text
            elif children[i].text == 'CFBundleShortVersionString' and i + 1 < len(children) and children[i+1].tag == 'string':
                short_version = children[i+1].text
    return version, short_version

def update_plist_version(file_path, new_version):
    """Updates CFBundleVersion and CFBundleShortVersionString in a .plist file."""
    with open(file_path, 'r', encoding='utf-8') as f:
        original_content = f.read()

    # Preserve original line endings
    line_ending = detect_line_ending(original_content)

    # Extract XML declaration and DOCTYPE for reassembly
    xml_decl_match = re.match(r'<\?xml[^?]+\?>', original_content)
    xml_declaration = xml_decl_match.group(0) + '\n' if xml_decl_match else ''

    doctype_match = re.search(r'<!DOCTYPE plist[^>]+>', original_content)
    doctype = doctype_match.group(0) + '\n' if doctype_match else ''

    # Find the actual <plist> root element start for parsing
    plist_start_index = original_content.find('<plist')
    if plist_start_index == -1:
        print(f"Error: <plist> tag not found in {file_path}")
        return

    xml_content_only = original_content[plist_start_index:]
    root = ET.fromstring(xml_content_only)

    dict_elem = root.find('dict')
    if dict_elem is None:
        print(f"Error: No <dict> element found in {file_path}")
        return

    changed = False
    children = list(dict_elem)
    for i in range(len(children)):
        if children[i].tag == 'key':
            if children[i].text == 'CFBundleVersion' and i + 1 < len(children) and children[i+1].tag == 'string':
                if children[i+1].text != new_version:
                    children[i+1].text = new_version
                    changed = True
            elif children[i].text == 'CFBundleShortVersionString' and i + 1 < len(children) and children[i+1].tag == 'string':
                if children[i+1].text != new_version:
                    children[i+1].text = new_version
                    changed = True

    if changed:
        # Reconstruct the XML string, without XML declaration initially
        updated_xml_content = ET.tostring(root, encoding='UTF-8', xml_declaration=False).decode('utf-8')

        # Reassemble with original declaration and doctype
        final_content = xml_declaration + doctype + updated_xml_content

        # Ensure consistent line endings for the entire file
        final_content = final_content.replace('\n', line_ending)

        with open(file_path, 'w', encoding='utf-8', newline='') as f:
            f.write(final_content)
        print(f"Updated CFBundleVersion and CFBundleShortVersionString in {file_path} to {new_version}")
    else:
        print(f"No change needed for CFBundleVersion and CFBundleShortVersionString in {file_path} (already {new_version})")

def print_versions(title, iss_file, cpp_file, json_file, plist_file):
    """Prints the current versions from all tracked files."""
    print(f"\n{title.center(40, '-')}")
    iss_version = get_iss_version(iss_file)
    cpp_version = get_cpp_version(cpp_file)
    json_versions = get_json_versions(json_file)
    plist_version, plist_short_version = get_plist_version(plist_file)

    print(f"{iss_file}: {iss_version}")
    print(f"{cpp_file}: {cpp_version}")
    print(f"{json_file}:")
    for env, ver in json_versions.items():
        print(f"  {env}: {ver}")
    print(f"{plist_file}: CFBundleVersion={plist_version}, CFBundleShortVersionString={plist_short_version}")

def git_add(files):
    """Stages specified files for a git commit."""
    subprocess.check_call(['git', 'add'] + files)
    print(f"\nStaged files for commit: {', '.join(files)}")

def reformat_json(json_file):
    """Reformats (pretty prints) a JSON file."""
    with open(json_file, 'r', encoding='utf-8') as f:
        original_content = f.read()
        line_ending = detect_line_ending(original_content)
        data = json.loads(original_content)

    formatted = json.dumps(data, indent=4).replace('\n', line_ending)

    with open(json_file, 'w', encoding='utf-8', newline='') as f:
        f.write(formatted)
    print(f"Reformatted {json_file}")

def update_build_bat(build_bat_path, qt_version):
    """Updates the Qt version in a Windows build.bat script."""
    if not qt_version:
        return
    with open(build_bat_path, 'rb') as f:
        content = f.read().decode('utf-8')
        line_ending = detect_line_ending(content)
        lines = content.splitlines()

    new_lines = []
    for line in lines:
        if line.strip().startswith('SET PATH='):
            # Replace any Qt path like c:\Qt\6.8.0\...
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
    """Updates APP_VERSION and QT_VERSION in a Unix build.sh script."""
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
    print(f"Updated build.sh file {build_sh_path}")

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
    parser.add_argument('--plist', default='Info.plist', help='Path to Info.plist file (for macOS apps)') # Added plist argument
    args = parser.parse_args()

    iss_file = args.iss
    cpp_file = args.cpp
    json_file = args.json
    plist_file = args.plist # Get plist file path

    # Only reformat JSON if flag is set
    if args.reformat_json:
        reformat_json(json_file)
        return

    # new_version is required unless --reformat-json is used
    if not args.new_version:
        parser.error("the following arguments are required: new_version (unless --reformat-json is used)")

    print_versions('Current Versions', iss_file, cpp_file, json_file, plist_file)

    update_iss_version(iss_file, args.new_version)
    update_cpp_version(cpp_file, args.new_version)
    update_json_versions(json_file, args.new_version, update_all=args.all, qt_version=args.qt_version)
    update_plist_version(plist_file, args.new_version) # Call plist update function

    print_versions('Updated Versions', iss_file, cpp_file, json_file, plist_file)

    update_build_sh(args.build_sh, app_version=args.new_version, qt_version=args.qt_version)
    update_build_bat(args.build_bat, qt_version=args.qt_version)

    if args.git:
        git_add([iss_file, cpp_file, json_file, args.build_sh, args.build_bat, plist_file]) # Add plist_file to git staging

if __name__ == "__main__":
    main()
