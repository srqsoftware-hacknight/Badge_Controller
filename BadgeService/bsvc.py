#!/usr/bin/env python

# BadgeService
#  - implement badge API end-point
#  - call to Auth Service (java web app)
#  - toggle gpio pin with wiring pi to open door

import os,time
import threading, BaseHTTPServer, SocketServer
import BadgeUtil

class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):

    def do_GET(self):
        try:
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            idval = bu.parseBadgeId(self.path)
            print "---------"
            print "idval:",idval
            auth = bu.isBadgeAuthorized(idval)
            if auth:
                self.wfile.write('ACCEPT:'+idval)
                bu.openDoor()
            else:
                self.wfile.write('DENY:'+idval)
        except IOError:
            self.send_error(404, 'File not found: %s' % self.path)
        except Exception as e:
            self.send_error(400, 'auth problem: %s' % e)

class ThreadedHTTPServer(SocketServer.ThreadingMixIn, BaseHTTPServer.HTTPServer):
    ''' handle requests in separate threads '''

def main():
    try:
        server = ThreadedHTTPServer((BadgeUtil.BADGE_HOST,BadgeUtil.BADGE_PORT), MyHandler)
        print time.asctime(), "start badge server - %s:%s" % (BadgeUtil.BADGE_HOST,BadgeUtil.BADGE_PORT)
        server.serve_forever()
    except KeyboardInterrupt:
        print "interrupt, shutting down"
        server.socket.close()

if __name__ == '__main__':
    bu = BadgeUtil.BadgeUtil()
    main()


