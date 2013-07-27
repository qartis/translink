#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

#include "translink.h"

#define MAX 1024

enum {
    DAY = 0,
    TIME,
    Z_ISSUED,
    N_ZONES
};

const uint8_t CMD_ERASE[] = {0x1b, 0x63, 0x07};
const uint8_t CMD_READ_RAW[] = {0x1b, 0x6d};
const uint8_t CMD_SET_ZEROS[] = {0x1b, 0x7a, 0x0e, 0x0e};
const uint8_t CMD_SET_DENSITY[] = {0x1b, 0x62, 0x4b};

int edit_state = DAY;
int edit_pos = 0;
char edit_buf[5];
int buf_timeout;

int x_start = 60;
int y_start = 26;
int width = 12;
int height = 20;
int padding = 1;
int z_issued = 1;
int n_zones = 2;
int fd = -1;

int values[] = {
    0,
    275,
    400,
    550
};

void reset_buf(int sig)
{
    buf_timeout = 1;
}

const char *daynames[] = {"SU","MO","TU","WE","TH","FR","SA"};
const char *monthnames[] = {"JA","FE","MR","AP","MA","JN","JL","AU","SE","OC","NO","DE"};

ssize_t safe_read(int fd, uint8_t *buf, size_t count)
{
    size_t readb = 0;
    ssize_t rc;

    if (fd == -1) {
        return -1;
    }

    while (readb < count) {
        rc = read(fd, buf + readb, count - readb);
        if (rc < 0){
            if ((errno != EAGAIN) && (errno != EINTR)) {
                perror("read");
                exit(-1);
            }
            break;
        } else if (rc == 0) {
            break;
        } else {
            readb += rc;
        }
    }

    return readb;
}

ssize_t safe_write(int fd, const uint8_t *buf, size_t count)
{
    size_t wroteb = 0;
    ssize_t rc;

    if (fd == -1) {
        return -1;
    }

    while (wroteb < count) {
        rc = write(fd, buf + wroteb, count - wroteb);
        if (rc < 0) {
            if ((errno != EAGAIN) && 
                (errno != EINTR)) {
                perror("write");
                exit(-1);
            }
        } else if (rc == 0) {
            break;
        } else {
            wroteb += rc;
        }
    }

    printf("wrote %d bytes (%d)\n", count, rc);

    return wroteb;
}

class MyWindow : public Fl_Double_Window {
    Fl_PNG_Image *bg;
    Fl_PNG_Image *font;
    int x_offset;
    struct tm *tm;

public:
    void drawchar(char c){
        fl_push_clip(x_start + x_offset, y_start, width, height);
        if (isalpha(c)){
            font->draw(x_start + x_offset - width * (c - 'A'), y_start);
        } else if (isdigit(c)){
            font->draw(x_start + x_offset - width * (26 + c - '0'), y_start);
        } else if (c == '-'){
            font->draw(x_start + x_offset - width * (26 + 10), y_start);
        } else if (c == ':'){
            font->draw(x_start + x_offset - width * (26 + 10 + 1), y_start);
        } else if (c == '.'){
            font->draw(x_start + x_offset - width * (26 + 10 + 2), y_start);
        }
        fl_pop_clip();
        x_offset += width + padding;
    }

    void draw() {
        char buf[64];

        Fl_Double_Window::draw();
        bg->draw(0,0);
        x_offset = 0;
        sprintf(buf, "%s.%s.", daynames[tm->tm_wday], monthnames[tm->tm_mon]);
        strftime(buf + strlen(buf), sizeof(buf), "%d %I:%M", tm);
        sprintf(buf + strlen(buf), " %c  -%d- %d %d", (tm->tm_hour > 11) ? 'P' : 'A', z_issued, n_zones, values[n_zones]);
        char *str = buf;
        while (*str){
            drawchar(*str);
            str++;
        }

        switch (edit_state){
        case DAY:
            fl_line(x_start,y_start + height,x_start + strlen("FR.MR.30") * (width + padding),y_start + height);
            break;
        case TIME:
            fl_line(x_start + strlen("FR.MR.30 ") * (width + padding), y_start + height,x_start + (strlen("FR.MR.30 12:12 P")) * (width + padding),y_start + height);
            break;
        case Z_ISSUED:
            fl_line(x_start + strlen("FR.MR.30 12:12 P  ") * (width + padding), y_start + height, x_start + (strlen("FR.MR.30 12:12 P  -2-")) * (width + padding),y_start + height);
            break;
        case N_ZONES:
            fl_line(x_start + strlen("FR.MR.30 12:12 P  -2- ") * (width + padding), y_start + height, x_start + (strlen("FR.MR.30 12:12 P  -2- 1")) * (width + padding),y_start + height);
            break;
        }
    }

    MyWindow(int W, int H) : Fl_Double_Window(W,H) {
        bg  = new Fl_PNG_Image("ticket-small.png");
        font = new Fl_PNG_Image("font.png");
        edit_state = DAY;
        time_t now = time(NULL);
        tm = localtime(&now);
        tm->tm_min += 5;
        tm->tm_min -= tm->tm_min%6;
        mktime(tm);
        show();
    }

