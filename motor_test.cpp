#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <linux/can.h>
#include <linux/can/raw.h>

using namespace std;

#define MOTOR_ID	0x001

int32_t position;
int16_t speed;


// sudo ip link set can0 type can bitrate 1000000
// sudo ifconfig can0 up

int main() {

	int ret;
	int s_can;

    struct sockaddr_can addr;
	struct ifreq ifr;

    struct can_filter rfilter[1];

	// 1. Create socket
	s_can = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(s_can < 0){
		perror("socket PF_CAN failed");
		return 0;
	}

	// 2. Specify can0 device
	strcpy(ifr.ifr_name, "can0");
	ret = ioctl(s_can, SIOCGIFINDEX, &ifr);
	if(ret < 0){
		perror("ioctl failed");
		return 0;
	}

	// 3.Bind the socket to can0
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	ret = bind(s_can, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
	    perror("bind failed");
	    return 1;
	}

	// 4.Receive all frame
	// example only receive ID 1 (0x141)
	// rfilter[0].can_id = 0x141;
	// rfilter[0].can_mask = 0x1FF;
	rfilter[0].can_id = 0x000;
	rfilter[0].can_mask = 0x000;
	setsockopt(s_can, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

	// 5. Send test can frame
	int i;
	struct can_frame frame;

	for(i = 1; i < 6; i++){
		frame.can_id = 0x140 + i;
		frame.can_dlc = 8;
		memset(frame.data, 0, 8);
		frame.data[0] = 0x88;

		// read / write -> no timeout
		write(s_can, &frame, sizeof(frame));

		// -> wait response
		read(s_can, &frame, sizeof(frame));
		//usleep(100000);

		frame.data[0] = 0xA4;
		position = 0;
		speed = 40;
		memcpy(frame.data + 2, &speed, 2);
		memcpy(frame.data + 4, &position, 4);
		write(s_can, &frame, sizeof(frame));
	}

	return 0;
}