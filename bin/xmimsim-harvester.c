#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "xmi_random_dev.h"
#include <sys/resource.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>


//#define LOCKFILE "/var/run/xmi_harvester.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


struct harvester {
	char fifoslave[100];
	int nseeds;
};


void *harvest_thread(void *sh) {
	struct harvester *shl = (struct harvester *) sh;
	unsigned long int *seeds;
	int fifofd2;
	int i;
	
	if ((fifofd2 = open(shl->fifoslave,O_WRONLY)) == -1) {
		syslog(LOG_ERR,"Could not open named link %s: %s",shl->fifoslave,strerror(errno));
		exit(1);
	}

	seeds = (unsigned long int *) malloc(sizeof(unsigned long int)*shl->nseeds);
	for (i = 0 ; i < shl->nseeds ; i++) {
		if (xmi_get_random_number_dev(seeds+i) != 1) {
			syslog(LOG_ERR,"xmi_get_random_number error");
			exit(1);
		}
#ifdef DEBUG
		syslog(LOG_INFO,"Daemon -> seeds: %lu",seeds[i]);
#endif
	}
	if (write(fifofd2,seeds,sizeof(unsigned long int)*shl->nseeds) != sizeof(unsigned long int)*shl->nseeds) {
		syslog(LOG_ERR,"Error writing to %s: %s",shl->fifoslave,strerror(errno));
		exit(1);
	}
	//cleanup
	free(seeds);
	close(fifofd2);
	free(sh);

	return NULL;
}


void daemonize(const char *cmd) {
	int i , fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	umask(0);

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		fprintf(stderr,"%s: can't get file limit\n",cmd);
		exit(1);	
	}

	if ((pid = fork()) < 0) {
		fprintf(stderr,"%s: can't fork\n",cmd);
		exit(1);	
	}
	else if (pid != 0)
		exit(0);
	setsid();

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		fprintf(stderr,"%s: can't ignore SIGHUP\n",cmd);
		exit(1);	
	}
	if ((pid = fork()) < 0) {
		fprintf(stderr,"%s: can't fork\n",cmd);
		exit(1);	
	}
	else if (pid != 0)
		exit(0);

	if (chdir("/") < 0) {		
		fprintf(stderr,"%s: can't change directory to /\n",cmd);
		exit(1);	
	}

	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i=0 ; i < rl.rlim_max ; i++) {
		close(i);
	}

	fd0 = open("/dev/null",O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	openlog(cmd, LOG_CONS | LOG_PID, LOG_DAEMON);
	if (fd0 != 0  || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR,"unexpected file descriptors %d %d %d",fd0,fd1,fd2);
		exit(1);
	}

	setlogmask(LOG_UPTO(LOG_DEBUG));

} 


int lockfile(int fd) {
	struct flock fl = {.l_type = F_WRLCK, .l_start = 0, .l_whence = SEEK_SET, .l_len = 0};

	return fcntl(fd,F_SETLK, &fl);		
}



int already_running(void) {
	int fd;
	char buf[16];

	fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
	if (fd < 0) {
		syslog(LOG_ERR,"can't open %s: %s",LOCKFILE,strerror(errno));
		exit(1);
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			close(fd);
			return 1;
		}
		syslog(LOG_ERR, "can't lock %s: %s",LOCKFILE, strerror(errno));
		exit(1);
	}
	ftruncate(fd,0);
	sprintf(buf,"%ld",(long) getpid());
	write(fd,buf,strlen(buf)+1);
	return 0;
}

void sigterm(int signo) {
	syslog(LOG_INFO, "got SIGTERM; exiting");
	if (xmi_end_random_acquisition_dev() != 1) {
		syslog(LOG_ERR,"xmi_end_random_acquisition_dev error");
		exit(1);
	}
	unlink(LOCKFILE);
	unlink(FIFOMASTER);
	exit(0);
}

void sighup(int signo) {
	syslog(LOG_INFO, "got SIGHUP; doing nothing really");
}


int main (int argc, char *argv[]) {
	char *cmd;
	struct sigaction sa;
	int fifofd,fifofd2;
	long int pidslave[3];
//	char fifoslave[512];
	int i;
	struct harvester *sh;
	pthread_t *ht;
	pthread_attr_t ha;

	if ((cmd = strrchr(argv[0], '/')) == NULL)
		cmd = argv[0];
	else
		cmd++;

	daemonize(cmd);

	if (already_running()) {
		syslog(LOG_ERR, "daemon already running");
		exit(1);
	}

	sa.sa_handler = sigterm;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGTERM);
	sa.sa_flags=0;
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		syslog(LOG_ERR, "can't catch SIGTERM: %s", strerror(errno));
		exit(1);
	}

	sa.sa_handler = sighup;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sa.sa_flags=0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		syslog(LOG_ERR, "can't catch SIGHUP: %s", strerror(errno));
		exit(1);
	}


	if (xmi_start_random_acquisition_dev() != 1) {
		syslog(LOG_ERR,"xmi_start_random_acquisition_dev error");
		exit(1);
	}
	

	syslog(LOG_INFO,"daemon running succesfully");

	pthread_attr_init(&ha);
	pthread_attr_setdetachstate(&ha,PTHREAD_CREATE_DETACHED);


	while (1) {
		//create fifo
		if (mkfifo(FIFOMASTER, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1) {
			syslog(LOG_ERR,"Could not create named link" FIFOMASTER ": %s",strerror(errno));
			exit(1);
		}
		if ((fifofd = open(FIFOMASTER,O_RDONLY)) == -1) {
			syslog(LOG_ERR,"Could not open named link" FIFOMASTER ": %s",strerror(errno));
			exit(1);
		}
	
		//read pid from slave
		if (read(fifofd,pidslave,3*sizeof(long int)) != 3*sizeof(long int)) {
			syslog(LOG_ERR,"Error reading from " FIFOMASTER ": %s",strerror(errno));
			exit(1);
		}				
		close(fifofd);
		unlink(FIFOMASTER);
#ifdef DEBUG
		syslog(LOG_INFO,"Daemon -> pid: %li  counter: %li number: %li",pidslave[0],pidslave[1],pidslave[2]);
#endif
		//allocate necessary variables
		sh = (struct harvester *) malloc(sizeof(struct harvester));
		ht = (pthread_t *) malloc(sizeof(pthread_t));
		
		//create slave fifo
		sprintf(sh->fifoslave, FIFOSLAVE "%li.%li",pidslave[0],pidslave[1]);
		sh->nseeds = pidslave[2];

		//start thread
		pthread_create(ht,&ha,harvest_thread,sh);




	}


	return 0;
}

