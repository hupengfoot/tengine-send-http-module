#include <ngx_event.h>
#include <ngx_core.h>
#include <ngx_config.h>

#include <curl/curl.h>
#include <pthread.h>

#define SHARE_MEMORY 0
#define BUFSIZE 2000

static void *ngx_proc_daytime_create_conf(ngx_conf_t *cf);
static char *ngx_proc_daytime_merge_conf(ngx_conf_t *cf, void *parent,
		void *child);
static ngx_int_t ngx_proc_daytime_prepare(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_daytime_process_init(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_daytime_loop(ngx_cycle_t *cycle);
static void ngx_proc_daytime_exit_process(ngx_cycle_t *cycle);
static void ngx_proc_daytime_accept(ngx_event_t *ev);
#if !SHARE_MEMORY
void* send_info(void* arg);
#endif

static char  *week[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday" };

static char  *months[] = { "January", "February", "March", "April", "May",
	"June", "July", "August", "Semptember", "October",
	"November", "December" };
char buf[BUFSIZE][101];
int start = 0;
int end = 0 ;

#if SHARE_MEMORY
extern ngx_queue_t		*shqueue;
extern ngx_slab_pool_t	*shpool;
#else
extern int pipefd[2];
#endif

typedef struct {
	ngx_flag_t       enable;
	ngx_uint_t       port;
	ngx_socket_t     fd;
} ngx_proc_daytime_conf_t;


static ngx_command_t ngx_proc_daytime_commands[] = {

	{ ngx_string("listen"),
		NGX_PROC_CONF|NGX_CONF_TAKE1,
		ngx_conf_set_num_slot,
		NGX_PROC_CONF_OFFSET,
		offsetof(ngx_proc_daytime_conf_t, port),
		NULL },

	{ ngx_string("daytime"),
		NGX_PROC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_flag_slot,
		NGX_PROC_CONF_OFFSET,
		offsetof(ngx_proc_daytime_conf_t, enable),
		NULL },


	ngx_null_command
};


static ngx_proc_module_t ngx_proc_daytime_module_ctx = {
	ngx_string("daytime"),
	NULL,
	NULL,
	ngx_proc_daytime_create_conf,
	ngx_proc_daytime_merge_conf,
	ngx_proc_daytime_prepare,
	ngx_proc_daytime_process_init,
	ngx_proc_daytime_loop,
	ngx_proc_daytime_exit_process
};


ngx_module_t ngx_proc_daytime_module = {
	NGX_MODULE_V1,
	&ngx_proc_daytime_module_ctx,
	ngx_proc_daytime_commands,
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
ngx_proc_daytime_create_conf(ngx_conf_t *cf)
{
	ngx_proc_daytime_conf_t  *pbcf;

	pbcf = ngx_pcalloc(cf->pool, sizeof(ngx_proc_daytime_conf_t));

	if (pbcf == NULL) {
		ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
				"daytime create proc conf error");
		return NULL;
	}

	pbcf->enable = NGX_CONF_UNSET;
	pbcf->port = NGX_CONF_UNSET_UINT;

	return pbcf;
}


	static char *
ngx_proc_daytime_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
	ngx_proc_daytime_conf_t  *prev = parent;
	ngx_proc_daytime_conf_t  *conf = child;

	ngx_conf_merge_uint_value(conf->port, prev->port, 0);
	ngx_conf_merge_off_value(conf->enable, prev->enable, 0);

	return NGX_CONF_OK;
}


	static ngx_int_t
ngx_proc_daytime_prepare(ngx_cycle_t *cycle)
{
	ngx_proc_daytime_conf_t  *pbcf;

	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_daytime_module);
	if (!pbcf->enable) {
		return NGX_DECLINED;
	}

	if (pbcf->port == 0) {
		return NGX_DECLINED;
	}

	return NGX_OK;
}


	static ngx_int_t
ngx_proc_daytime_process_init(ngx_cycle_t *cycle)
{
	int                       reuseaddr;
	ngx_event_t              *rev;
	ngx_socket_t              fd;
	ngx_connection_t         *c;
	struct sockaddr_in        sin;
	ngx_proc_daytime_conf_t  *pbcf;

	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_daytime_module);
	fd = ngx_socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "daytime socket error");
		return NGX_ERROR;
	}

	reuseaddr = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(const void *) &reuseaddr, sizeof(int))
			== -1)
	{
		ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
				"daytime setsockopt(SO_REUSEADDR) failed");

		ngx_close_socket(fd);
		return NGX_ERROR;
	}
	if (ngx_nonblocking(fd) == -1) {
		ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
				"daytime nonblocking failed");

		ngx_close_socket(fd);
		return NGX_ERROR;
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(pbcf->port);

	if (bind(fd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "daytime bind error");
		return NGX_ERROR;
	}

	if (listen(fd, 20) == -1) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "daytime listen error");
		return NGX_ERROR;
	}

	c = ngx_get_connection(fd, cycle->log);
	if (c == NULL) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "daytime no connection");
		return NGX_ERROR;
	}

	c->log = cycle->log;
	rev = c->read;
	rev->log = c->log;
	rev->accept = 1;
	rev->handler = ngx_proc_daytime_accept;

	if (ngx_add_event(rev, NGX_READ_EVENT, 0) == NGX_ERROR) {
		return NGX_ERROR;
	}

	pbcf->fd = fd;

	return NGX_OK;
}


	static ngx_int_t
