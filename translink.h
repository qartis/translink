#define BLANK_OFFSET 15
#define SERIAL_OFFSET 16
#define MACHINE_ID_OFFSET 16
#define EXP_DATE_OFFSET 30
#define EXP_TIME_OFFSET 44
#define FLAGS_OFFSET 52
#define ADDFARE_OFFSET 72
#define TICKET_TYPE_OFFSET 73
#define CONCESSION_OFFSET 66
#define NUM_ZONES_OFFSET 76
#define ZONES_VISITED_OFFSET 78
#define ACTIVATION_DATE_OFFSET 81
#define CHECKSUM_OFFSET 108
#define DAYPASS_OFFSET 65

/* DDR Vehicle ? */


#define max(a,b) ((a>b)?a:b)
#define min(a,b) ((a<b)?a:b)

unsigned int readn(char *str, int n)
{
    unsigned int num = 0;
    int i;
    for(i=0;i<n;i++){
        num <<= 1;
        num |= str[i]-'0';
    }
    return num;
}

static const uint16_t crc_lut[256] = {
    0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
    0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
    0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
    0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
    0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
    0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
    0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
    0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
    0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
    0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
    0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
    0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
    0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
    0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
    0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
    0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
    0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
    0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
    0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
    0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
    0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
    0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
    0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
    0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
    0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
    0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
    0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
    0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
    0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
    0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

uint16_t fcs16(uint16_t crc, uint8_t *data, unsigned int len)
{
    uint16_t next;
    data+=4;
    while(len--){
        next = readn((char *)data, 8);
        crc = crc_lut[(next ^ crc) & 0xff] ^ (crc >> 8);
        data += 8;
    }

    crc = crc_lut[(0 ^ crc) & 0xff] ^ (crc >> 8);
    return ~crc;
}

/* CHEC MW means "check message writer".

Message Writer is the old DOS program to program Luminator destination signs. Luminator updated the program to Win32, and called it IPS.

You use a piece of hardware called an MTU (Mobile Transfer Unit) to put the new codes onto a bus.

One minor correction ... the MW stands for Message Writer. This code goes back to the old MS-DOS days of Luminator programming and Message Writer was the software to program the signs.

The MW program is no longer around as Luminator updated the software to a Windows based program, called IPS. It is kind of funny that they did not update the error message at the time.

Any sign can say "CHK/MW" even the front sign. All that it means is that the programmer did not put any code for that specific sign.

P/R = public relations


P/R messages are displayed in addition to the programmed route, for those who didn't know.


TTC still uses that program, and not the WYSIWYG program that everyone else does...TTC's old programmer always called it memory writer. 
      

CHK MW (as confirmed by a Luminator rep at the TransExpo in
Vancouver last year) is a programming problem with the guy in HQ (or
maintenance) with the computer that does the actual programming of codes
and destinations. Vancouver had this problem a lot (maybe they still
do, not sure) with "Not In Service" signs where CHK MW showed up on the
    route sign in back. Basically, if a sign is supposed to be blank (such
    as the route number box for a "Not In Service" sign), the programmer
    (not the driver) has to check a particular box in the software that
    tells it to keep that sign blank. If they simply fail to input text
    into that sign, but don't tell the software that it's supposed to be
    blank, the sign will look for data that isn't there.
*/

void superimpose(char *bits, unsigned int val, int offset, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        bits[offset + i] = !!(val & (1 << (len - i - 1))) + '0';
    }
}

char *encode(struct tm tm, int z_visited, int n_zones){
    static char bits[] = "000000010111001000010000010111010001100101000001"
                         "100000000001000000000000001111001010001100101100"
                         "00000000000011001011000100010000";
    struct tm epoch = {0};
    time_t now_time;
    time_t epoch_time;
    int secs;
    int mins;
    int expdate;
    int exptime;
    uint16_t calculated;

    tm.tm_isdst = -1;

    epoch.tm_mday = 31;
    epoch.tm_mon = 12 - 1; /* 0 - 11 */
    epoch.tm_year = 1999 - 1900;
    epoch.tm_hour = 17;
    epoch.tm_min = 36;

    now_time   = mktime(&tm);
    epoch_time = mktime(&epoch);

    secs = difftime(now_time, epoch_time);
    mins = secs / 60;
    expdate = mins / (60 * 24);
    exptime = mins % (60 * 24) / 6;

    z_visited = 1 << (z_visited - 1);

    superimpose(bits, expdate, EXP_DATE_OFFSET, 14);
    superimpose(bits, exptime, EXP_TIME_OFFSET, 8);
    superimpose(bits, z_visited, ZONES_VISITED_OFFSET, 3);
    superimpose(bits, n_zones, NUM_ZONES_OFFSET, 2);
    superimpose(bits, n_zones, TICKET_TYPE_OFFSET, 3);
    superimpose(bits, expdate + 2, ACTIVATION_DATE_OFFSET, 14);

    calculated = fcs16(0xffff, (uint8_t *)bits, 12);

    superimpose(bits, calculated, CHECKSUM_OFFSET, 16);

    return bits;
}

