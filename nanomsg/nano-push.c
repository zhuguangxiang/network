#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>

struct {
  int count;
  int verbose;
  int nn_socket;
  char *buf;
  int len;
  time_t now;
  char *remote;
} CF = {
  .remote = "tcp://127.0.0.1:9995",
  .nn_socket = -1,
  .count = 10,
};

void usage(char *prog) {
  fprintf(stderr, "usage: %s [-v] [-i <iterations>] <remote> ...\n", prog);
  fprintf(stderr, "  <remote> is a nano endpoint (e.g., tcp://127.0.0.1:1234)\n");
  exit(-1);
}

int main(int argc, char *argv[]) {
  int eid, opt, rc=0;

  while ( (opt = getopt(argc, argv, "v+i:")) != -1) {
    switch (opt) {
      case 'v': CF.verbose++; break;
      case 'i': CF.count = atoi(optarg); break;
      default: usage(argv[0]); break;
    }
  }

  rc = CF.nn_socket = nn_socket(AF_SP, NN_PUSH);
  if (rc < 0) goto done;

  do {
    char *remote =  (optind < argc) ? argv[optind++] : CF.remote;
    eid = nn_connect(CF.nn_socket, remote);
    if ( (rc = eid) < 0) goto done;
  } while(optind < argc);

  while (CF.count--) {
    CF.now = time(NULL);
    CF.buf = asctime(localtime(&CF.now));
    CF.len = strlen(CF.buf);
    rc = nn_send(CF.nn_socket, CF.buf, CF.len, 0);
    if (rc < 0) goto done;
    sleep(10);
  }

  rc = 0;

 done:
  if (rc < 0) fprintf(stderr,"nano: %s\n", nn_strerror(errno));
  if (CF.nn_socket >= 0) nn_close(CF.nn_socket);
  return 0;
}

