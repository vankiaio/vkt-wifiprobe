/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Martin d'Allens <martin.dallens@gmail.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

// FIXME: sprintf->snprintf everywhere.
//#include "rom.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "tcpclient.h"
#include "httpclient.h"
#include "devicexx_app.h"
#include "platform.h"

// Internal state.
typedef struct request_args_t {
	char            * hostname;
	int             port;
	bool            secure;
	char            * method;
	char            * path;
	char            * headers;
	char            * post_data;
	int             post_size;
	char            * buffer;
	int             buffer_size;
	void            * ctx;
	http_callback_t user_callback;
} request_args_t;

static int http_parallel = 0;

static void ICACHE_FLASH_ATTR http_finish(request_args_t * req, char * response_body, int http_status, char * response_headers, int body_size)
{
	http_parallel--;
	if (req != NULL)
		if (req->user_callback != NULL) { // Callback is optional.
			req->user_callback(req->ctx, response_body, body_size, http_status, response_headers);
		}
}

static void ICACHE_FLASH_ATTR free_request_args(request_args_t * req)
{
	if (NULL != req) {
		if (NULL != req->user_callback) {
			// Disable user callback
			req->user_callback = NULL;
		}
		if (NULL != req->hostname) {
			os_free(req->hostname);
			req->hostname = NULL;
		}
		if (NULL != req->method) {
			os_free(req->method);
			req->method = NULL;
		}
		if (NULL != req->path) {
			os_free(req->path);
			req->path = NULL;
		}
		if (NULL != req->headers) {
			os_free(req->headers);
			req->headers = NULL;
		}
		if (NULL != req->post_data) {
			os_free(req->post_data);
			req->post_data = NULL;
		}
		if (NULL != req->buffer) {
			os_free(req->buffer);
			req->buffer = NULL;
		}
		os_free(req);
		req = NULL;
	}
}

static int ICACHE_FLASH_ATTR chunked_decode(char * chunked, int size)
{
	char *src = chunked;
	char *end = chunked + size;
	int i, dst = 0;

	do
	{
		char * endstr = NULL;
		//[chunk-size]
		i = strtol(src, (char **) NULL, 16);
		HTTPCLIENT_DEBUG("Chunk Size:%d\r\n", i);
		if (i <= 0)
			break;
		//[chunk-size-end-ptr]
		src = (char *)os_strstr(src, "\r\n") + 2;
		//[chunk-data]
		os_memmove(&chunked[dst], src, i);
		src += i + 2; /* CRLF */
		dst += i;
	} while (src < end);

	//
	//footer CRLF
	//

	/* decoded size */
	return dst;
}

static void ICACHE_FLASH_ATTR receive_callback(void * arg, char * buf, unsigned short len)
{
	os_printf("%s called\n",__func__);
	struct espconn * conn = (struct espconn *)arg;
	request_args_t * req = (request_args_t *)conn->reverse;

	if (req->buffer == NULL) {
		// It is abnormal, disconnect it
		if (req->secure)
			espconn_secure_disconnect(conn);
		else
			espconn_disconnect(conn);
		return; // The disconnect callback will be called.
	}

	// Let's do the equivalent of a realloc().
	const int new_size = req->buffer_size + len;
	char * new_buffer;
	if (new_size > BUFFER_SIZE_MAX || NULL == (new_buffer = (char *)os_malloc(new_size))) {
		HTTPCLIENT_DEBUG("Response too long (%d)\n", new_size);
		req->buffer[0] = '\0'; // Discard the buffer to avoid using an incomplete response.
		if (req->secure)
			espconn_secure_disconnect(conn);
		else
			espconn_disconnect(conn);
		return; // The disconnect callback will be called.
	}

	os_memcpy(new_buffer, req->buffer, req->buffer_size);
	os_memcpy(new_buffer + req->buffer_size - 1 /*overwrite the null character*/, buf, len); // Append new data.
	new_buffer[new_size - 1] = '\0'; // Make sure there is an end of string.

	os_free(req->buffer);
	req->buffer = new_buffer;
	req->buffer_size = new_size;

//	uint16_t i;
//	for(i=0;i<new_size;i++)
//		os_printf("%02x", new_buffer[i]);
//	os_printf("\n");


    resolution_times(new_buffer);

	espconn_disconnect(conn);
}

