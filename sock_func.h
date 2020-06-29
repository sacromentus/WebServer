
typedef struct Header {
	size_t size;
	char *lines[4];
	int filetype;
} Header;

#define TEXT	0
#define GIF	1
#define JPG	2
#define PNG	3

void error(const char *);
int getServerSocketfd(int );
void init_header(Header *);
