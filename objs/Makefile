
CC = gcc
CFLAGS =  -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -fPIC
CPP = gcc -E
LINK = $(CC) -rdynamic  -fPIC  -shared  -undefined dynamic_lookup 


CORE_LIBS =    -lpcre -lssl -lcrypto  -lz -lcurl -lpthread
CORE_LINK = 
NGX_LD_OPT = 


ALL_INCS =  -I /Users/hupeng/Downloads/tengine-1.4.6/src/core -I /Users/hupeng/Downloads/tengine-1.4.6/src/event -I /Users/hupeng/Downloads/tengine-1.4.6/src/event/modules -I /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix -I /Users/hupeng/Downloads/tengine-1.4.6/src/proc -I /Users/hupeng/Downloads/tengine-1.4.6/objs -I /Users/hupeng/Downloads/tengine-1.4.6/src/http -I /Users/hupeng/Downloads/tengine-1.4.6/src/http/modules -I /Users/hupeng/Downloads/tengine-1.4.6/src/mail

CORE_DEPS =  /Users/hupeng/Downloads/tengine-1.4.6/src/core/nginx.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_config.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_core.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_log.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_palloc.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_array.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_list.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_hash.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_buf.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_queue.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_string.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_parse.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_inet.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_file.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_crc.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_crc32.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_murmurhash.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_md5.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_sha1.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_rbtree.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_trie.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_segment_tree.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_radix_tree.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_slab.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_times.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_shmtx.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_connection.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_cycle.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_conf_file.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_resolver.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_open_file_cache.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_crypt.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event_timer.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event_posted.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event_busy_lock.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event_connect.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event_pipe.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_time.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_errno.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_alloc.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_files.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_channel.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_shmem.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_process.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_setaffinity.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_setproctitle.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_atomic.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_gcc_atomic_x86.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_thread.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_socket.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_os.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_user.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_pipe.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_sysinfo.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_process_cycle.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_darwin_config.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_darwin.h /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix/ngx_syslog.h /Users/hupeng/Downloads/tengine-1.4.6/src/proc/ngx_proc.h /Users/hupeng/Downloads/tengine-1.4.6/src/event/ngx_event_openssl.h /Users/hupeng/Downloads/tengine-1.4.6/src/core/ngx_regex.h /Users/hupeng/Downloads/tengine-1.4.6/objs/ngx_auto_config.h

CORE_INCS =  -I /Users/hupeng/Downloads/tengine-1.4.6/src/core -I /Users/hupeng/Downloads/tengine-1.4.6/src/event -I /Users/hupeng/Downloads/tengine-1.4.6/src/event/modules -I /Users/hupeng/Downloads/tengine-1.4.6/src/os/unix -I /Users/hupeng/Downloads/tengine-1.4.6/src/proc -I /Users/hupeng/Downloads/tengine-1.4.6/objs

HTTP_DEPS =  /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_request.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_config.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_core_module.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_cache.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_variables.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_script.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_upstream.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_upstream_round_robin.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/ngx_http_busy_lock.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/modules/ngx_http_ssi_filter_module.h /Users/hupeng/Downloads/tengine-1.4.6/src/http/modules/ngx_http_ssl_module.h


HTTP_INCS =  -I /Users/hupeng/Downloads/tengine-1.4.6/src/http -I /Users/hupeng/Downloads/tengine-1.4.6/src/http/modules

ADDON_DEPS = $(CORE_DEPS) /Users/hupeng/Downloads/tengine-1.4.6/tengine-send-http-module//ngx_proc_daytime_module.c


objs/ngx_proc_daytime_module.so:	objs/src/tengine-send-http-module/ngx_proc_daytime_module.o

	$(LINK) -o  objs/ngx_proc_daytime_module.so \
	objs/src/tengine-send-http-module/ngx_proc_daytime_module.o \
	-lpcre -lssl -lcrypto -lz -lcurl -lpthread
	


objs/src/tengine-send-http-module/ngx_proc_daytime_module.o:	$(ADDON_DEPS) \
	/Users/hupeng/Downloads/tengine-1.4.6/tengine-send-http-module//ngx_proc_daytime_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/tengine-send-http-module/ngx_proc_daytime_module.o \
		/Users/hupeng/Downloads/tengine-1.4.6/tengine-send-http-module//ngx_proc_daytime_module.c

