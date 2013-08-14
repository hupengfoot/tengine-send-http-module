#include <ngx_event.h>
#include <ngx_core.h>
#include <ngx_config.h>

#include <curl/curl.h>
#include <pthread.h>

#define BUFSIZE 2000
#define SENDOUTBUFSIZE 400
#define WRITE_BUFFER_SIZE 1024
#define READ_WRITE_OFFSET_FILE "offsize_file"
#define READ_WRITE_FILE "write_buf"

static void *ngx_proc_send_create_conf(ngx_conf_t *cf);
static char *ngx_proc_send_merge_conf(ngx_conf_t *cf, void *parent,
		void *child);
static ngx_int_t ngx_proc_send_prepare(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_send_process_init(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_send_loop(ngx_cycle_t *cycle);
static void ngx_proc_send_exit_process(ngx_cycle_t *cycle);
static void ngx_proc_send_accept(ngx_event_t *ev);
void* send_info(void* arg);
int open_fifo();


char buf[BUFSIZE][201];
int start = 0;
int end = 0;
int filenum = 0;
long* read_write_mmap = NULL; 
char* write_file = NULL; 

extern int pipefd[2];

typedef struct {
	ngx_flag_t       enable;
	ngx_uint_t       port;
	ngx_socket_t     fd;
	ngx_str_t		destination;
} ngx_proc_send_conf_t;

//typedef struct{
//	char type;
//	long offset;
//}offset_t;


static ngx_command_t ngx_proc_send_commands[] = {

	{ ngx_string("listen"),
		NGX_PROC_CONF|NGX_CONF_TAKE1,
		ngx_conf_set_num_slot,
		NGX_PROC_CONF_OFFSET,
		offsetof(ngx_proc_send_conf_t, port),
		NULL },

	{ ngx_string("daytime"),
		NGX_PROC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_flag_slot,
		NGX_PROC_CONF_OFFSET,
		offsetof(ngx_proc_send_conf_t, enable),
		NULL },

	{ ngx_string("destination"),
		NGX_PROC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_str_slot,
		NGX_PROC_CONF_OFFSET,
		offsetof(ngx_proc_send_conf_t, destination),
		NULL },


	ngx_null_command
};


static ngx_proc_module_t ngx_proc_send_module_ctx = {
	ngx_string("send"),
	NULL,
	NULL,
	ngx_proc_send_create_conf,
	ngx_proc_send_merge_conf,
	ngx_proc_send_prepare,
	ngx_proc_send_process_init,
	ngx_proc_send_loop,
	ngx_proc_send_exit_process
};


ngx_module_t ngx_proc_send_module = {
	NGX_MODULE_V1,
	&ngx_proc_send_module_ctx,
	ngx_proc_send_commands,
	NGX_PROC_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
};


	static void *
ngx_proc_send_create_conf(ngx_conf_t *cf)
{
	ngx_proc_send_conf_t  *pbcf;

	pbcf = ngx_pcalloc(cf->pool, sizeof(ngx_proc_send_conf_t));

	if (pbcf == NULL) {
		ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
				"send create proc conf error");
		return NULL;
	}

	pbcf->enable = NGX_CONF_UNSET;
	pbcf->port = NGX_CONF_UNSET_UINT;

	return pbcf;
}


	static char *
ngx_proc_send_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
	ngx_proc_send_conf_t  *prev = parent;
	ngx_proc_send_conf_t  *conf = child;

	ngx_conf_merge_uint_value(conf->port, prev->port, 0);
	ngx_conf_merge_off_value(conf->enable, prev->enable, 0);

	return NGX_CONF_OK;
}


	static ngx_int_t
ngx_proc_send_prepare(ngx_cycle_t *cycle)
{
	ngx_proc_send_conf_t  *pbcf;

	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_send_module);
	if (!pbcf->enable) {
		return NGX_DECLINED;
	}

	if (pbcf->port == 0) {
		return NGX_DECLINED;
	}

	return NGX_OK;
}

	static ngx_int_t
