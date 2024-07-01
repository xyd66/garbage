#ifndef __GARBAGE__H
#define __GARBAGE__H

void garbage_init(void);
void garbage_final(void);
char *garbage_category(char *category);

#define WGET_CMD "wget http://192.168.1.116:8080/?action=snapshot -O /tmp/4.jpg"
#define GARBAGE_FILE "/tmp/4.jpg"

#endif