    int handle(int e) {
        if (e == FL_KEYDOWN){
            if (Fl::event_key() == FL_Left){
                if (edit_state > 0){
                    edit_pos = 0;
                    edit_state--;
                }
            } else if (Fl::event_key() == FL_Right){
                if (edit_state < N_ZONES){
                    edit_pos = 0;
                    edit_state++;
                }
            } else if (Fl::event_key() == FL_Up){
                switch (edit_state){
                case DAY:
                    tm->tm_mday++;
                    break;
                case TIME:
                    tm->tm_min += 6;
                    break;
                case Z_ISSUED:
                    z_issued = min(z_issued+1, 3);
                    break;
                case N_ZONES:
                    n_zones = min(n_zones+1, 3);
                    break;
                }
            } else if (Fl::event_key() == FL_Down){
                switch (edit_state){
                case DAY:
                    tm->tm_mday--;
                    break;
                case TIME:
                    tm->tm_min -= 6;
                    break;
                case Z_ISSUED:
                    z_issued = max(z_issued-1, 1);
                    break;
                case N_ZONES:
                    n_zones = max(n_zones-1, 1);
                    break;
                }
            } else if ((Fl::event_key() < 0x100) && isalnum(Fl::event_key())){
                if (edit_state == DAY){
                    if (edit_pos == 0){
                        switch (Fl::event_key()){
                        case 'm':
                        case 't':
                        case 'w':
                        case 'f':
                        case 'j':
                        case 'a':
                        case 'o':
                        case 'n':
                        case 'd':
                        case 's':
                        case '0'...'9':
                            edit_buf[0] = Fl::event_key();
                            edit_pos++;
                            break;
                        }
                    } else if (edit_pos == 1){
                        edit_buf[1] = Fl::event_key();
                        unsigned i;
                        for(i=0;i<sizeof(daynames)/sizeof(daynames[0]);i++){
                            if (strncasecmp(edit_buf, daynames[i], 2) == 0){
                                tm->tm_mday += i - tm->tm_wday;
                            }
                        }
                        for(i=0;i<sizeof(monthnames)/sizeof(monthnames[0]);i++){
                            if (strncasecmp(edit_buf, monthnames[i], 2) == 0){
                                tm->tm_mon = i;
                            }
                        }
                        if (isdigit(edit_buf[0]) && isdigit(edit_buf[1])){
                            edit_buf[2] = '\0';
                            int day = atoi(edit_buf);
                            if (day > 31){
                                tm->tm_mon++;
                                tm->tm_mday = 0;
                            } else {
                                tm->tm_mday = day;
                            }
                        }
                        edit_pos = 0;
                    }
                } else if (edit_state == TIME){
                    if (Fl::event_key() == 'p'){
                        if (tm->tm_hour < 12){
                            tm->tm_hour += 12;
                        }
                    } else if (Fl::event_key() == 'a'){
                        if (tm->tm_hour > 11){
                            tm->tm_hour -= 12;
                        }
                    } else if (isdigit(Fl::event_key())){
                        edit_buf[edit_pos] = Fl::event_key();
                        edit_pos++;
                        if (edit_pos == 2){
                            edit_buf[2] = '\0';
                            int hour = atoi(edit_buf);
                            hour = min(hour, 23);
                            tm->tm_hour = hour;
                        } else if (edit_pos == 4){
                            edit_buf[4] = '\0';
                            int min = atoi(edit_buf+2);
                            min = min(min, 59);
                            min += 5;
                            min -= min%6;
                            tm->tm_min = min;
                            edit_pos = 0;
                        }
                    }
                } else if (edit_state == Z_ISSUED){
                    if (Fl::event_key() == '1'){
                        z_issued = 1;
                    } else if (Fl::event_key() == '2'){
                        z_issued = 2;
                    } else if (Fl::event_key() == '3'){
                        z_issued = 3;
                    }
                } else if (edit_state == N_ZONES){
                    if (Fl::event_key() == '1'){
                        n_zones = 1;
                    } else if (Fl::event_key() == '2'){
                        n_zones = 2;
                    } else if (Fl::event_key() == '3'){
                        n_zones = 3;
                    }
                }
            } else if (Fl::event_key() == ';' && edit_state == TIME){
                if (edit_pos == 1){
                    tm->tm_hour = edit_buf[0] - '0';
                }
                edit_pos = 2;
            } else if (Fl::event_key() == FL_Enter) {
                char *bits = encode(*tm, z_issued, n_zones);
                bits = (char *)"0000000101110010000100000101111111111110100101011101000000000000000000000011111111111111110101100000000000001010010100000001000000";
                uint8_t bytes[MAX];
                int len = reformat(bits, bytes);
                int i;
                for (i=0;i<len;i++){
                    printf("%02x ", bytes[i]);
                }
                printf("\n");
                safe_write(fd, bytes, len);
            } else if (Fl::event_key() == FL_BackSpace) {
                safe_write(fd, CMD_ERASE, sizeof(CMD_ERASE));
            } else if (Fl::event_key() == ' '){
                safe_write(fd, CMD_READ_RAW, sizeof(CMD_READ_RAW));
            }
        }
        mktime(tm);
        redraw();
        return Fl_Double_Window::handle(e);
    }
};


