#!/bin/sh
curl -L \
    -H "Accept: application/vnd.github+json" \
    -H "Authorization: Bearer ${GITHUB_TOKEN}" \
    -H "X-GitHub-Api-Version: 2022-11-28" \
    https://api.github.com/repos/${GITHUB_REPOSITORY}/releases |\
python3 -c "
import json, sys, re
release = [rel for rel in filter(lambda rel: rel['published_at'] and not rel['prerelease'], json.load(sys.stdin))][0]
version = re.sub('^[vV]', '', release['name'])
url = [asset for asset in filter(lambda asset: re.search('\.[zZ][iI][pP]\$', asset['name']), release['assets'])][0]['browser_download_url']
print('version: %s'%(version), file=sys.stderr)
print('export const Version = {')
print('  text: \"%s\",'%(version))
print('  package: \"%s\"'%(url))
print('}')
" > "${OUTPUT_FILE}"