static void ICACHE_FLASH_ATTR sent_callback(void * arg)
{
	struct espconn * conn = (struct espconn *)arg;
	request_args_t * req = (request_args_t *)conn->reverse;

	if (req->post_data == NULL) {
		HTTPCLIENT_DEBUG("All sent\n");
		os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
		wifi_state = 0;
	}
	else {
		// The headers were sent, now send the contents.
		HTTPCLIENT_DEBUG("Sending request body:%s\n",req->post_data);

//		if (req->secure)
//			espconn_secure_send(conn, (uint8_t *)req->post_data, req->post_size);
//		else
			espconn_send(conn, (uint8_t *)req->post_data, req->post_size);
		os_free(req->post_data);
		req->post_data = NULL;
	}
}

static void ICACHE_FLASH_ATTR connect_callback(void * arg)
{
    os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	HTTPCLIENT_DEBUG("Connected\n");
	struct espconn * conn = (struct espconn *)arg;
	request_args_t * req = (request_args_t *)conn->reverse;

	espconn_regist_recvcb(conn, receive_callback);
	espconn_regist_sentcb(conn, sent_callback);

	char post_headers[32];
	os_memset(post_headers, '\0', sizeof(post_headers));

	if (req->post_data != NULL) { // If there is data this is a POST request.
		os_sprintf(post_headers, "Content-Length: %d\r\n", req->post_size);
	}

	if (req->headers == NULL) /* Avoid NULL pointer, it may cause exception */
	{
		req->headers = (char *)os_malloc(sizeof(char));
		// Check memory enough
		if (req->headers != NULL)
			req->headers[0] = '\0';
		else
			return;
	}

	char header_ua[sizeof("User-Agent: " HTTP_USER_AGENT "\r\n")];
	os_memset(header_ua, '\0', sizeof(header_ua));
	if (os_strstr(req->headers, "User-Agent:") == NULL && os_strstr(req->headers, "user-agent:") == NULL)
	{
		os_sprintf(header_ua, "User-Agent: " HTTP_USER_AGENT "\r\n");
	}

	char header_host[sizeof("Host: %s:%d\r\n") + os_strlen(req->hostname) + os_strlen("65535")];
	os_memset(header_host, '\0', sizeof(header_host));
	if (os_strstr(req->headers, "Host:" ) == NULL && os_strstr( req->headers, "host:" ) == NULL)
	{
		if ((req->port == 80) || ((req->port == 443) && (req->secure)))
		{
			os_sprintf(header_host, "Host: %s\r\n", req->hostname);
		}
		else if ((req->port > 0) && (req->port < 65536))
		{
			os_sprintf(header_host, "Host: %s:%d\r\n", req->hostname, req->port);
		}
	}

	uint8 headers_str[]="Content-Type:application/json";
	os_memset(req->headers,'\0',os_strlen(req->headers));
	os_memcpy(req->headers,headers_str,os_strlen(headers_str));

	char buf[69 +
	         strlen(req->method) +
	         strlen(req->path) +
	         strlen(header_host) +
	         strlen(req->headers) +
	         strlen(header_ua) +
	         strlen(post_headers)];

	int len = os_sprintf(buf,
	                     "%s %s HTTP/1.1\r\n"
	                     "%s"                       // Host (if not provided in the headers from user app)
	                     "Connection: keep-alive\r\n"
	                     "%s\r\n"                   // Headers from user app (optional)
	                     "%s"                       // User-Agent (if not provided in the headers from app)
	                     "%s"                       // Content-Length
	                     "\r\n",
	                     req->method, req->path, header_host, headers_str, header_ua, post_headers);


	os_printf("header:\n%s\n sizeof[buf]:%d\n strlen buf:%d\n len:%d\n",buf,sizeof(buf),69 +
	             strlen(req->method) +
	             strlen(req->path) +
	             strlen(header_host) +
	             strlen(req->headers) +
	             strlen(header_ua) +
	             strlen(post_headers),len);


	os_printf("secure:%d\n",req->secure);
	if (req->secure)
		espconn_secure_send(conn, (uint8_t *)buf, len);
	else
		espconn_send(conn, (uint8_t *)buf, len);
	os_free(req->headers);
	req->headers = NULL;
	HTTPCLIENT_DEBUG("Sending request header\n");
	os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
}

