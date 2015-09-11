all:
	gcc server.c -Wall -o server
	gcc client.c -Wall -o client
	gcc -DSOCK_UNIX server.c -Wall -o server_unix
	gcc -DSOCK_UNIX client.c -Wall -o client_unix

clean:
	rm -rf server client server_unix client_unix
