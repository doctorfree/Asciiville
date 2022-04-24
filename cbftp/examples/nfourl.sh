#!/bin/bash

# This script is meant to be executed as an external script through the
# cbftp browse screen when a directory containing an nfo file is selected.
# It takes a site/path/item and uses the cbftp api to locate an nfo file,
# and then opens an url within the file with a web browser.
# This obviously only works when cbftp is running on your desktop system.
# External requirements: curl, jq

cb_api_port=55477
web_browser=x-www-browser

cb_api() {
  base="https://localhost:$cb_api_port"
  url=$base/$1
  result=$(curl -sSfku ":$api_token" "$url" 2>&1)
  status=$?
  if [ $status -ne 0 ]; then
    echo "$url failed: $result"
    exit $status
  fi
}

use_nfo_contents() {
  nfo_contents=$1
  url=$(echo "$nfo_contents" | grep -o -E "http(s)?://.*")
  if [ -n "$url" ]; then
    $web_browser "$url"
  else
    echo "No url found in nfo."
    exit 1
  fi
}

use_nfo_path() {
  nfo_path=$1
  cb_api "file?site=$site&path=$nfo_path"
  nfo_contents=$result
  use_nfo_contents "$nfo_contents"
}

api_token=$1
operation=$2

if [[ "$operation" == "browse-site"* ]]; then
  site=$3
  path=$4
  items=$5
  item=$(echo "$items" | sed 's/,.*//g')  # this script ignores multiple selected items
  cb_api "path?site=$site&path=$path"
  maindirlist=$result
  type=$(echo "$maindirlist" | jq -c -r '.[] | select(.name | contains('"\"$item\""')) | .type')
  if [ -z "$type" ]; then
    echo "Error: item not found."
    exit 1
  fi
  if [ "$type" == "DIR" ]; then
    cb_api "path?site=$site&path=$path/$item"
    dirlist=$result
    nfo=$(echo "$dirlist" | jq -c -r '.[].name | select(endswith(".nfo"))')
    if [ -z "$nfo" ]; then
      echo "No nfo file found."
      exit 1
    fi
    use_nfo_path "$path/$item/$nfo"
  elif [ "$type" == "FILE" ]; then
    use_nfo_path "$path/$item"
  else
    echo "Error: unsupported file type selected"
    exit 1
  fi
elif [[ "$operation" == "browse-local"* ]]; then
  path=$3
  item=$4
  if [ -d "$path/$item" ]; then
    nfo_contents=$(bash -c "cat $path/$item/*.nfo")
    if [ -z "$nfo_contents" ]; then
      echo "No nfo file found."
      exit 1
    fi
    use_nfo_contents "$nfo_contents"
  elif [ -f "$path/$item" ]; then
    nfo_contents=$(cat "$path/$item")
    use_nfo_contents "$nfo_contents"
  else
    echo "Error: unsupported file type selected"
    exit 1
  fi
else
  echo "Error: unknown operation: $operation"
  exit 1
fi
exit 0