char *printb(int value, int len)
{
    static char buf[64];
    char *ptr = buf;
    int i;

    for(i=0;i<len;i++) {
        if (value & (1 << (len - i - 1))) {
            *ptr++ = '1';
        } else {
            *ptr++ = '0';
        }
    }

    return buf;
}

int reformat(char *bits, uint8_t *bytes)
{
    int len;
    uint8_t byte;
    int count = 0;
    int header_len = 7;
    bits += strspn(bits, "0");


    while (*bits) {
        if (strchr(bits, '1') == NULL) break;
        len = min(8, strlen(bits));
        byte = readn(bits, len);
        byte = (byte * 0x0202020202ULL & 0x010884422010ULL) % 1023;
        bytes[header_len + count++] = byte;
        bits += len;
    }

    bytes[0] = 0x1b;
    bytes[1] = 0x6e;
    bytes[2] = 0x1b;
    bytes[3] = 0x73;
    bytes[4] = 0x1b;
    bytes[5] = 0x02;
    bytes[6] = count;
    bytes[header_len + count + 0] = 0x3f;
    bytes[header_len + count + 1] = 0x1c;

    return header_len + count + 2;
}

int unformat(uint8_t *bytes, int len, char *bits)
{
    int i = 0;
    int j;
    printf("bytes:\n");
    for(i=0;i<len;i++){
        printf("buf[%d] = %#x\n", i, bytes[i]);
    }
    i = 0;
    if (bytes[0] == 0x1b){
        i = 7;
        len -= 2;
    }
    strcpy(bits, "0000000");
    int idx = 7;
    for(;i<len;i++){
        for(j=0;j<8;j++){
            bits[idx++] = !!(bytes[i] & (1 << j)) + '0';
        }
    }
    bits[idx] = '\0';
    if (strncmp(bits, "00000001011100", strlen("00000001011100")) != 0){
        memmove(bits + strlen("00000001011100"), bits + 7, strlen(bits) - 7);
        memcpy(bits, "00000001011100", strlen("00000001011100"));
        idx = strlen(bits);
    }
    return idx;
}

void calcdayoffset(struct tm *tm, int offset)
{
    tm->tm_mday = 31;
    tm->tm_mon = 12 - 1; /* 0 - 11 */
    tm->tm_year = 1999-1900;
    tm->tm_mday += offset;
}

const char *machinename(int machine_id, int assume_canada_line){
    unsigned i;
    static char buf[128];
    const char *expo[] = {"Waterfront", "Burrard", "Granville", "Stadium",
        "Main St", "Broadway", "Nanaimo", "29th Ave", "Joyce", "Patterson",
        "Metrotown", "Royal Oak", "Edmonds", "22nd St", "New West", "Columbia",
        "Scott Road", "Gateway", "Surrey Central", "King George"};
    const char *mill[] = {"VCC/Clark", "Commercial", "Renfrew", "Rupert",
        "Gilmore", "Brentwood", "Holdom", "Sperling", "Lake City Way",
        "Production Way", "Lougheed", "Braid", "Sapperton"};
    const char *canada[] = {"Waterfront", "Vancouver City Centre", "Yaletown",
        "Olympic Village", "Broadway", "King Edward", "Oakridge", "Langara",
        "Marine Drive", "Bridgeport", "Templeton", "Sea Island Centre",
        "YVR-Airport", "Aberdeen", "Landsdowne", "Richmond-Brighouse"};

    struct {
        int sign;
        const char *msg;
    } routes[] = {
#include "busses.h"
    };

    if ((10001 <= machine_id) && (machine_id <= 10020)){
        sprintf(buf, "%s expo line stn", expo[machine_id - 10001]);
        return buf;
    } else if ((10031 <= machine_id) && (machine_id <= 10043)){
        sprintf(buf, "%s millennium line stn", mill[machine_id - 10031]);
        return buf;
    } else if (machine_id == 10090) {
        return "Seabus";
    } else if (machine_id == 73) {
        return "YVR-airport canada line special";
    }

    if (assume_canada_line) {
        if ((50 <= machine_id) && (machine_id <= 65)){
            sprintf(buf, "%s canada line stn", canada[machine_id - 50]);
            return buf;
        }
    }

    for(i=0;i<sizeof(routes)/sizeof(routes[0]);i++){
        if (machine_id == routes[i].sign){
            return routes[i].msg;
        }
    }

    if (!assume_canada_line && (50 <= machine_id) && (machine_id <= 65)){
        sprintf(buf, "%s canada line stn", canada[machine_id - 50]);
        return buf;
    }

    return "unknown machine id";
}