static void ICACHE_FLASH_ATTR disconnect_callback(void * arg)
{
	HTTPCLIENT_DEBUG("Disconnected\n");
	struct espconn *conn = (struct espconn *)arg;

	if (conn == NULL) {
		// This is abnormal
		// Pass error response to user
		http_finish(NULL, "", HTTP_STATUS_GENERIC_ERROR, "", 0);
		return; // Exit
	}

	if (conn->proto.tcp != NULL) {
		os_free(conn->proto.tcp);
		conn->proto.tcp = NULL;
	}

	if (conn->reverse != NULL) {
		request_args_t * req = (request_args_t *)conn->reverse;
		int http_status = -1;
		int body_size = 0;
		const char empty_string[] = "";
		char * body = (char *)empty_string;
		if (req->buffer == NULL) {
			HTTPCLIENT_DEBUG("Buffer shouldn't be NULL\n");
		}
		else if (req->buffer[0] != '\0') {
			// FIXME: make sure this is not a partial response, using the Content-Length header.

			const char * version10 = "HTTP/1.0 ";
			const char * version11 = "HTTP/1.1 ";
			if (os_strncmp(req->buffer, version10, strlen(version10)) != 0
			        && os_strncmp(req->buffer, version11, strlen(version11)) != 0) {
				HTTPCLIENT_DEBUG("Invalid version in %s\n", req->buffer);
			}
			else {
				http_status = atoi(req->buffer + strlen(version10));
				/* find body and zero terminate headers */
				body = (char *)os_strstr(req->buffer, "\r\n\r\n");
				if (NULL == body) {
					/* Find NULL body */
					HTTPCLIENT_DEBUG("Body shouldn't be NULL\n");
					/* To avoid NULL body */
					body = (char *)empty_string;
					body_size = 0;
				} else {
					/* Skip CR & LF, and fill zero */
					body += 2;
					*body++ = '\0';
					*body++ = '\0';
					body_size = req->buffer_size - (body - req->buffer);
					if (os_strstr(req->buffer, "Transfer-Encoding: chunked"))
					{
						body_size = chunked_decode(body, body_size);
					}
				}
			}
		}

		// if (req->user_callback != NULL) { // Callback is optional.
		// 	req->user_callback(body, http_status, req->buffer, body_size);
		// }

		// Finish and response to user's callback
		http_finish(req, body, http_status, req->buffer, body_size);

		// os_free(req->buffer);
		// os_free(req->hostname);
		// os_free(req->method);
		// os_free(req->path);
		// os_free(req);

		// Safe free all arguments
		free_request_args(req);
		os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	}
	espconn_delete(conn);
	// Check conn and safe to free
	if (conn != NULL) {
		if (conn->proto.tcp != NULL) {
			os_free(conn->proto.tcp);
			conn->proto.tcp = NULL;
		}
		os_free(conn);
	}
}

static void ICACHE_FLASH_ATTR error_callback(void *arg, sint8 errType)
{
    disconnect_callback(arg);
    mac_inrom_flag=0;
    write_to_flash();
    scan_qz=0;
    connected_wifi=0;

    app_save(0);
    queue_uart_send(zgmode,os_strlen(zgmode));
    os_printf("send %s\n",zgmode);
    at_state = ZGMODE;
    wifi_bad++;
	//网络故障
	//get_rssi();
}

