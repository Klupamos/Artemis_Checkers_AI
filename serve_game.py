#!/usr/bin/env python
# serve_game.py ~ run a game of checkers over the network

import os, sys, getopt
import select, socket
import threading
import re

def main() :
    # Optional options
    optlist, cmd = getopt.getopt(
        sys.argv[1:], "", [ "port=" ],
    )
    opts = dict(optlist)
    
    # Option defaults
    if not opts.has_key("--port") :
        opts["--port"] = 9000
    
    opts["port"] = int( opts["--port"] )
    
    print "Listening on port %s" % opts["port"]
    
    # Bind to localhost for now
    global server
    server = socket.socket()
    server.bind(("", opts["port"]))
    server.listen(5)
    
    global msg
    msg = {}
    
    # Main server loop turn on
    while True :
        sock, client = server.accept() # we get signal
        # Don't block the sock
        sock.setblocking(0)
        
        # Add the client to the connection queue
        threading.Thread(
            None, handle_client, "handle_client", [ sock, client ]
        ).start()

def handle_client(sock, client) :
    global msg, turn
    role = ""
    if not msg.has_key("black") :
        role = "black"
    elif not msg.has_key("white") :
        role = "white"
    else :
        role = "spectator%s" % str( len(msg) - 2 )
    msg[role] = None
    sock.send("%s\n" % role)
    print "Connection from %s (%s)" % ( client[0], role )
    
    # Wait until a spectator connects and says "play "
    import time
    
    while msg[role] == None :
        if re.match("^spectator", role) :
            rlist, wlist, xlist = select.select(
                [ sock ], [ ], [ ], 0.1,
            )
            if len(rlist) == 0 : continue
            line = ""
            while True :
                buf = sock.recv(1)
                if buf == "\r" or buf == "\n" : break
                line += buf
            if re.match(r"^play\b", line) :
                for k in msg.keys() :
                    msg[k] = "wwwwwwwwwwww________bbbbbbbbbbbb"
        time.sleep(0.1)
    turn = "black"
    while msg.has_key(role) :
        if msg[role] != None :
            sock.send("%s\n" % msg[role])
            if msg[role] == "invalid" :
                break
            if re.match("\w+ wins", msg[role]) :
                break
            if re.match("\w+ declared", msg[role]) :
                break
            msg[role] = None
        rlist, wlist, xlist = select.select(
            [ sock ], [ ], [ ], 0.1,
        )
        if len(rlist) == 0 : continue
        if role == turn :	# will only read from black or white
            line = ""
            while True :
                buf = sock.recv(1)
                if len(buf) == 0 :
                    print "%s disconnected" % role
                    msg.pop(role)
                    line = ""
                    break
                if buf == "\r" or buf == "\n" : break
                line += buf
            if len(line) <= 2 :
                continue
            print "%s says: %s" % (role, line)
            if line == "invalid" :
                for k in [ k for k in msg.keys() if k != role ] :
                    msg[k] = line
                if role == "black" or role == "white" :	#always true
                    break
            if re.match("\w+ wins", line) :
                winner = re.match("(\w+) wins", line).group(1).lower()
                if winner == role :
                    line = "%s declared itself the winner" % role
                for k in [ k for k in msg.keys() if k != role ] :
                    msg[k] = line
                if role == "black" or role == "white" :
                    break
            for k in [ k for k in msg.keys() if k != role ] :
                msg[k] = line
            if turn == "black" :
                turn = "white"
            else :
                turn = "black"
    if msg.has_key(role) :
        msg.pop(role)
        print "Disconnected: %s" % role
 
if __name__ == "__main__" :
    import traceback
    try :
        main()
    except KeyboardInterrupt :
        # ctrl-c'd
        print "Interrupt from keyboard"
    else :
        # something blew up
        traceback.print_exc()
    
    # Then shut down the server socket
    global server
    server.close()
    
    print "Exiting... ",
    sys.exit(0)

