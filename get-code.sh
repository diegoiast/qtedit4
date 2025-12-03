#! /bin/bash

# Assume you are on a trip, and want to work on the code of the project.
# Before leaving, run this script, and the configure with QTEDIT4_WORK_OFFLINE=on
#
# Now you should be able to work offline (the CMake stuff will not query online servers.
#
# When you are done - for each of the repos you modified, create a new branch,
# and push and create PRs.

set -e

clone_or_update_repo() {
    local owner=$1
    local repo=$2
    local release=$3
    local target_dir="lib/$repo"

    if [ -d "$target_dir" ]; then
        echo -n "Updating $target_dir"
        (cd "$target_dir" && git fetch -q origin)
    else
        echo -n "Cloning $target_dir (gh:$owner/$repo)"
        git clone -q "https://github.com/$owner/$repo" "$target_dir"
        cd $target_dir
        git remote add github "git@github.com:$owner/$repo"
        cd - > /dev/null
    fi
    echo "."

    (
        cd "$target_dir"
        if ! git checkout -q $release 2>/dev/null &&
           ! git checkout -q origin/$release 2>/dev/null; then
            echo "Failed to checkout $release for $repo. Falling back to master/main."
            git checkout -q master || git checkout -q main
        fi
    )
}

while read -r line; do
    if [[ $line =~ CPMAddPackage\(\"gh:([^/]+)/([^#]+)#([^\"]+) ]]; then
        owner=${BASH_REMATCH[1]}
        repo=${BASH_REMATCH[2]}
        release=${BASH_REMATCH[3]}

        clone_or_update_repo "$owner" "$repo" "$release"
    fi
done < <(grep CPMAddPackage CMakeLists.txt)
