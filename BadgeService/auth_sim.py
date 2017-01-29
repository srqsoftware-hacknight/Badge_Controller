#!/usr/bin/env python

# AuthService Simulator (only for testing BadgeService)
#  - implement auth API end-point
#  - HACK: return ACCEPT if badge starts with '1', else return DENY

import time
from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import BadgeUtil

class MyHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        try:
            bid = bu.parseBadgeId(self.path)
            print "---------"
            print "do auth bid=",bid
            auth = (bid[0] == '1')  # FIXME HACK
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            if auth:
                self.wfile.write('STATUS=ACCESS')
            else:
                self.wfile.write('STATUS=DENY')
        except IOError:
            self.send_error(404, 'error on badge: %s' % self.path)
        except Exception as e:
            self.send_error(400, 'auth problem: %s' % e)

def main():
    try:
        server = HTTPServer((BadgeUtil.AUTH_HOST,BadgeUtil.AUTH_PORT), MyHandler)
        print time.asctime(), "start auth_sim server - %s:%s" % (BadgeUtil.AUTH_HOST,BadgeUtil.AUTH_PORT)
        server.serve_forever()
    except KeyboardInterrupt:
        print "interrupt, shutting down"
        server.socket.close()

if __name__ == '__main__':
    bu = BadgeUtil.BadgeUtil()
    main()

