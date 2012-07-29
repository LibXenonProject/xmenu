#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <xenos/xe.h>
#include <xenon_sound/sound.h>
#include <console/console.h>
#include <diskio/diskio.h>
#include <fat/fat.h>
#include <xenos/xenos.h>

extern void xenos_init();
extern void console_init();
extern void usb_init();
extern void do_edram_foo(struct XenosDevice *xe, int complete);
extern void xenon_sound_init(void);
extern void xenon_sound_submit(void *data, int len);
extern int xenon_sound_get_free(void);

#define SMS_WIDTH 512
#define SMS_HEIGHT 384
