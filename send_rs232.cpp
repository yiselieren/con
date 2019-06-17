/*************************************
 * Send RS232 sequence, wait for reply
 *************************************
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include <string>

#include "tty.h"
#include "str_utils.h"

using namespace std;

void usage(const char *s)
{
    fprintf(stderr,"Usage:\n\t%s [switches] string tty_device\n\n", s);
    fprintf(stderr,"\t-h[elp]             - Print help message.\n");
    fprintf(stderr,"\t-b[aud] <baud_rate> - Set the baud rate for target connection.\n");
    fprintf(stderr,"\t-r NUMBER           - Read NUMBER of bytes after write. 0 - means read to EOF\n");
    fprintf(stderr,"\t-e CHAR             - Read  after write. Till character CHAR is accepted.\n");
    fprintf(stderr,"\t                      CHAR may be just a character or \\r \\n for CR or LF\n");
    fprintf(stderr,"\t                      or \\xNN for any hexa code\n");
    exit (1);
}

int main(int ac, char *av[])
{
    int     nparams=0;
    int     TargetBaud = 0;
    int     readReq = -1;
    bool    readChar = false;
    uint8_t readTerm = 0;
    char    *send_str = 0;
    char    *tty_name = 0;


    /* Command line parsing. */
    for (int i=1; i<ac; i++)
    {
        switch (*av[i])
        {
        case '-':
            ++av[i];
            if (!strcmp(av[i], "b")  ||  !strcmp(av[i], "baud"))
            {
                if (++i >= ac)
                {
                    fprintf(stderr,"After switch \"%s\" baud rate is expected.\n",av[--i]);
                    fprintf(stderr,"\nType \"%s -h\" for help.\n", av[0]);
                    return 1;
                }
                char *end;
                TargetBaud = (int)strtol(av[i], &end, 0);
                if (*end)
                {
                    fprintf(stderr,"Invalid baud rate: \"%s\" -- ?\n", end);
                    fprintf(stderr,"\nType \"%s -h\" for help.\n", av[0]);
                    return 1;
                }
            }
            else if (!strcmp(av[i], "r")  ||  !strcmp(av[i], "read"))
            {
                if (++i >= ac)
                {
                    fprintf(stderr,"After switch \"%s\" number of read bytes is expected.\n",av[--i]);
                    fprintf(stderr,"\nType \"%s -h\" for help.\n", av[0]);
                    return 1;
                }
                char *end;
                readReq = (int)strtol(av[i], &end, 0);
                if (*end)
                {
                    fprintf(stderr,"Invalid number of read bytes: \"%s\" -- ?\n", end);
                    fprintf(stderr,"\nType \"%s -h\" for help.\n", av[0]);
                    return 1;
                }
            }
            else if (!strcmp(av[i], "e")  ||  !strcmp(av[i], "end"))
            {
                if (++i >= ac)
                {
                    fprintf(stderr,"After switch \"%s\" termination character is expected.\n",av[--i]);
                    fprintf(stderr,"\nType \"%s -h\" for help.\n", av[0]);
                    return 1;
                }
                readChar = true;
                string t = str::unescape(av[i]);
                if (t.length() != 1)
                {
                    fprintf(stderr,"After switch \"%s\" only one termination character is expected.\n",av[--i]);
                    fprintf(stderr,"\nType \"%s -h\" for help.\n", av[0]);
                    return 1;
                }
                readTerm = t[0];
            }
            else if (!strcmp(av[i], "h")  ||  !strcmp(av[i], "help"))
            {
                usage(av[0]);
            }
            else
            {
                fprintf(stderr,"Invalid switch \"%s\".\n", av[i]);
                fprintf(stderr,"Type \"%s -h\" for help.\n", av[0]);
                return 1;
            }
            break;

        case '?':
            usage(av[0]);
            break;

        default:
            switch (nparams++)
            {
            case 0:
                send_str = av[i];
                break;
            case 1:
                tty_name = av[i];
                break;
            default:
                fprintf(stderr,"Invalid number of parameters.\n");
                fprintf(stderr,"Type \"%s -h\" for help.\n", av[0]);
                return 1;
            }
        }
    }

    if (nparams != 2)
    {
        fprintf(stderr,"Invalid number of parameters.\n");
        fprintf(stderr,"Type \"%s -h\" for help.\n", av[0]);
        return 1;
    }

    // Open tty connection
    Tty tty;
    int tty_fd = -1;
    if ((tty_fd = tty.open(tty_name, TargetBaud)) < 0)
    {
        fprintf(stderr,"Can't open %s: %s\n", tty_name, strerror(errno));
        return 1;
    }

    // Write
    std::string s = str::unescape(send_str);

    //// DEBUG ++++
    //for (unsigned i = 0; i<s.size(); i++)
    //    printf("Write[%d] - 0x%02x (%c)\n", i, s[i]&0xff, (s[i] >= ' ' && s[i] <= '~') ? s[i] : '?');
    //// DEBUG ----

    unsigned rc = write(tty_fd, s.c_str(), s.length());
    if (rc != s.length())
    {
        fprintf(stderr, "%s write error: %s\n", tty_name, strerror(errno));
        return 1;
    }

    // Read till termination character
    if (readChar)
    {
        uint8_t c;
        string  rc;

        while (true)
        {
            int rd = read(tty_fd, &c, 1);
            if (rd < 0)
            {
                if (errno == EAGAIN)
                    continue;
                fprintf(stderr, "%s read error: %s\n", tty_name, strerror(errno));
                return 1;
            }
            if (rd == 0)
                break;
            if (c == readTerm)
                break;
            rc += c;
        }

        printf("%s\n", rc.c_str());
    }

    // Read specified number of chars
    else if (readReq != -1)
    {
#if 0
        const int   RBUF = 256;
        static char rbuf[RBUF+1];
        int         rcnt = 0;

        while (readReq == 0 || rcnt < readReq)
        {
            int toRead = readReq ? std::min(RBUF, readReq) : RBUF;
            int rd = read(tty_fd, rbuf, toRead);
            if (rd < 0)
            {
                if (errno == EAGAIN)
                    continue;
                fprintf(stderr, "%s read error: %s\n", tty_name, strerror(errno));
                return 1;
            }
            if (rd == 0)
                break;
            rcnt += rd;

            rbuf[rd] = 0;
            printf("Read %d bytes: \"%s\"\n", rd, rbuf);
        }
#else
        while (true)
        {
            uint8_t c;
            int rd = read(tty_fd, &c, 1);
            if (rd < 0)
            {
                if (errno == EAGAIN)
                    continue;
                fprintf(stderr, "%s read error: %s\n", tty_name, strerror(errno));
                return 1;
            }
            if (rd == 0)
                break;
            printf("Read 0x%02x (%c)\n", c&0xff, (c >= ' ' && c <= '~') ? c : '?');
        }
#endif
    }

    return 0;
}
