/*
 * CGI program to generate random bytes of a length specified
 * by the Range:  header in the HTTP request that invoked this
 */
/* Mark Moraes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

/**** Downloaded 20210303 from https://xorshift.di.unimi.it/splitmix64.c ****/
/*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

/* This is a fixed-increment version of Java 8's SplittableRandom generator
   See http://dx.doi.org/10.1145/2714064.2660195 and 
   http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html

   It is a very fast generator passing BigCrush, and it can be useful if
   for some reason you absolutely want 64 bits of state. */

static uint64_t x; /* The state can be seeded with any value. */

static inline uint64_t next() {
    uint64_t z = (x += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}
/**** End of https://xorshift.di.unimi.it/splitmix64.c ****/

int main(int argc, char **argv)
{
    uint64_t n0, n1, n, i, nb, wb = 0;
    const char *cp = getenv("HTTP_RANGE");
    const char *timing = getenv("GENRANDOM_TIMING");

    if (sizeof(uint64_t) != 8) {
	/* check we are dealing with 64-bits! */
	printf("Content-Type: text/plain\n\nError: sizeof(uint64_t) %zu != 8.\n", sizeof(uint64_t));
	exit(0);
    } else if (cp == NULL) {
	/* empty doc */
	printf("Content-Type: text/plain\n\nNo Range header provided\n");
	exit(0);
    }
    /* CGI with Range header */
    sscanf(cp, "bytes=%" SCNu64 "-%" SCNu64 "\n", &n0, &n1);
    if (n1 > n0) 
        n = (n1 - n0) + 1;
    else
        n = (n0 - n1) + 1;

    if (timing)
	printf("Content-Type: text/plain\n\n");
    else
	printf("Content-Type: application/octet-stream\nCache-Control: no-store\nContent-Length: %" PRIu64 "\n\n", n);

    nb = sizeof(uint64_t);
    for (i = 0; i < n; i += nb) {
	uint64_t v = next();
	if (i + nb > n)
	    nb = n - i;

	if (timing) {
	    wb ^= v;
	} if ((wb = fwrite((void *)&v, 1, nb, stdout)) != nb) {
	    fprintf(stderr, "%s: Error: fwrite returned %" PRIu64 " (expected %" PRIu64 ")\n",
		    argv[0], wb, nb);
	    return 0; /* no point upsetting httpd by returning 1? */
	};
    }
    if (timing)
	printf("i=%" PRIu64 " wb=%" PRIu64 "\n", i,wb);
    if (fflush(stdout) != 0 || ferror(stdout)) {
	    fprintf(stderr, "%s: Error: fflush or ferror failed: %s\n",
		    argv[0], strerror(errno));
	    return 0; /* no point upsetting httpd by returning 1? */
    }
    return 0;
}

