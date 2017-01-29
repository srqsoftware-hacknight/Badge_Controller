#!/usr/bin/env python

# Badge Reader Simulator (only for testing BadgeService)
#  - implement badge API end-point
#  - Assume HACK: return ACCEPT if badge starts with '1', else return DENY

import urllib

import BadgeUtil

def sendRequest(badge_id):
    params = urllib.urlencode({'id':badge_id})
    url = (BadgeUtil.BADGE_URL) % params
    print "---------"
    print "url:",url
    f = urllib.urlopen(url)
    ret = f.read()
    print "ret:",ret

sendRequest('1234') # start with '1', should pass with ACCEPT
sendRequest('234') # start with '2', should fail with DENY

