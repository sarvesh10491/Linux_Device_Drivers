#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "config_ioctl.h"

#define CHAR_DEVICE "/dev/config_ioctl"

int fd;


void get_config(){
	config_dev_t cfg;

	if (ioctl(fd, GET_CONFIG_VAR, &cfg) == -1){
		perror("user config ioctl get");
	}
	else{
        printf("\nGetting config data..\n");
		printf("madeup_dev_id     : %d\n", cfg.madeup_dev_id);
		printf("some_config       : %d\n", cfg.some_config);
		printf("another_config    : %d\n", cfg.another_config);
	}
}

void clr_config()
{
    printf("\nClearing config data..\n");
	if (ioctl(fd, CLR_CONFIG_VAR) == -1){
		perror("user config ioctl clr");
	}
}

void set_config(){
	int usrip;
	config_dev_t cfg;

	printf("Enter madeup_dev_id : ");
	scanf("%d", &usrip);
	getchar();
	cfg.madeup_dev_id = usrip;

	printf("Enter some_config : ");
	scanf("%d", &usrip);
	getchar();
	cfg.some_config = usrip;

	printf("Enter another_config: ");
	scanf("%d", &usrip);
	getchar();
	cfg.another_config = usrip;

    printf("\nSetting config data..\n");
	if (ioctl(fd, SET_CONFIG_VAR, &cfg) == -1){
		perror("user config ioctl set");
	}
}


int main(int argc, char *argv[])
{
    enum{get, clr, set} operation;

    printf("Starting device test code example.\n");
    fd = open(CHAR_DEVICE, O_RDWR);             // Open the device with read/write access
    if (fd < 0){
        perror("Failed to open the device...");
        return errno;
    }
   

	if (argc == 1){
		operation = get;
	}
	else if (argc == 2){
		if (strcmp(argv[1], "-g") == 0){
			operation = get;
		}
		else if (strcmp(argv[1], "-c") == 0){
			operation = clr;
		}
		else if (strcmp(argv[1], "-s") == 0){
			operation = set;
		}
		else{
			fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
			return 1;
		}
	}
	else{
		fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
		return 1;
	}


	switch (operation){
		case get:
			get_config();
			break;
		case clr:
			clr_config();
			break;
		case set:
			set_config();
			break;
		default:
			break;
	}

	close(fd);

	return 0;
}