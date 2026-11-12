/* The station tree: three cards' tables and an Argus's composed, the numbering cases of
 * ../../mayak/FIRMWARE.md §7 one by one, the roster written from the tree. */
#include <stdio.h>
#include <string.h>

#include "../nic_sweep.h"
#include "../nic_tree.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static void row(uint8_t *t, int i, uint8_t addr, uint8_t port)
{
    t[4 * i] = addr;
    t[4 * i + 1] = port;
    t[4 * i + 2] = 3;          /* a rung code */
    t[4 * i + 3] = 0;
}

static const nic_decision *of(const nic_tree *t, const nic_decision *d, uint8_t card, uint8_t argus,
                              uint8_t slot)
{
    for (uint16_t i = 0; i < t->n; i++)
        if (t->u[i].card == card && t->u[i].argus == argus && t->u[i].slot == slot)
            return &d[i];
    return NULL;
}

int main(void)
{
    /* card 0x21: Quake #1, Tesla #5, an Argus on #1 and #2 (six sondes); card 0x22: a Tesla #5
     * from another station, a fresh Quake, a Photon moved here as #4; card 0x23: a Positron #4
     * nobody knows. The Argus: Gauss #1, five fresh Gauss on the default. */
    uint8_t c1[32] = {0}, c2[32] = {0}, c3[32] = {0}, a1[32] = {0};
    row(c1, 0, 0x51, 1); row(c1, 1, 0x75, 2); row(c1, 2, 0x31, 3); row(c1, 3, 0x32, 3);
    row(c2, 0, 0x75, 1); row(c2, 1, 0x5F, 1); row(c2, 2, 0x94, 4);
    row(c3, 0, 0xB4, 2);
    row(a1, 0, 0x11, 1);
    for (int i = 1; i < 6; i++)
        row(a1, i, 0x1F, (uint8_t)(1 + i % 4));
    nic_tree t;
    nic_tree_init(&t);
    CHECK(nic_tree_card(&t, 0x21, c1) == 0 && nic_tree_card(&t, 0x22, c2) == 0);
    CHECK(nic_tree_card(&t, 0x23, c3) == 0 && nic_tree_argus(&t, 0x21, 0x31, a1) == 0);
    CHECK(t.n == 4 + 3 + 1 + 6);
    CHECK(nic_tree_card(&t, 0x23, c3) == 0 && t.n == 14);              /* sent again: replaced */
    /* the tags: the Argus one box on two NUMBERs */
    CHECK(nic_tree_tags(&t, 0x21, 0, 1, 0xA001, 0xB001) == 0);         /* Quake #1 */
    CHECK(nic_tree_tags(&t, 0x21, 0, 2, 0xA002, 0xB002) == 0);         /* Tesla #5 */
    CHECK(nic_tree_tags(&t, 0x21, 0, 3, 0xA003, 0xB003) == 0);         /* the Argus */
    CHECK(nic_tree_tags(&t, 0x21, 0, 4, 0xA003, 0xB003) == 0);
    CHECK(nic_tree_tags(&t, 0x22, 0, 1, 0xA004, 0xB004) == 0);         /* the other Tesla */
    CHECK(nic_tree_tags(&t, 0x22, 0, 2, 0xA005, 0xB005) == 0);         /* the fresh Quake */
    CHECK(nic_tree_tags(&t, 0x22, 0, 3, 0xA006, 0xB006) == 0);         /* the Photon */
    CHECK(nic_tree_tags(&t, 0x23, 0, 1, 0xA007, 0xB007) == 0);         /* the Positron */
    for (uint8_t i = 0; i < 6; i++)
        CHECK(nic_tree_tags(&t, 0x21, 0x31, i, (uint16_t)(0xC000 + i), (uint16_t)(0xD000 + i)) == 0);
    CHECK(nic_tree_tags(&t, 0x21, 0x31, 6, 1, 1) == -1);
    /* positions: #1's are rows 0–3, #2's rows 4–7 */
    const nic_seen *s = nic_tree_sonde(&t, 0x21, 0x31, 2);
    CHECK(s && s->argus == 0x31 && s->slot == 2);
    s = nic_tree_sonde(&t, 0x21, 0x32, 1);
    CHECK(s && s->slot == 5 && nic_tree_sonde(&t, 0x21, 0x32, 2) == NULL);
    CHECK(nic_tree_sonde(&t, 0x21, 0x51, 0) == NULL && nic_tree_sonde(&t, 0x21, 0x31, 4) == NULL);

    /* the service sweep against the mirror */
    nic_mirror_row m[] = {
        {NIC_NODBUS, 0x51, 0xA001, 0xB001},     /* Quake #1, here */
        {NIC_NODBUS, 0x52, 0xA0F0, 0xB0F0},     /* Quake #2, gone */
        {NIC_NODBUS, 0x75, 0xA002, 0xB002},     /* Tesla #5 */
        {NIC_NODBUS, 0x31, 0xA003, 0xB003},     /* the Argus's two NUMBERs */
        {NIC_NODBUS, 0x32, 0xA003, 0xB003},
        {NIC_NODBUS, 0x93, 0xA006, 0xB006},     /* the Photon as #3 */
        {NIC_MINI, 0x11, 0xC000, 0xD000},       /* Gauss #1 */
        {NIC_MINI, 0x13, 0xC0F0, 0xD0F0},       /* Gauss #3, gone */
    };
    nic_decision d[NIC_TREE_MAX];
    uint8_t missing[NIC_TREE_MAX];
    CHECK(nic_tree_number(&t, m, 8, 1, d, missing) == 14);
    const nic_decision *x;
    CHECK((x = of(&t, d, 0x21, 0, 1)) && x->verdict == NIC_V_CONFIRMED && x->to == 0x51);
    CHECK((x = of(&t, d, 0x21, 0, 2)) && x->verdict == NIC_V_CONFIRMED);
    CHECK((x = of(&t, d, 0x21, 0, 3)) && x->verdict == NIC_V_CONFIRMED && x->to == 0x31);
    CHECK((x = of(&t, d, 0x21, 0, 4)) && x->verdict == NIC_V_CONFIRMED && x->to == 0x32);
    CHECK((x = of(&t, d, 0x22, 0, 1)) && x->verdict == NIC_V_RENUMBERED && x->to == 0x71 && !x->ask);
    CHECK((x = of(&t, d, 0x22, 0, 2)) && x->verdict == NIC_V_REPLACED && x->to == 0x52 && x->ask);
    CHECK((x = of(&t, d, 0x22, 0, 3)) && x->verdict == NIC_V_MOVED && x->to == 0x93 && !x->ask);
    CHECK((x = of(&t, d, 0x23, 0, 1)) && x->verdict == NIC_V_STRANGER && x->to == 0xB1 && !x->ask);
    CHECK((x = of(&t, d, 0x21, 0x31, 0)) && x->verdict == NIC_V_CONFIRMED);
    CHECK((x = of(&t, d, 0x21, 0x31, 1)) && x->verdict == NIC_V_REPLACED && x->to == 0x13 && x->ask);
    CHECK((x = of(&t, d, 0x21, 0x31, 2)) && x->verdict == NIC_V_NEW && x->to == 0x12 && x->ask);
    CHECK((x = of(&t, d, 0x21, 0x31, 5)) && x->verdict == NIC_V_NEW && x->to == 0x16);
    CHECK(missing[1] && missing[7] && !missing[0] && !missing[3] && !missing[4] && !missing[5]);

    /* the ASSIGN_ADDR goes by both tags, on the row's slot; the unit takes it */
    nic_sweep_unit u;
    uint8_t uid[12] = {0}, f[NIC_CTRL_LEN], ack[NIC_CTRL_LEN];
    nic_sweep_unit_init(&u, uid, 0x75, 0);
    nic_tree_tags(&t, 0x22, 0, 1, u.tag, u.tag2);
    x = of(&t, d, 0x22, 0, 1);
    nic_tree_assign(&t, x, f);
    CHECK(nic_sweep_unit_assign(&u, f, ack) == 1 && u.addr == 0x71 && u.slot == 1);

    /* applied — the human said yes to every question — and the roster written from the tree */
    for (uint16_t i = 0; i < t.n; i++)
        nic_tree_apply(&t, &d[i]);
    nic_roster r;
    nic_tree_roster(&t, &r);
    uint16_t card = nic_roster_find(&r, NIC_NODBUS, 0x21), ar2 = nic_roster_find(&r, NIC_NODBUS, 0x32);
    CHECK(card != NIC_NONE && r.rows[card].parent == NIC_NONE);
    CHECK(r.rows[nic_roster_find(&r, NIC_NODBUS, 0x51)].parent == card);
    CHECK(nic_roster_find(&r, NIC_NODBUS, 0x71) != NIC_NONE && nic_roster_find(&r, NIC_NODBUS, 0x93) != NIC_NONE);
    uint16_t g = nic_roster_find(&r, NIC_MINI, 0x11);
    CHECK(g != NIC_NONE && r.rows[g].parent == nic_roster_find(&r, NIC_NODBUS, 0x31) && r.rows[g].slot == 0);
    g = nic_roster_find(&r, NIC_MINI, 0x16);
    CHECK(g != NIC_NONE && ar2 != NIC_NONE && r.rows[g].parent == ar2 && r.rows[g].slot == 1);
    CHECK(r.n == 3 + 8 + 6);

    /* the initial sweep: an empty mirror, everything new from the bottom — two Teslas #5 leave
     * as #1 and #2 with nobody at the site */
    nic_tree t2;
    uint8_t c4[32] = {0};
    row(c4, 0, 0x75, 1); row(c4, 1, 0x75, 2); row(c4, 2, 0x5F, 3);
    nic_tree_init(&t2);
    nic_tree_card(&t2, 0x21, c4);
    nic_tree_tags(&t2, 0x21, 0, 1, 1, 2);
    nic_tree_tags(&t2, 0x21, 0, 2, 3, 4);
    CHECK(nic_tree_number(&t2, m, 8, 0, d, missing) == 3);
    CHECK(d[0].verdict == NIC_V_NEW && d[0].to == 0x71 && d[1].to == 0x72 && !d[0].ask);
    CHECK(d[2].verdict == NIC_V_UNTAGGED);
    /* fourteen of a type and no more */
    nic_tree t3;
    uint8_t c5[32] = {0};
    nic_mirror_row full[14];
    for (int i = 0; i < 14; i++)
        full[i] = (nic_mirror_row){NIC_NODBUS, (uint8_t)(0x71 + i), (uint16_t)i, 0};
    row(c5, 0, 0x75, 1);                                               /* a stranger */
    nic_tree_init(&t3);
    nic_tree_card(&t3, 0x21, c5);
    nic_tree_tags(&t3, 0x21, 0, 1, 0xEEEE, 0xEEEE);
    CHECK(nic_tree_number(&t3, full, 14, 1, d, missing) == 1 && d[0].verdict == NIC_V_FULL);
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the station tree: every check passed\n");
    return 0;
}
