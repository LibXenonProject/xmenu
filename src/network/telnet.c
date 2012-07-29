#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

#define FALSE 0
#define TRUE 1
#define BUFSIZE	2000

typedef struct {
	int len;
	char buf[BUFSIZE];
} TEL_TXST;

static TEL_TXST tel_st;
static int close_session;
static struct tcp_pcb *tel_pcb;

//static void flush_queue(void);
static void telnet_send(struct tcp_pcb *pcb, TEL_TXST *st);
static err_t telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t telnet_sent(void *arg, struct tcp_pcb *pcb, u16_t len);

void tel_init(void)
{	
err_t err;


	tel_pcb = tcp_new();									/* new tcp pcb */

	if(tel_pcb != NULL)
	{
		close_session = FALSE;								/* reset session */
		err = tcp_bind(tel_pcb, IP_ADDR_ANY, 23);		/* bind to port */

		if(err == ERR_OK)
		{
			tel_pcb = tcp_listen(tel_pcb);					/* set listerning */
			tcp_accept(tel_pcb, telnet_accept);				/* register callback */
		}
		else
		{

		}
	}
	else
	{

	}

	printf(" * Telnet server ok\n");
}

static void telnet_close(struct tcp_pcb *pcb)
{

	close_session = FALSE;
	tcp_arg(pcb, NULL);									/* clear arg/callbacks */
	tcp_err(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_close(pcb);
}

void tel_close(void)
{
	close_session = TRUE;								/* flag close session */
}

void tel_tx_str( char *buf )
{
	TEL_TXST *st;


	st = &tel_st;									
	memcpy(st->buf + st->len, buf, strlen(buf));					/* copy data */
	st->len += strlen(buf);							/* inc length */
	//printf("tel -> tel_tx_str - TxPut (%i)\n", st->len);
}

static check_tx_que(void)
{
int tx_bc;
TEL_TXST *st;

	
	st = &tel_st;

	if(st->len > 0)
	{
		//printf("tel -> check_tx_que - st->len (%i)\n", st->len);
		tcp_sent(tel_pcb, telnet_sent);				/* register callback */
		telnet_send(tel_pcb, st);				/* send telnet data */
	}
}

static void telnet_send(struct tcp_pcb *pcb, TEL_TXST *st)
{
err_t err;
u16_t len;


	if(tcp_sndbuf(pcb) < st->len)
	{
		len = tcp_sndbuf(pcb);								
	}
	else
	{ 
		len = st->len;
	}

	do 
	{
		err = tcp_write(pcb, st->buf, len, 0);		

		//printf("tel -> telnet_send - tcp_write (%i)\n", len);

		if(err == ERR_MEM)
		{ 
			len /= 2;
		}
	} 
	while(err == ERR_MEM && len > 1);  
  
	if(err == ERR_OK)
	{ 
		st->len -= len;

		//printf("tel -> telnet_send - tcp_write (%i)\n", st->len);
	}
}

static err_t telnet_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
TEL_TXST *st;

	
	st = arg;
 
	if(st->len > 0)			
	{ 
		//printf ("tel -> telnet_sent - telnet_send (%i)\n", st->len);

		telnet_send(pcb, st);
	}

	return ERR_OK;
}
/*
char path_now[1024];
Dirent *get_dir(const char *name);
Dirent *list = NULL;
char * strcat(char * dest, const char * src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

void get_fuses()
{
	int i;

	tel_tx_str("\r\n", 1);

	for (i=0; i<12; ++i)
	{
		char fuse[512];
		memset( fuse, 0, 512 );
		sprintf( fuse, "fuseset %02d: %016lx\r\n", i, *(unsigned long*)(0x8000020000020000 + (i * 0x200)));
		tel_tx_str( fuse, 0 );
	}

	tel_tx_str("\r\n", 1);
}

void ls_cmd( char *directory )
{
	int i;
	int ret = 0;

	if( strlen( directory ) < 3 )
	{
		if ( ( list = get_dir( path_now ) ) != NULL )
			ret = 1;
	}
	else
	{
		char dir_ask[512];
		memset( dir_ask, 0, 512) ;

		for ( i = 3; i < strlen( directory ); i++ )
			dir_ask[i-3] = directory[i];

		if ( ( list = get_dir( dir_ask ) ) != NULL )
			ret = 1;
	}

	if ( ret )
	{
		for ( i = 0; i < list->count; i++ )
		{
			if ( list->is_dir[i] )
				tel_tx_str("\x1B[36m", 0 );
			else
				tel_tx_str("\x1b[0m", 0 );

			tel_tx_str( list->name[i], 0  );
			tel_tx_str("\r\n", 1);
		}
		tel_tx_str("\x1b[0m", 0 );
	}
	else
	{
		tel_tx_str( "Directory not found\r\n", 0  );;		
	}
}

void cd_cmd( char *folder )
{
	char folder_ask[1024];
	memset(folder_ask, 0, 1024);

	char path_old[1024];
	memset(path_old, 0, 1024);
	strcpy( path_old, path_now );

	int i;
	for ( i = 3; i < strlen( folder ) ; i++ )
	{
		folder_ask[i-3] = folder[i];
	}
	
	if ( strlen( folder_ask ) > 0 )
	{
		int path_now_len = strlen( path_now );

		if ( strncmp ( folder_ask, "..", 2 )  == 0 )
		{
			if ( path_now_len > 1 )
			{
				char new_path[1024];
				int not_found = 1;
				for ( i = path_now_len; i > 0; i-- )
				{
					if ( path_now[i] == '/' )
					{
						strncpy( new_path, path_now, i );
						memset( path_now, 0, 1024 );
						strncpy( path_now, new_path, i );
						printf("path_now:%s new_path:%s\n", path_now, new_path);
						not_found = 0;
						break;
					}
				}
				if ( not_found ) 
				{
					memset( path_now, 0, 1024 );
					strncpy( path_now, "/", 1 );
				}
			}
		}

		if ( ( list = get_dir( folder_ask ) ) != NULL )
		{
			memset( path_now, 0, 1024 );
			strcpy( path_now, folder_ask );
			return;
		}
		
		strcat( path_now, "/" );
		strcat( path_now, folder_ask );
		if ( ( list = get_dir( path_now ) ) != NULL )
		{
			return;
		}	
	
		memset( path_now, 0, 1024 );
		strcpy( path_now, path_old );
		tel_tx_str( "Directory do not exist\r\n", 0  );
	}
}

#define LOADER_RAW         0x8000000004000000ULL
#define LOADER_MAXSIZE     0x1000000

void ld_cmd(char *exec )
{
	int i;

	char exec_ask[512];
	memset( exec_ask, 0, 512) ;

	char path_new[1024];
	memset(path_new, 0, 1024);
	strcpy( path_new, path_now );
	
	for ( i = 5; i < strlen( exec ); i++ )
	{
		exec_ask[i-5] = exec[i];
	}

	if ( fat_open( exec_ask ) )
	{
		strcat( path_new, "/" );
		strcat( path_new, exec_ask );
		if ( fat_open( path_new ) )
			return;
	}
	
	else
	{
		close_session = TRUE;
		telnet_close(tel_pcb);

		printf(" * fat open okay, loading file...\n");
		int r = fat_read(LOADER_RAW, LOADER_MAXSIZE);
		printf(" * executing...\n");
		execute_elf_at((void*)LOADER_RAW);
	}
}

void tftp_cmd()
{
	if ( boot_tftp(network_boot_server_name(), network_boot_file_name()) )
	{
		close_session = TRUE;
		telnet_close(tel_pcb);
	}
}

void help_cmd()
{
	tel_tx_str("\r\n", 0);
	tel_tx_str("print: print a text on screen\r\n", 0);	
	tel_tx_str("ls: list current directory\r\n", 0);
	tel_tx_str("cd: enter directory\r\n", 0);	
	tel_tx_str("exec: execute an elf32 binary\r\n", 0);
	tel_tx_str("tftp: try tftp boot\r\n", 0);
	tel_tx_str("get_fuses: print fuses data\r\n", 0);
	tel_tx_str("help: this message\r\n", 0);
	tel_tx_str("\r\n", 0);
	
}


void telnet_check_cmd( struct pbuf *p )
{
	char msg[512];
	memset(msg, 0, 512);

	strncpy(msg, p->payload, p->tot_len-2 );

	if ( strncmp ( msg, "exit", 4 ) == 0 )
	{
		tel_close();
			
	}
	else if ( strncmp ( msg, "print", 5 ) == 0 ) 
	{
		printf("%s", msg );
			
	}
	else if ( strncmp ( msg, "ls", 2 ) == 0 )
	{
		ls_cmd( msg );		
	}
	else if ( strncmp ( msg, "cd", 2 ) == 0 )
	{
		cd_cmd(msg);		
	}
	else if ( strncmp ( msg, "exec", 4 )  == 0 )
	{
		ld_cmd( msg );
	}
	else if ( strncmp ( msg, "tftp", 4 )  == 0 )
	{
		tftp_cmd( msg );
	}
	else if ( strncmp ( msg, "help", 4 )  == 0 )
	{
		help_cmd();
	}

	else if ( strncmp ( msg, "get_fuses", 9 )  == 0 )
	{
		get_fuses();
	}
	else
	{
		tel_tx_str("> Unknow command\r\n", 16);
	}
	tel_tx_str("> ", 2);
}
*/
static err_t telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{

	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);	/* some recieved data */
		//telnet_check_cmd(p);
		tel_tx_str( p );
	}

	pbuf_free(p);				/* dealloc mem */
	if (err == ERR_OK && p == NULL)
	{
		telnet_close(pcb);
	}

	return ERR_OK;
}

