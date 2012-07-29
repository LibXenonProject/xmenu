#define DEBUG printf
#define printf

void initScreen(int,int);
void updateScreen(int,int);

struct XenosSurface *gfxplane;
struct XenosDevice _xe, *xe;
struct XenosSurface *fb;
struct XenosShader *sh_ps, *sh_vs;
struct XenosVertexBuffer *vb;
struct XenosIndexBuffer *ib;

extern void do_edram_foo(struct XenosDevice *xe, int complete);