int decode(char *str){
    struct tm time = {0};
    char buf[64];
    int extrayears = 0;
    char *resized;

    int len = strlen(str);
    if (len < CHECKSUM_OFFSET + 16) {
        printf("decode(): input too short(%d), padding..\n", len);

        resized = calloc(1024, 1);
        strcpy(resized, str);
        resized[len] = '0';
        resized[130] = '\0';
        str = resized;
    }

    int blank = readn(str + BLANK_OFFSET, 1);
    int serial = readn(str + SERIAL_OFFSET, 32);
    int machine_id = readn(str + MACHINE_ID_OFFSET, 14);
    int expdate = readn(str + EXP_DATE_OFFSET, 14);
    int exptime = readn(str + EXP_TIME_OFFSET, 8);
    int flags = readn(str + FLAGS_OFFSET, 8);
    int addfare = readn(str + ADDFARE_OFFSET, 1);
    int ticket_type = readn(str + TICKET_TYPE_OFFSET, 3);
    int num_zones = readn(str + NUM_ZONES_OFFSET, 2);
    int zone_info = readn(str + ZONES_VISITED_OFFSET, 3);
    int activation_date = readn(str + ACTIVATION_DATE_OFFSET, 14);
    int daypass = readn(str + DAYPASS_OFFSET, 1);
    int concession = readn(str + CONCESSION_OFFSET, 1);
    uint16_t checksum = readn(str + CHECKSUM_OFFSET, 16);

    uint16_t calculated = fcs16(0xffff, (uint8_t *)str, 12);
    if (calculated != checksum){
        printf("checksum error (%04x)\n", calculated);
        return 1;
    }

    time.tm_isdst = -1;

    calcdayoffset(&time, expdate);

    time.tm_hour = 17;
    time.tm_min = 36;

    if (time.tm_isdst){
        time.tm_hour = 18;
    }

    time.tm_hour += exptime / 10;
    time.tm_min += (exptime % 10) * 6;
    if (time.tm_mday > 14000) { /* days between 1999 and 2038 */
        extrayears = time.tm_mday / 1461;
        extrayears *= 4;
        time.tm_mday %= 1461;
    }

    mktime(&time);
    time.tm_year += extrayears;

    strftime(buf, sizeof(buf), "%a, %b %d %Y %I:%M %p", &time);

    char zones_visited[] = "...";
    if (zone_info & 0b001) {
        zones_visited[0] = '1';
    }
    if (zone_info & 0b010) {
        zones_visited[1] = '2';
    }
    if (zone_info & 0b100) {
        zones_visited[2] = '3';
    }

    if (concession) {
        printf("C");
    } else {
        printf("A");
    }

    if (daypass) {
        printf("D");
    } else {
        printf(" ");
    }

    if (addfare) {
        printf("+");
    } else {
        printf(" ");
    }

    struct ticket {
        int daypass : 1;
        int concession: 1;
    };

    const char *type_str[] = {
        "CONC DAY",
        "1 ZONE  ",
        "2 ZONE  ",
        "3 ZONE  ",
        "CONC 1Z ",
        "CONC 2Z ",
        "C3Z/ADDF",
        "DAYPASS "
    };

    const char *flags_str;
    if (exptime == flags) {
        flags_str = "[canada]";
    } else {
        flags_str = printb(flags, 8);
    }

    if (blank){
        printf("Blank %d zone faresaver, serial number %d-%d\n",
            num_zones, serial / 10, serial % 10);
    } else {
        printf(" %s, %d", buf, num_zones);
        printf(" zone(s) type: %d (%s)", ticket_type, type_str[ticket_type]);
        printf(" visited: %s", zones_visited);
        printf(" flags: %s", flags_str);
        printf(" sign: %d (%s) ", machine_id,
            machinename(machine_id, exptime == flags));

        if (activation_date == (1<<14)-1){
            printf("ticket type: faresaver\n");
        } else {
            printf("ticket type: transfer (expdate%+d)\n",
                activation_date - expdate);
        }
    }

    return 0;
}