static void ICACHE_FLASH_ATTR dns_callback(const char * hostname, ip_addr_t * addr, void * arg)
{
	request_args_t * req = (request_args_t *)arg;
	os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	if (addr == NULL) {
		HTTPCLIENT_DEBUG("DNS failed for %s\n", hostname);
		// This is abnormal, call user callback function
		// if (req->user_callback != NULL) {
		// 	req->user_callback("", -1, "", 0);
		// }

		// Pass error response to user
		http_finish(req, "", HTTP_STATUS_GENERIC_ERROR, "", 0);
		// os_free(req->buffer);
		// os_free(req->post_data);
		// os_free(req->headers);
		// os_free(req->path);
		// os_free(req->hostname);
		// os_free(req);

		// Safe free all arguments
		free_request_args(req);
	}
	else {
		HTTPCLIENT_DEBUG("DNS found %s " IPSTR "\n", hostname, IP2STR(addr));

		struct espconn * conn = (struct espconn *)os_malloc(sizeof(struct espconn));
		conn->type = ESPCONN_TCP;
		conn->state = ESPCONN_NONE;
		conn->proto.tcp = (esp_tcp *)os_malloc(sizeof(esp_tcp));
		conn->proto.tcp->local_port = espconn_port();
		conn->proto.tcp->remote_port = req->port;
		conn->reverse = req;

		os_memcpy(conn->proto.tcp->remote_ip, addr, 4);

		espconn_regist_connectcb(conn, connect_callback);
		espconn_regist_disconcb(conn, disconnect_callback);
		espconn_regist_reconcb(conn, error_callback);

		if (req->secure) {
			espconn_secure_set_size(ESPCONN_CLIENT, 5120); // set SSL buffer size
			espconn_secure_connect(conn);
		} else {
			espconn_connect(conn);
		}
	}
	os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
}

static void ICACHE_FLASH_ATTR http_raw_request(void * ctx, const char * hostname, int port, bool secure, const char * method, const char * path, const char * headers, const char * post_data, size_t post_size, http_callback_t user_callback)
{
    os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	HTTPCLIENT_DEBUG("DNS request\n");

	request_args_t * req  = (request_args_t *) os_malloc(sizeof(request_args_t));
	req->hostname         = strdup(hostname);
	req->port             = port;
	req->secure           = secure;
	req->method           = strdup(method);
	req->path             = strdup(path);
	req->headers          = strdup(headers);

	if (NULL != post_data && post_size > 0) {
		req->post_data    = (char *)os_malloc(post_size);
		req->post_size    = post_size;
		os_memcpy(req->post_data, post_data, post_size);
	} else {
		req->post_data    = NULL;
		req->post_size    = 0;
	}

	req->buffer_size      = 1;
	req->buffer           = (char *)os_malloc(1);
	req->buffer[0]        = '\0'; // Empty string.                                        /* Empty string. */
	req->user_callback    = user_callback;
	req->ctx              = ctx;

	ip_addr_t addr;
	err_t error = espconn_gethostbyname((struct espconn *)req, // It seems we don't need a real espconn pointer here.
	                                    hostname, &addr, dns_callback);

	if (error == ESPCONN_INPROGRESS) {
		HTTPCLIENT_DEBUG("DNS pending\n");
	}
	else if (error == ESPCONN_OK) {
		// Already in the local names table (or hostname was an IP address), execute the callback ourselves.
		dns_callback(hostname, &addr, req);
	}
	else {
		if (error == ESPCONN_ARG) {
			HTTPCLIENT_DEBUG("DNS arg error %s\n", hostname);
		}
		else {
			HTTPCLIENT_DEBUG("DNS error code %d\n", error);
		}
		dns_callback(hostname, NULL, req); // Handle all DNS errors the same way.
	}
}

