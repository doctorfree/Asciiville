#!/usr/bin/env python3

# This script is meant to be executed as an external script through the
# cbftp browse screen. It will run "site rescan" in the currently
# selected directory, or in the current directory if a file is selected.

cb_api_port = 55477

import json
import ssl
import sys
import urllib3

urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

def req(path, body=None):
  http = urllib3.PoolManager(cert_reqs=ssl.CERT_NONE)
  headers = urllib3.make_headers(basic_auth=':%s' % api_token)
  command = 'GET' if body is None else 'POST'
  r = http.request(command, 'https://localhost:%s/%s' % (cb_api_port, path),
    headers=headers, body=None if body is None else json.dumps(body))
  if r.status >= 400:
    if r.status == 401:
      print('Error: invalid API token')
    elif r.status == 404:
      print(f"Error: path not found: {path}")
    else:
      print(f"Error: Received HTTP status {r.status} for {path}")
    sys.exit(1)
  if len(r.data) == 0:
    return {}
  return json.loads(r.data)

def rescan(site, path):
  req("raw", {
    "sites": site,
    "path": path,
    "command": "site rescan",
    "timeout": 600
  })

api_token = sys.argv[1]
operation = sys.argv[2]

if operation.startswith("browse-site"):
  site = sys.argv[3]
  path = sys.argv[4]
  items = sys.argv[5].split(",")
  base_list = req(f"path?site={site}&path={path}")
  rescan_base = False
  rescan_list = []
  for item in items:
    for file in base_list:
      if file['name'] == item:
        if file['type'] == "DIR":
          rescan_list.append(f"{path}/{item}")
        elif file['type'] == "LINK":
          rescan_list.append(file['link_target'])
        else:
          rescan_base = True
        break
  if rescan_base:
    rescan_list.append(path)
  if len(rescan_list) == 1:
    print(f"Rescanning {rescan_list[0]} on {site}...")
  else:
    print(f"Rescanning {len(rescan_list)} items on {site}...")
  sys.stdout.flush()
  for rescan_item in rescan_list:
    rescan(site, rescan_item)
  print("Rescan complete!")
else:
  print(f"Unsupported operation: {operation}")
  sys.exit(1)