void msr505_read(int state, void *userp)
{
    static uint8_t buf[MAX];
    static int len = 0;
    int i;
    ssize_t rc;
    
    printf("hey\n");

    if (buf_timeout) {
        len = 0;
        buf_timeout = 0;
    }

    for(;;){
        rc = safe_read(fd, buf + len, sizeof(buf) - len);
        printf("safe_read() = %d\n", rc);
        len += rc;

        while (buf[0] != 0x1b && len > 0) {
            memmove(buf, buf+1, len-1);
            len--;
        }

        if (rc == 0) {
            break;
        }


        while (len >= 2) {
            if (buf[0] != 0x1b) {
                printf("error, malformed response\n");
                len = 0;
                continue;
            }
            if (buf[1] == '0'){
                printf("OK\n");
                memmove(buf, buf + 2, len - 2);
                len -= 2;
            } else if (buf[1] == 0x73){
                if (len >= 5){
                    int t1_len = buf[4];
                    if (t1_len > 20) {
                        printf("unlikely t1 len %d, aborting\n", t1_len);
            break;
                        len = 0;
                        continue;
                    }
                    if (len >= 5 + t1_len + 3){
                        if ((buf[5 + t1_len + 0] != 0x1b) || 
                            (buf[5 + t1_len + 1] != 0x02)) {
                            printf("malformed response");
                            if ((buf[5 + t1_len + (-1)] == 0x1b) &&
                                (buf[5 + t1_len + 0] == 0x02)) {
                                printf(", attempting to fix\n");
                                t1_len--;
                            } else {
                                printf(", cannot repair, aborting\n");
                                len = 0;
                                continue;
                            }
                        }
                        int t2_len = buf[5 + t1_len + 2];
                        if (t2_len > 20) {
                            printf("unlikely t2 len %d, aborting\n", t2_len);
            break;
                            len = 0;
                            continue;
                        }
                        if (len >= 5 + t1_len + 3 + t2_len + 3){
                            if ((buf[5 + t1_len + 3 + t2_len + 0] != 0x1b) || 
                                (buf[5 + t1_len + 3 + t2_len + 1] != 0x03)) {
                                printf("malformed response");
                                if ((buf[5 + t1_len + 3 + t2_len + (-1)] == 0x1b) &&
                                    (buf[5 + t1_len + 3 + t2_len + 0] == 0x03)) {
                                    printf(", attempting to fix\n");
                                    t2_len--;
                                } else {
                                    printf(", cannot repair, aborting\n");
                                    len = 0;
                                    continue;
                                }
                            }
                            int t3_len = buf[5 + t1_len + 3 + t2_len + 2];
                            if (t3_len > 20) {
                                printf("unlikely t3 len %d, aborting\n", t3_len);
                                len = 0;
                                continue;
                            }
                            if (len >= 5 + t1_len + 3 + t2_len + 3 + t3_len + 2){
                                char bits[MAX];
                                for(i=0;i<t2_len;i++){
                                    buf[5 + t1_len + 3 + i] = (buf[5 + t1_len + 3 + i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;
                                }
                                unformat(buf + 5 + t1_len + 3, t2_len, bits);
                                if (decode(bits) == 0){
                                    printf("bits: %s\n", bits);
                                    printf("ticket number: ");
                                    int num;
                                    if (scanf("%d", &num) == 1) {
                                        fprintf(stderr, "%d %s\n", num, bits);
                                    } else {
                                        /* chomp the line */
                                        if (fgets(bits, sizeof(bits), stdin)) {}
                                    }
                                }
                                int processed = 5 + t1_len + 3 + t2_len + 3 + t3_len + 2;
                                memmove(buf, buf + processed, len - processed);
                                len -= processed;
                                continue;
                            }
                        }
                    }
                }
                break;
            } else {
                printf("Error: %c\n", isprint(buf[1])?buf[1]:'X');
                len = 0;
            }
        }
    }
}

int main(int argc, char **argv)
{
    const char *device = "/dev/ttyUSB0";
    if (argc == 2){
        device = argv[1];
    }

    fl_register_images();
    Fl::visual(FL_RGB);
    MyWindow win(500,312);

    signal(SIGALRM, reset_buf);

    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1){
        printf("no msr505 found\nusage: %s /dev/ttyUSB0\n", argv[0]);
    } else {
        printf("msr505 found: %d\n", fd);
        struct termios tp;
        tcgetattr(fd, &tp);
        cfsetspeed(&tp, 9600);
        tcsetattr(fd, TCSANOW, &tp);
        fcntl(fd, F_SETFL, FNDELAY);

        safe_write(fd, CMD_SET_ZEROS, sizeof(CMD_SET_ZEROS));
        safe_write(fd, CMD_SET_DENSITY, sizeof(CMD_SET_DENSITY));

        Fl::add_fd(fd, FL_READ, msr505_read, (void *)&win);
    }

    win.show();
    return(Fl::run());
}
