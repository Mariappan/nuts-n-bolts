all:
	gcc -o socket_utils.o -c socket_utils.c
	gcc -o tcp_server.o -c tcp_server.c
	gcc -o tcp_client.o -c tcp_client.c
	gcc -o tcp_server tcp_server.o socket_utils.o
	gcc -o tcp_client tcp_client.o socket_utils.o
	gcc -o udp_server.o -c udp_server.c
	gcc -o udp_client.o -c udp_client.c
	gcc -o udp_server udp_server.o socket_utils.o
	gcc -o udp_client udp_client.o socket_utils.o
	gcc -o tcp_echo_server.o -c tcp_echo_server.c
	gcc -o tcp_echo_client.o -c tcp_echo_client.c
	gcc -o tcp_echo_server tcp_echo_server.o socket_utils.o
	gcc -o tcp_echo_client tcp_echo_client.o socket_utils.o
	gcc -o udp_echo_server.o -c udp_echo_server.c
	gcc -o udp_echo_client.o -c udp_echo_client.c
	gcc -o udp_echo_server udp_echo_server.o socket_utils.o
	gcc -o udp_echo_client udp_echo_client.o socket_utils.o


clean:
	rm -rf *.o tags
	rm -rf tcp_server tcp_client udp_server udp_client
	rm -rf tcp_echo_server tcp_echo_client udp_echo_server udp_echo_client

