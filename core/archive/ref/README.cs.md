★ N.I.C. ★

# Referenční kód archivu — Python a C

[English](README.md) · **Čeština** · [Русский](README.ru.md)

Referenční knihovna, kterou jmenuje `MLF.md`, v Pythonu a v C: MLF a NRC tak, jak je určují `MLF.md`
a `NRC.md`, obě zapisují tytéž bajty, a měření, které rozhoduje o jeho kompresních poměrech
proti Steim-2 na zaznamenaných řadách. Nic z toho zatím neběží v centrále; centrála bude zapisovat přes C.

**Jen profily Heimdallu, na jádrech z NIC-SDK**: kontejner MLF a kodér řad NRC jsou v NIC-SDK
(`tools/mlf.py`, `tools/nrc.py`, `mlf/`, `nrc/`), hledané v klonu NIC-SDK vedle tohoto repozitáře
nebo tam, kam ukazuje `NIC_SDK`; samotná jádra testuje `tests/test_mlf.py` v NIC-SDK.

| soubor | co |
|---|---|
| `hnrc.py` | kodér a dekodér NRC: bitový proud, pět režimů, Riceovy oddíly, referenční kanál, rozvržení složené z polí a z bajtů, které žádné pole nepokrývá, rozvržení NUMBER Argusu z jeho pozic |
| `hmlf.py` | MLF v paměti: hlavička a její sekce, z `LAYOUT` a `UNIT` rozvržení, kterým NRC kóduje, segmenty zakódované surově i přes NRC a zapsaný ten kratší, konfigurace, rámování COBS, záznamy popisu a binární hledání v nich, deník událostí, cesta, pravidla záznamu a odesílání nad rámci a ModBus bloky; čtečka, která při ztrátě popisu najde objekty souboru podle nul |
| `session.py` | zprávy odesílacího sezení podle `../../PROTOCOL.md` §6 — HELLO, CURSOR, ARCHIVE se zřetězeným tagem, ACK, LIVE, WINDOW — výřez souboru od kurzoru a pro okno a soubor sestavený serverem z paketů |
| `sdk.py` | kde se najdou jádra NIC-SDK — vedle tohoto repozitáře nebo v `NIC_SDK`; `hnrc.py` a `hmlf.py` ho importují první |
| `tests/test_hnrc.py` | případy, které uvádí `NRC.md`, na syntetických záznamech; každý projde tam a zpět bajt po bajtu |
| `tests/test_damaged.py` | proudy zkrácené, prodloužené, s převrácenými bity, přepsané a vymyšlené: každý je buď odmítnut s `ValueError`, nebo se dekóduje, nikdy nespadne ani se nezasekne; zkrácený nebo prodloužený se vždy čte jako poškozený. `NRC_FUZZ_SEED` a `NRC_FUZZ_ROUNDS` test rozšiřují |
| `tests/test_hmlf.py` | MLF na syntetických souborech: Quake, kterému se uprostřed souboru změní rozvržení, bloky Palatine, NUMBER Argusu, deník událostí, soubor zaplněný do konce — každý přečtený zpět záznam po záznamu; poškozený objekt přeskočen, ztracený popis obnoven; co které pravidlo ponechá |
| `tests/test_session.py` | každý testovací soubor poslaný celý i od kurzoru a sestavený zpět na bajty karty; podvržený, vynechaný, přehozený i přehraný paket odmítnut; výřez okna; každá zpráva přečtena zpět |
| `c/hnrc.h` · `c/hnrc.c` | tentýž kodér a dekodér v C99, závislý jen na malloc a free; překládá se s `-ffp-contract=off`, aby prediktor zaokrouhloval každou operaci v double jako Python, a autokorelaci sčítá přesně ve 128 bitech |
| `c/hmlf.h` · `c/hmlf.c` | tentýž kontejner v C99 nad `hnrc.c`: sestavení objektů, zapisovač, který drží soubor v paměti a přidává do něj jako centrála, čtečka, pravidla |
| `c/session.h` · `c/session.c` | totéž sezení v C99 s vlastním SHA-256 a HMAC |
| `c/hnrc_tool.c` · `c/Makefile` | C ovládané přes stdin a stdout, pro `tests/test_c.py` |
| `tests/test_c.py` | C proti Pythonu: zprávy sezení, HMAC proti RFC 4231, pakety ARCHIVE ověřené oběma, výřezy; proudy rámců přes pravidlo záznamu a pravidlo odesílání; soubory MLF zapsané oběma a porovnané bajt po bajtu a oběma přečtené celé i s převrácenými bity; všechny případy NRC výše a náhodné segmenty — každá šířka pole, obě pořadí bajtů, referenční kanály, krajní a přetékající hodnoty, sekundy kdekoli ve 32 bitech — zakódované oběma a porovnané bajt po bajtu, každý proud přečtený zpět v C a každý poškozený proud posouzený oběma stejně, se stejnými záznamy. `NRC_C_SEED` a `NRC_C_ROUNDS` ho rozšíří |

Exportéry, které tyto soubory čtou až dodatečně, jsou v [`../export/`](../export/).

```
python3 tests/test_hnrc.py
python3 tests/test_damaged.py
python3 tests/test_hmlf.py
python3 tests/test_session.py
python3 tests/test_c.py                 # builds c/ with $CC, which may carry flags
NRC_RUN=qemu-arm CC="arm-linux-gnueabihf-gcc -static" python3 tests/test_c.py   # on a 32-bit ARM
```

**NRC se měří na otevřených záznamech** nástrojem `tools/nrcmeasure.py` z NIC-SDK, vedle něj Steim-2;
jak se řady stahují, říká README NIC-SDK.
