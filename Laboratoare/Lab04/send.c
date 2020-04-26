#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000
#define FRAMESIZE 1404

int main(int argc, char *argv[])
{
	msg t;
	int i, res;
	int speed = 100;
	int delay = 100;

	unsigned int bdp = atoi(argv[1]);
	int win_size = bdp / (FRAMESIZE * 7);
	
	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);
	printf("[SENDER]: SPEED=%d\n", speed);
    printf("[SENDER]: DELAY=%d\n", delay);
	printf("[SENDER]: BDP=%d\n", bdp);
	printf("[SENDER]: WIN_SIZE=%d\n", win_size);

	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */

	for (i = 0; i < win_size + 10; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		
	}

	for (i = 0; i < COUNT - win_size; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}
	
	for (i = 0; i < win_size + 10; i++) {
		printf("PIZZA \n");
		
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}



	printf("[SENDER] Job done, all sent.\n");
		
	return 0;
}
