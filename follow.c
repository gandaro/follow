# include <stdio.h>
# include <errno.h>
# include <string.h>
# include <stdlib.h>
# include <stdarg.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/inotify.h>

# define BUFSIZE 256

void errexit(const char* str, ...)
{
        va_list args;

        va_start(args,str);
        vfprintf(stderr,str,args);
        va_end(args);

        if ( errno )
                fprintf(stderr,": %s",strerror(errno));

        fputs("\n",stderr);

        exit(1);
}

int main(int argc, char** argv)
{
        int fd, notify;
        size_t readchars = 1, inotifysize = sizeof(struct inotify_event);
        const char* path = argv[1];
        char buf[BUFSIZE];
        struct inotify_event evt;

        if ( (notify = inotify_init()) == -1 )
                errexit("follow: couldn't initialize inotify");

        if ( argc != 2 )
                errexit("Usage: %s FILENAME",argv[0]);

        if ( -1 == (fd = open(path,O_RDONLY)) )
                errexit("follow: couldn't open file");

        while ( readchars > 0 )
        {
                readchars = read(fd,buf,255);
                write(1,buf,readchars);
        }

        inotify_add_watch(notify,path,IN_MODIFY);

        while ( 1 )
        {
                read(notify,&evt,inotifysize); // Blocks until event

                if ( evt.mask == IN_MODIFY )
                {
                        while (0 < (readchars = read(fd,buf,255)))
                                write(1,buf,readchars);
                }
        }

        return 0;
}