/*
 * Parse an URL of the form http://host:port/path
 * <host> can be a hostname or an IP address
 * <port> is optional
 */
int ICACHE_FLASH_ATTR http_request(void * ctx, const char * url, const char * method, const char * headers, const char * post_data, size_t post_size, http_callback_t user_callback)
{
    os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	/*
	 * FIXME: handle HTTP auth with http://user:pass@host/
	 * FIXME: get rid of the #anchor part if present.
	 */
    os_printf("url:%s\n",url);
	char    hostname[128]   = "";
	int     port            = 80;
	bool    secure          = false;

	bool    is_http         = os_strncmp(url, "http://", strlen("http://")) == 0;
	bool    is_https        = os_strncmp(url, "https://", strlen("https://")) == 0;

	if (is_http)
		url += strlen("http://");               /* Get rid of the protocol. */
	else if (is_https) {
		port    = 443;
		secure  = true;
		url     += strlen("https://");          /* Get rid of the protocol. */
	} else {
		HTTPCLIENT_DEBUG("URL is not HTTP or HTTPS %s\n", url);
		return REQUEST_ERROR_ARG;
	}

	char * path = os_strchr(url, '/');

	if (path == NULL) {
		path = os_strchr(url, '\0');    /* Pointer to end of string. */
	}

	char * colon = os_strchr(url, ':');

	if (colon > path) {
		colon = NULL;                   /* Limit the search to characters before the path. */
	}

	if (colon == NULL) {                    /* The port is not present. */
		os_memcpy(hostname, url, path - url);
		hostname[path - url] = '\0';
	} else {
		port = atoi(colon + 1);

		if (port == 0) {
			HTTPCLIENT_DEBUG("Port error %s\n", url);
			return REQUEST_ERROR_ARG;
		}

		os_memcpy(hostname, url, colon - url);
		hostname[colon - url] = '\0';
	}


	if (path[0] == '\0') { /* Empty path is not allowed. */
		path = "/";
	}

	HTTPCLIENT_DEBUG("hostname=%s\n", hostname);
	HTTPCLIENT_DEBUG("port=%d\n", port);
	HTTPCLIENT_DEBUG("method=%s\n", method);
	HTTPCLIENT_DEBUG("path=%s\n", path);

	os_printf("http_parallel %d\n",http_parallel);
	os_printf("HTTP_PARALLEL_MAX %d\n",HTTP_PARALLEL_MAX);
//	if (http_parallel < HTTP_PARALLEL_MAX) {
    if (1) {
	    os_printf("next http_raw_request\n");
		http_raw_request(ctx, hostname, port, secure, method, path, headers, post_data, post_size, user_callback);
		http_parallel++;
	}
	else
	{
		return REQUEST_ERROR_QUOTA;
	}
	return REQUEST_OK;
}

/*
 * Parse an URL of the form http://host:port/path
 * <host> can be a hostname or an IP address
 * <port> is optional
 */
int ICACHE_FLASH_ATTR http_post(void * ctx, const char * url, const char * headers, const char * post_data, size_t post_size, http_callback_t user_callback)
{
    os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
    os_printf("return http_request \n");
	return http_request(ctx, url, "POST", headers, post_data, post_size, user_callback);
}


int ICACHE_FLASH_ATTR http_get(void * ctx, const char * url, const char * headers, http_callback_t user_callback)
{
	return http_request(ctx, url, "GET", headers, NULL, 0, user_callback);
}


int ICACHE_FLASH_ATTR http_delete(void * ctx, const char * url, const char * headers, const char * post_data, size_t post_size, http_callback_t user_callback)
{
	return http_request(ctx, url, "DELETE", headers, post_data, post_size, user_callback);
}


int ICACHE_FLASH_ATTR http_put(void * ctx, const char * url, const char * headers, const char * post_data, size_t post_size, http_callback_t user_callback)
{
	return http_request(ctx, url, "PUT", headers, post_data, post_size, user_callback);
}


