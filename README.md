# C-Playground

This is an HTTP server (webserver), that let the front-end (client-side) execute commands on the server's machine.

Usage:
-p : for specifying the port number, default is 3838.
-r : for deactivating lan restriction (let other machines in the same network to connect to the server and execute commands).
-h : for displaying the usage or help message.

server -r -p 2000 || server -p 2000 -r

P.S : compile with gcc.
