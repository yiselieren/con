# con
console program for tty and sockets communication

## PURPOSE
    **Just console program and nothing besides that**

    Note that it runs on Linux only!

The **con** tool is a console program. It can communicate to another terminal
device (like minicom) or to TCP or UNIX domain socket.

In the simplest form
con [-b BAUDRATE] TERMINAL_DEVICE
it is simple minicom replacement without its complexity and UI.


## USAGE

	con [SWITCHES] {tty_device | path_to_local_socket | [host]:port}

The tool can be used in a few different modes:

1. Communicaton program to serial line (like minicom):
       con [-t] [-b BAUDRATE] TERM_DEVICE
   Example:
       con /dev/ttyUSB0
       con -b 115200 /dev/ttyUSB0
       con - 9600 /dev/ttyS0

2. Communicaton program to TCP socket in a client mode:
       con -c ADDR:PORT
   Example:
       con -c www.something.org:80
       con -c 192.168.2.100:8080

3. Communicaton program to TCP socket in a server mode:
   See Note 2
       con -s :PORT
   Example:
       con -s :8080

4. Communicaton program to UNIX socket in a client mode:
       con -c SOCKET_PATH
   Example:
       con -c /tmp/my_named_socket

5. Communicaton program to UNIX socket in a server mode:
   See Note 2
       con -s SOCKET_PATH
   Example:
       con -s /tmp/my_named_socket

SWITCHES may be:
	-h[elp]             - Print help message.
	-e[cho]             - Echo keyboard input locally.
	-q[uit] KEY         - Quit connection key. May be in integer as 0x01 or 001
	                      or in a "control-a", "cntrl/a" or "ctrl/a" form
	                      Default is "cntrl/a".

Switches specific for tty_device:
	-t[erm]             - Work as serial communicaton program. The is a default
	                      mode. Note that "-b" switch assumes "-t"
	-b[aud] <baud_rate> - Set the baud rate for target connection.

Switches specific for socket connection:
	-s[erver]           - Accept connection to socket as server.
	-c[lient]           - Connection to socket as client.


## NOTES

1. For some funny historical reason this directory can't be copied to MS-Windows system.
   Don't event try to do that.

2. Server mode accepts connection only once. I.e. when connection is accepted **con**
   stops to listen for other incoming connections
