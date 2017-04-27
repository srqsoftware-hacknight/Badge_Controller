
# BadgeUtil - handy class utils for parsing ID from URL, and doing GPIO on Raspberry PI

import httplib, urllib, urlparse
import os

# Authorization Service (Badge_Web java web service)
AUTH_HOST   = 'localhost'
AUTH_PORT   = 80
AUTH_URL    = 'http://'+AUTH_HOST+':'+str(AUTH_PORT)+'/device/check?%s'



# Badge Service: to authorize for RFID; open door with GPIO
#BADGE_HOST  = 'localhost'
BADGE_HOST  = ''
BADGE_PORT  = 8081
BADGE_URL   = 'http://'+BADGE_HOST+':'+str(BADGE_PORT)+'/badge/check?%s'
CALL_URL    = 'http://'+BADGE_HOST+':'+str(BADGE_PORT)+'/call'


class BadgeUtil:

# call Auth service to check badge id, return True/False
    def isBadgeAuthorized(self,bid):
        try:
            conn = httplib.HTTPConnection(AUTH_HOST, AUTH_PORT, timeout=5)
            #conn.set_debuglevel(5)
            #print "after conn"
            params = urllib.urlencode({'device_id':0,'badge_id':bid})
            url = AUTH_URL % params
            print "after params, url=", url
            conn.request("GET",url)
            res = conn.getresponse()
            print "after getresponse"
            dat = res.read()
            print "after read:", dat
            isAuth = (dat.count('accept')>0)
            return isAuth
        except Exception as e:
            print "error:",os.strerror(e.errno), " exception:",e
            raise Exception('isBadgeAuthorized: auth_problem: %s' % url)

    def parseBadgeId(self,path):
        prs = urlparse.urlparse(path)
        print "prs=",prs
        qs = urlparse.parse_qs(prs[4])
        print "qs=",qs
        ids = qs.get('id','*')
        if len(ids) > 0:
            badge_id = ids[0]
        else:
            badge_id = '*'
        print "parseBadgeId:badge_id:", badge_id
        return badge_id

# pulse level '0' to gpio #1 to press 'exit' button to unlock door.
    def openDoor(self):
        print "OPEN DOOR"
        os.system('/usr/local/bin/gpio mode 1 output')
        os.system('/usr/local/bin/gpio write 1 0; sleep 1; /usr/local/bin/gpio write 1 1')
