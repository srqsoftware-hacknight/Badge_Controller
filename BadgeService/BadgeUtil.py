
# BadgeUtil - handy class utils for parsing ID from URL, and doing GPIO on Raspberry PI

import httplib
import os


import urllib
#try:
#        from urllib.parse import urlparse
#except ImportError:
#        from urlparse import urlparse
#        from urlparse
import urlparse

# Authorization Service (Badge_Web java web service)
# Badge Service (bsvc) calls Auth Service.
AUTH_HOST   = 'localhost'
#AUTH_PORT   = 80
AUTH_PORT   = 8080
AUTH_URL    = 'http://'+AUTH_HOST+':'+str(AUTH_PORT)+'/device/check?%s'



# Badge Service: to authorize for RFID; open door with GPIO
# RFID Reader calls this service
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
            isAuth = (dat.count('ACCESS')>0)
            return isAuth
        except Exception as e:
            print "error:",os.strerror(e.errno), " exception:",e
            raise Exception('isBadgeAuthorized: auth_problem: %s' % url)

    def parseBadgeId(self,path):
        parsed = urlparse.urlparse(path)
        print "parsed=",parsed
        thequery = urlparse.urlparse(path).query
        thepath = urlparse.urlparse(path).path
        print "thequery=",thequery
        print "thepath=",thepath
        query_dict = urlparse.parse_qs(thequery.encode('ASCII'))
        print "query_dict=",query_dict
        badge_id = query_dict['badge_id'][0]

        #qs = thequery
        #device_id = item.find('device_id').text
        #parsed = urlparse(path)
        #print "parsed=",parsed
        #qs = urllib.parse.parse_qs(parsed.query)
        #qs = urlparse.parse_qs(parsed.query)

        #print "qs=",qs
        #ids = qs.get('id','*')
        #print "ids=",ids
        #if len(ids) > 0:
        #    badge_id = ids[0]
        #else:
        #    badge_id = '*'
        print "parseBadgeId: badge_id=", badge_id
        return badge_id

# pulse level '0' to gpio #1 to press 'exit' button to unlock door.
    def openDoor(self):
        print "OPEN DOOR"
        os.system('/usr/local/bin/gpio mode 1 output')
        os.system('/usr/local/bin/gpio write 1 0; sleep 1; /usr/local/bin/gpio write 1 1')

# pulse level '0' to gpio #1 to press 'exit' button to unlock door.
    def callButton(self):
        print "CALL BUTTON PRESSED - send email or text"
        # TODO email or text Eric's phone


