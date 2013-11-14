#ifndef REGEXP_HEADER
#define  REGEXPHEADER

#define MAX_PARAN_NEST_DEPTH 100

enum
{
    Match = 256,
    Split = 257
};

typedef struct State State;
struct State
{
    int type;
    State *out1;
    State *out2;
    int lastlist;
};

typedef struct Frag Frag;
typedef union Statelist Statelist;
struct Frag
{
    State *start;
    Statelist *out;
};

union Statelist
{
    Statelist *next;
    State *s;
};

#endif