static void telnet_err(void *arg, err_t err)
{

	printf ("tel -> telnet_err - error (0x%.4x)\n", err);
}

static err_t telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
int i;


//	vt1_init();					/* init vt100 interface */

	tel_pcb = pcb;
	tel_st.len = NULL;				/* reset length */
	close_session = FALSE;				/* reset session */
	tcp_arg(pcb, &tel_st);				/* argument passed to callbacks */
	tcp_err(pcb, telnet_err);			/* register error callback */
	tcp_recv(pcb, telnet_recv);			/* register recv callback */	

//	strcpy( path_now, "/" );

	tel_tx_str("\x1b[2J");			/* clear screen */		
	tel_tx_str("\x1b[0;0H");			/* set cursor position */
//	tel_tx_str("\x1b[0m", 0 );
	tel_tx_str("Xell telnet shell by Cpasjuste\r\n"); /* display message */
	tel_tx_str("Type help for ... help\r\n"); /* display message */
	tel_tx_str("> ");

	//printf ("tel -> telnet_accept\n", err);

	return ERR_OK;
}

void tel_process(void)
{	

	if(close_session == TRUE)
	{
		telnet_close(tel_pcb);			/* close telnet session */
	}
	else
	{
		check_tx_que();					/* any data to be sent */
	}
}