ngx_proc_send_process_init(ngx_cycle_t *cycle)
{
	int                       reuseaddr;
	ngx_event_t              *rev;
	ngx_socket_t              fd;
	ngx_connection_t         *c;
	struct sockaddr_in        sin;
	ngx_proc_send_conf_t  *pbcf;

	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_send_module);
	fd = ngx_socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "send socket error");
		return NGX_ERROR;
	}

	reuseaddr = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(const void *) &reuseaddr, sizeof(int))
			== -1)
	{
		ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
				"send setsockopt(SO_REUSEADDR) failed");

		ngx_close_socket(fd);
		return NGX_ERROR;
	}
	if (ngx_nonblocking(fd) == -1) {
		ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
				"send nonblocking failed");

		ngx_close_socket(fd);
		return NGX_ERROR;
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(pbcf->port);

	if (bind(fd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "send bind error");
		return NGX_ERROR;
	}

	if (listen(fd, 20) == -1) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "send listen error");
		return NGX_ERROR;
	}

	c = ngx_get_connection(fd, cycle->log);
	if (c == NULL) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "send no connection");
		return NGX_ERROR;
	}

	c->log = cycle->log;
	rev = c->read;
	rev->log = c->log;
	rev->accept = 1;
	rev->handler = ngx_proc_send_accept;

	if (ngx_add_event(rev, NGX_READ_EVENT, 0) == NGX_ERROR) {
		return NGX_ERROR;
	}

	pbcf->fd = fd;

	return NGX_OK;
}


	static ngx_int_t
ngx_proc_send_loop(ngx_cycle_t *cycle)
{
	ngx_proc_send_conf_t  *pbcf;
	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_send_module);
	while(1){
		pthread_t tid;
		int err = pthread_create(&tid, NULL, send_info, pbcf->destination.data);
		if(err){
			continue;	
		}
		while(read(pipefd[0], buf[end], SENDOUTBUFSIZE) > 0){
			end = ( end + 1 ) % BUFSIZE;
			if(end == start){
				__sync_bool_compare_and_swap(&start, end, (end + 1) % BUFSIZE);
			}
		}
	}
	return NGX_OK;
}


	static void
ngx_proc_send_exit_process(ngx_cycle_t *cycle)
{
	ngx_proc_send_conf_t *pbcf;

	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_send_module);

	ngx_close_socket(pbcf->fd);
	munmap(read_write_mmap, sizeof(long)*2);
	munmap(write_file, sizeof(char)*WRITE_BUFFER_SIZE*2);
}


	static void
ngx_proc_send_accept(ngx_event_t *ev)
{
	u_char             sa[NGX_SOCKADDRLEN];
	socklen_t          socklen;
	ngx_socket_t       s;
	ngx_connection_t  *lc;

	lc = ev->data;
	s = accept(lc->fd, (struct sockaddr *) sa, &socklen);
	if (s == -1) {
		return;
	}

	if (ngx_nonblocking(s) == -1) {
		goto finish;
	}

finish:
	ngx_close_socket(s);
}