ngx_proc_daytime_loop(ngx_cycle_t *cycle)
{
	ngx_log_error(NGX_LOG_EMERG, cycle->log, 0, "daytime %V",
			&ngx_cached_http_time);
	//CURL *curl;

#if SHARE_MEMORY
	while(1){
		while(shqueue->next != shqueue->prev){
			ngx_queue_t* tmp = shqueue->next;
			ngx_queue_remove(tmp);

			char out[100];
			memset(out, 0, 100);
			sprintf(out, "192.168.22.95/%d", *(int*)((u_char*)tmp - 8));
			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_URL, out);
			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
			curl_easy_setopt(curl, CURLOPT_HTTPGET, "");
			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			ngx_slab_free(shpool, (u_char*)tmp - 8);
		}
		sleep(1);
	}
#else
	while(1){
		pthread_t tid;
		int err = pthread_create(&tid, NULL, send_info, NULL);
		if(err){
			continue;	
		}
		while(read(pipefd[0], buf[end], 100) > 0){
			end = ( end + 1 ) % BUFSIZE;
			if(end == start){
				__sync_bool_compare_and_swap(&start, end, (end + 1) % BUFSIZE);
			}

			//			memset(out, 0, 200);
			//			sprintf(out, "192.168.22.95/%s", buf[i]);
			//			curl = curl_easy_init();
			//			curl_easy_setopt(curl, CURLOPT_URL, out);
			//			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			//			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
			//			curl_easy_setopt(curl, CURLOPT_HTTPGET, "");
			//			curl_easy_perform(curl);
			//			curl_easy_cleanup(curl);
		}
	}
#endif
	return NGX_OK;
}


	static void
ngx_proc_daytime_exit_process(ngx_cycle_t *cycle)
{
	ngx_proc_daytime_conf_t *pbcf;

	pbcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_daytime_module);

	ngx_close_socket(pbcf->fd);
}


	static void
ngx_proc_daytime_accept(ngx_event_t *ev)
{
	u_char             sa[NGX_SOCKADDRLEN], buf[256], *p;
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

	/*
	   Daytime Protocol

http://tools.ietf.org/html/rfc867

Weekday, Month Day, Year Time-Zone
	 */
#if SHARE_MEMORY
	int shnum = *(ngx_int_t*)((u_char*)shqueue->next - 8);
	if(shqueue->next != shqueue->prev){
		ngx_queue_t* tmp = shqueue->next;
		ngx_queue_remove(tmp);
		ngx_slab_free_locked(shpool, (u_char*)tmp - 8);
	}
	p = ngx_sprintf(buf, "%s, %s, %d, %d, %d:%d:%d-%s %d",
			week[ngx_cached_tm->tm_wday],
			months[ngx_cached_tm->tm_mon],
			ngx_cached_tm->tm_mday, ngx_cached_tm->tm_year,
			ngx_cached_tm->tm_hour, ngx_cached_tm->tm_min,
			ngx_cached_tm->tm_sec, ngx_cached_tm->tm_zone, shnum);
#else
	p = ngx_sprintf(buf, "%s, %s, %d, %d, %d:%d:%d-%s %d",
			week[ngx_cached_tm->tm_wday],
			months[ngx_cached_tm->tm_mon],
			ngx_cached_tm->tm_mday, ngx_cached_tm->tm_year,
			ngx_cached_tm->tm_hour, ngx_cached_tm->tm_min,
			ngx_cached_tm->tm_sec, ngx_cached_tm->tm_zone);
#endif
	ngx_write_fd(s, buf, p - buf);

finish:
	ngx_close_socket(s);
}

#if !SHARE_MEMORY
void* send_info(void* arg){
	CURL *curl;
	curl = curl_easy_init();
	while(1){
		if(start != end){
			int p = start;
			char out[200];
			memset(out, 0, 200);
			sprintf(out, "192.168.26.48/%s", buf[p]);
			if(__sync_bool_compare_and_swap(&start, p, (p + 1) % BUFSIZE)){
				curl_easy_setopt(curl, CURLOPT_URL, out);
				//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
				curl_easy_setopt(curl, CURLOPT_HTTPGET, "");
				curl_easy_perform(curl);
			}
		}
		else{
			usleep(10);
		}
	}
	curl_easy_cleanup(curl);
}
#endif