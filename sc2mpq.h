#include <stdint.h>

struct mpq {

};

void mpq_init();
struct mpq *mpq_open(const char *);
char *mpq_read_file(struct mpq *, int, char *);
int mpq_find_file(struct mpq *, const char *, int *);
