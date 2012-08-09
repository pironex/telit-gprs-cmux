/*
 * main.c
 *
 *  Created on: Jul 20, 2012
 *      Author: Björn Krombholz <b.krombholz@pironex.de>
 *   Copyright: pironex GmbH (2012)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 * Enable n-gsm or CMUX for Telit GSM/GPRS module.
 * Based on kernel example. Quick and dirty solution, but works.
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/gsmmux.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#define N_GSM0710	21	/* GSM 0710 Mux */
#define DEFAULT_SPEED	B115200
#define SERIAL_PORT	"/dev/ttyO0"

#define DEBUG

#ifdef	DEBUG
#define	debugf(fmt,args...)	printf (fmt ,##args)
#else
#define debugf(fmt,args...)
#endif

int main(int argc, char **argv)
{
	int fd;
	int ldisc = N_GSM0710;
	struct gsm_config c;
	struct termios old_tio;
	struct termios tio;
	//struct termios configuration;

	/* open the serial port connected to the modem */
	fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);

	tcgetattr(fd,&old_tio);
	debugf("iflag: %08x\n", old_tio.c_iflag);
	debugf("oflag: %08x\n", old_tio.c_oflag);
	debugf("cflag: %08x\n", old_tio.c_cflag);
	debugf("lflag: %08x\n", old_tio.c_lflag);
	debugf("ispeed: %08x\n", old_tio.c_ispeed);
	debugf("ospeed: %08x\n", old_tio.c_ospeed);
	ioctl(fd, TIOCSETD, 0);
	memset(&tio,0,sizeof(tio));
	/* configure the serial port : speed, flow control ... */

	tio.c_iflag=0;
	tio.c_oflag=0;
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
	tio.c_lflag=0;
	tio.c_cc[VMIN]=1;
	tio.c_cc[VTIME]=0;
	cfsetospeed(&tio,B115200);            // 115200 baud
	cfsetispeed(&tio,B115200);            // 115200 baud
	tcsetattr(fd,TCSANOW,&tio);

	/* send the AT commands to switch the modem to CMUX mode
	   and check that it's successful (should return OK) */
	char buf[256];
	write(fd, "AT#SELINT=2\r", 12);
	usleep(50000);
	memset(buf, 0, 128);
	read(fd, buf, 256);
	debugf("AT#SELINT=2\nres:%s\n", buf);

	write(fd, "ATE0V1&K3&D2\r", 13);
	usleep(50000);
	memset(buf, 0, 128);
	read(fd, buf, 256);
	debugf("ATE0V1&K3&D2\nres:%s\n", buf);

	write(fd, "AT+IPR=115200\r", 14);
	usleep(50000);
	memset(buf, 0, 128);
	read(fd, buf, 256);
	debugf("AT+IPR=115200\nres:%s\n", buf);

	write(fd, "AT#CMUXMODE=0\r", 14);
	usleep(50000);
	memset(buf, 0, 128);
	read(fd, buf, 256);
	debugf("AT#CMUXMODE=0\nres:%s\n", buf);

	write(fd, "AT+CMUX=0\r", 10);

	/* experience showed that some modems need some time before
	   being able to answer to the first MUX packet so a delay
	   may be needed here in some case */
	sleep(2);
	memset(buf, 0, 128);
	debugf("AT+CMUX=0\nres:%s\n", buf);
	fsync(STDOUT_FILENO);
	fsync(STDERR_FILENO);

	/* use n_gsm line discipline */
	ioctl(fd, TIOCSETD, &ldisc);

	/* get n_gsm configuration */
	ioctl(fd, GSMIOC_GETCONF, &c);
	/* we are initiator and need encoding 0 (basic) */
	c.initiator = 1;
	c.encapsulation = 0;
	/* our modem defaults to a maximum size of 127 bytes */
	c.mru = 127;
	c.mtu = 127;
	/* set the new configuration */
	ioctl(fd, GSMIOC_SETCONF, &c);

	/* and wait for ever to keep the line discipline enabled */
	//ret = system("/etc/init.d/mkcmuxdev.sh");
	//printf("Creating devices: %d\n", ret);
	daemon(0,0);
	pause();

	return 0;
}
