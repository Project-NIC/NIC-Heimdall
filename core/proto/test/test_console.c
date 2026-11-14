/* The head's console and first boot on a PC: K0 made by the TRNG at the first boot and only then;
 * a failing TRNG; `k0` answered once, refused after, and still refused after a reboot; a key store
 * that will not mark it read showing nothing; `id`; the readers with their arguments; `card out`
 * and its misuse; an unknown command; lines ended by CR LF, CR or LF, an empty one, one too long. */
#include "../nic_console.h"

#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static uint8_t store_k0[8], store_have, store_read, store_refuses, trng_fails;
static char said[2048];
static char last_arg[64];
static int calls, card_slot = -1;

static int k0_load(void *x, uint8_t k0[8], uint8_t *r) { (void)x; if (!store_have) return -1; memcpy(k0, store_k0, 8); *r = store_read; return 0; }
static int k0_store(void *x, const uint8_t k0[8], uint8_t r)
{
    (void)x;
    if (store_refuses) return -1;
    memcpy(store_k0, k0, 8); store_have = 1; store_read = r;
    return 0;
}
static int trng(void *x, uint8_t *b, size_t n) { (void)x; if (trng_fails) return -1; for (size_t i = 0; i < n; i++) b[i] = (uint8_t)(0xA0 + i); return 0; }
static void say(void *x, const char *l) { (void)x; strcat(said, l); strcat(said, "|"); }
static int h_mirror(void *x, const char *a) { (void)x; calls++; strcpy(last_arg, a); say(0, "row 0"); return 0; }
static int h_tree(void *x) { (void)x; calls++; return 0; }
static int h_log(void *x, const char *a) { (void)x; calls++; strcpy(last_arg, a); return 0; }
static int h_tail(void *x, const char *a) { (void)x; calls++; strcpy(last_arg, a); return -1; }
static int h_hermes(void *x) { (void)x; calls++; return 0; }
static int h_card(void *x, int s) { (void)x; card_slot = s; return 0; }

static const nic_con_io IO = {k0_load, k0_store, trng, say, h_mirror, h_tree, h_log, h_tail, h_hermes, h_card, 0};
static nic_console c;

static const char *run(const char *in)
{
    said[0] = 0;
    nic_con_bytes(&c, (const uint8_t *)in, strlen(in));
    return said;
}

int main(void)
{
    /* the first boot */
    trng_fails = 1;
    CHECK(nic_con_first_boot(&IO) == -1 && !store_have);
    trng_fails = 0;
    nic_con_init(&c, &IO, 0x123456);
    CHECK(!strcmp(run("k0\n"), "error no K0|"));
    CHECK(nic_con_first_boot(&IO) == 1 && store_have && store_k0[0] == 0xA0 && !store_read);
    CHECK(nic_con_first_boot(&IO) == 0);                       /* the second boot makes none */

    /* K0 once */
    store_refuses = 1;
    CHECK(!strcmp(run("k0\n"), "error the key store refused|"));
    store_refuses = 0;
    CHECK(!strcmp(run("k0\r\n"), "k0 a0a1a2a3a4a5a6a7|ok|") && store_read);
    CHECK(!strncmp(run("k0\n"), "error K0 has left the board once", 32));
    nic_con_init(&c, &IO, 0x123456);                           /* a reboot */
    CHECK(!strncmp(run("k0\n"), "error K0 has left the board once", 32));

    /* the rest */
    CHECK(!strcmp(run("id\n"), "id 123456|ok|"));
    CHECK(!strcmp(run("mirror   52 0013\n"), "row 0|ok|") && !strcmp(last_arg, "52 0013"));
    CHECK(!strcmp(run("tree\rlog 100\r"), "ok|ok|") && !strcmp(last_arg, "100"));
    CHECK(!strcmp(run("tail 52\n"), "error tail|"));
    CHECK(!strcmp(run("hermes\n"), "ok|") && calls == 5);
    CHECK(!strcmp(run("card out 1\n"), "ok|") && card_slot == 1);
    CHECK(!strcmp(run("card out 7\n"), "error usage: card out 0|1|"));
    CHECK(!strcmp(run("card in 0\n"), "error usage: card out 0|1|"));
    CHECK(!strcmp(run("selfdestruct\n"), "error unknown command|"));
    CHECK(!strcmp(run("\n\r\n   \n"), ""));                  /* empty lines say nothing */
    char longl[300];
    memset(longl, 'x', 200);
    longl[200] = '\n'; longl[201] = 0;
    CHECK(!strcmp(run(longl), "error line too long|"));
    CHECK(!strcmp(run("id\n"), "id 123456|ok|"));             /* and the next line is whole */

    if (fails) {
        printf("test_console: %d FAILED\n", fails);
        return 1;
    }
    printf("test_console: all passed\n");
    return 0;
}
