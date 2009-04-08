#include "server.h"
#include "network.h"
#include "globals.h"
#include "utils.h"

#define MAX_LEN 1024
#define BUF_LEN 64

extern int mode;

int serialize_player(DPlaya *player, char *buf, size_t buflen);
int unserialize_player(char *buf, DPlaya *player);

void *start_udp_server(void *ptr) {
	int sd, port = 8000, n, player_loc;
	socklen_t client_len;
	char udpbuf[MAX_LEN];
	struct sockaddr_in udpserver, udpclient;
	DPlaya p1;
	struct sockaddr_in clientIP[8]; /* TODO: remove the magic number of 8 evertwhere */

	printf("starting udp server..\n");
	fflush(stdout);

	if((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Socket Creation");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&udpserver, sizeof(udpserver));
	udpserver.sin_family = PF_INET;
	udpserver.sin_port = htons(port);
	udpserver.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sd, (struct sockaddr *)&udpserver, sizeof(udpserver)) == -1) {
		perror("Binding Socket");
		exit(EXIT_FAILURE);
	}

	for(;;) {
		client_len = sizeof(udpclient);
		if((n = recvfrom(sd, udpbuf, 2, 0, (struct sockaddr *)&udpclient, &client_len)) <= 0) {
			if(n == 0) {
				fprintf(stderr,"Connection closed on socket: %d\n", sd);
				continue;
			} else {
				perror("Recv From");
				exit(EXIT_FAILURE);
			}
		}
		else {
			int i;
			for (i = 0; i < 8; i++) {
				if (clientIP[i].sin_addr.s_addr == udpclient.sin_addr.s_addr) {
					i = -1;
					break;
				}
			}
			if (i != -1) {
				for (i = 0; i < 8; i++) {
					if (clientIP[i].sin_addr.s_addr == 0) {
						printf("assigned client\n");
						clientIP[i] = udpclient;
						break;
					}
				}
			}
		}

		printf("%s\n", udpbuf);

		if((player_loc = get_player_loc(udpclient.sin_addr.s_addr)) >= 0 && player_loc < MAX_PLAYERS) {
			receive_packet(udpbuf[0], player_loc);
		}

		p1.setX(1);
		p1.setY(2);
		p1.setNumBombs(3);
		p1.setName("david");

		/* need to send to all clients.. */
		for (int i = 0; i < 8; i++) {
			if (clientIP[i].sin_addr.s_addr != 0) {
				printf("sending to %d\n", i);
				send_udp_player(&p1, sd, clientIP[i]);
			}
		}

		pthread_testcancel();
	}
	return 0;
}

