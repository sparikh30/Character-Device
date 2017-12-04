#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void main ( )
 {
        int i,fd;
        char ch, write_buf[100], read_buf[100];
        fd = open("/dev/namedpipe", O_RDWR);
        if (fd == -1)
        {
                printf("Error in opening file \n");
                exit(-1);
        }
        printf ("Press r to read from device or w to write the device ");
        scanf ("%c", &ch);

        switch (ch) {
                case 'w':
                        
                        i=0;
			while(1){
				sprintf(write_buf,"By Producer %d : Number: %d\n",getpid(),i++);
 				ssize_t ret=0;
				ret=write(fd, write_buf, sizeof(write_buf));
			
				if ( ret < 0) {
                        		fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
                        		perror("");
                		}
                		else {
                        		printf("Writing: %s", write_buf);
                		}
               			 sleep(1);
			}
                        break;
                case 'r':

			while(1) {
                		// read a line
                		ssize_t ret = read(fd, read_buf, sizeof(read_buf));
                		if( ret > 0) {
                        		printf("Line read: %s\n", read_buf);
                		} else {
                        		fprintf(stderr, "error reading ret=%ld errno=%d perror: ", ret, errno);
                        		perror("");
                        	
                		}
				sleep(1);
        		}	
		    
                        break;
                default:
                        printf("Wrong choice \n");
                        break;
        }
        close(fd);
}