void* send_info(void* arg){
	//CURL *curl;
	//int errornum = 0;
	//curl = curl_easy_init();
	open_fifo();
	while(1){
		if(start != end){
			int p = start;
			char out[SENDOUTBUFSIZE + 100];
			memset(out, 0, SENDOUTBUFSIZE + 100);
			//sprintf(out, "%s/%s", (char*)arg, buf[p]);
			sprintf(out, "%s\n", buf[p]);
			if(__sync_bool_compare_and_swap(&start, p, (p + 1) % BUFSIZE)){
				//curl_easy_setopt(curl, CURLOPT_URL, out);
				//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
				//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
				if((*read_write_mmap >= *(read_write_mmap + 1) && *read_write_mmap < (long)(WRITE_BUFFER_SIZE)) || (*read_write_mmap < *(read_write_mmap + 1) && *read_write_mmap + (long)strlen(out) < *(read_write_mmap + 1))){
					strcpy(write_file + *read_write_mmap, out);
					*read_write_mmap = *read_write_mmap + strlen(out);
				}
				else if(*read_write_mmap >= *(read_write_mmap + 1) && *read_write_mmap > (long)(WRITE_BUFFER_SIZE) && *(read_write_mmap + 1) > (long)strlen(out)){
					strcpy(write_file, out);
					*read_write_mmap = strlen(out);
				}
				//		if(curl_easy_perform(curl)){
				//			errornum ++;
				//			if(errornum > 3){
				//				sleep(100);
				//				curl_easy_cleanup(curl);
				//				curl = curl_easy_init();
				//				errornum = 0;
				//			}	
				//		}
				//		else{
				//			errornum = --errornum > 0 ? errornum : 0;
				//		}
			}
		}
		else{
			usleep(10);
		}
	}
	//curl_easy_cleanup(curl);
	return NULL;
}

long get_file_size(char *filename) 
{ 
	struct stat f_stat; 

	if( stat( filename, &f_stat ) == -1 ){ 
		return -1; 
	} 

	return (long)f_stat.st_size; 
}

int open_fifo(){
	int fd = 0;
	int off_fd = 0;
	if(access(READ_WRITE_OFFSET_FILE, 0) == -1){
		off_fd = open(READ_WRITE_OFFSET_FILE, O_CREAT|O_RDWR, 0777);
		if(off_fd == -1){
			return -1;
		}
		lseek(off_fd, sizeof(long)*2-1, SEEK_SET);
		write(off_fd,"",1);
		read_write_mmap = (long*) mmap( NULL,sizeof(long)*2, PROT_READ|PROT_WRITE, MAP_SHARED, off_fd, 0);
		*read_write_mmap = 0;
		*(read_write_mmap + 1) = 0;
	}
	else{
		off_fd = open(READ_WRITE_OFFSET_FILE, O_CREAT|O_RDWR, 0777);
		if(off_fd == -1){
			return -1;
		}
		if(get_file_size(READ_WRITE_OFFSET_FILE) < (long)sizeof(long)*2){
			lseek(off_fd, sizeof(long)*2-1, SEEK_SET);
			write(off_fd,"",1);
			read_write_mmap = (long*) mmap( NULL,sizeof(long)*2, PROT_READ|PROT_WRITE, MAP_SHARED, off_fd, 0);
			*read_write_mmap = 0;
			*(read_write_mmap + 1) = 0;
		}
		read_write_mmap = (long*) mmap( NULL,sizeof(long)*2, PROT_READ|PROT_WRITE, MAP_SHARED, off_fd, 0);
	}
	close(off_fd);

	if(access(READ_WRITE_FILE, 0) == -1){
		fd = open(READ_WRITE_FILE, O_CREAT|O_RDWR, 0777);
		if(fd == -1){
			return -1;
		}
		lseek(fd, sizeof(char)*WRITE_BUFFER_SIZE*2 - 1, SEEK_SET);
		write(fd,"",1);
		write_file = (char*) mmap( NULL,sizeof(char)*WRITE_BUFFER_SIZE*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	}
	else{
		fd = open(READ_WRITE_FILE, O_CREAT|O_RDWR, 0777);
		if(fd == -1){
			return -1;
		}
		if(get_file_size(READ_WRITE_FILE) < (long)sizeof(char)*WRITE_BUFFER_SIZE*2){
			lseek(fd, sizeof(char)*WRITE_BUFFER_SIZE*2, SEEK_SET);
			write(off_fd,"",1);
			read_write_mmap = (long*) mmap(NULL, sizeof(char)*WRITE_BUFFER_SIZE*2, PROT_READ|PROT_WRITE, MAP_SHARED, off_fd, 0);
		}
		write_file = (char*) mmap( NULL,sizeof(char)*WRITE_BUFFER_SIZE*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	}
	close(fd);
	return 0;
}
