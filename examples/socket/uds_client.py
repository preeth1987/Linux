#!/usr/bin/env python3
import threading
from io import StringIO
import signal
import socket
import sys
import os
import code


sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

#server_address = './uds'
server_address = '/etc/frr/bgpd_client_sock'
print(sys.stderr, 'connecting to %s' % server_address)
try:
    sock.connect(server_address)
except socket.error:
    print(sys.stderr)
    sys.exit(1)


try:
    # Send data
    message = 'This is the message.  It will be repeated.'
    print(sys.stderr, 'sending "%s"' % message)
    sock.sendall(message.encode('utf-8'))

    amount_received = 0
    amount_expected = len(message)
    
    while amount_received < amount_expected:
        data = sock.recv(16)
        amount_received += len(data)
        print(sys.stderr, 'received "%s"' % data)

finally:
    print(sys.stderr, 'closing socket')
    sock.close()
