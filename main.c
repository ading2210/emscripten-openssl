#include <stdio.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

//this c code places an https request using openssl
//compile with the following command:
//emcc main.c -lssl -lcrypto -I openssl-wasm/include/ -L openssl-wasm/lib/
//or:
//gcc main.c -lssl -lcrypto

void init_ssl() {
  SSL_library_init();
  SSL_load_error_strings();
}

void do_cleanup(SSL_CTX* ctx, BIO* bio) {
  SSL_CTX_free(ctx);
  BIO_free_all(bio);
}

void do_connect(const char* host, const char* port) {
  char hostname[1024];
  char response[1024];
  char request[1024];

  sprintf(hostname, "%s:%s", host, port); 
  printf("trying to connect to %s\n", hostname);

  printf("creating method\n");
  const SSL_METHOD* method = SSLv23_client_method();
  if (method == NULL) {
    printf("bad method\n"); return;
  } 

  printf("creating ctx\n");
  SSL_CTX* ctx = SSL_CTX_new(method);
  if (ctx == NULL) {
    printf("bad ctx\n"); return;
  }

  printf("creating bio\n");
  BIO* bio = BIO_new_ssl_connect(ctx);
  if (bio == NULL) {
    printf("bad bio\n"); return;
  }

  printf("setting hostname\n");
  BIO_set_conn_hostname(bio, hostname);

  printf("connecting\n");
  int ret = BIO_do_connect(bio);
  if (ret <= 0) {
    printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
    printf("connection failed with code %i\n", ret); return;
  }
  else {
    printf("connection success\n");
  }

  sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host); 
  ret = BIO_write(bio, request, strlen(request));
  if (ret <= 0) {
    printf("write failed\n"); return;
  }

  while (1) {
    int size = BIO_read(bio, response, 1024);
    if (size <= 0) {
      break;
    }
    response[size] = 0;
    printf("%s", response);
  }

  do_cleanup(ctx, bio);
}

int main() {
  init_ssl();
  printf("OpenSSL version %ld\n", OPENSSL_VERSION_NUMBER);
  do_connect("www.google.com", "443");
}