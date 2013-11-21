#ifndef REGEXP_HEADER
#define  REGEXPHEADER

#define MAX_PARAN_NEST_DEPTH 100

enum
{
    Match = 256,
    Split = 257,
    AnyWord = 258,
    AnyDigit = 259
};

typedef struct State State;
struct State
{
    int type;
    State *out1;
    State *out2;
    int lastlist;
};

typedef union Statelist Statelist;
union Statelist
{
    Statelist *next;
    State *s;
};

typedef struct Frag Frag;
struct Frag
{
    State *start;
    Statelist *out;
};


typedef struct List List;
struct List
{
    State **s;
    int n;
};

char* re2post(char* re);
State* state(int type, State* out1, State* out2);
Frag frag(State* start, Statelist *path);
Statelist* list(State **Pathlist);
void patch(Statelist *l, State *s);
Statelist* append(Statelist *l1, Statelist *l2);
State* post2nfa(char* postfix);
List* startlist(State* start, List *l);
int ismatch(List *l);
void addstate(List* l, State* s);
void step(List* clist, int type, List* nlist);
int match(State* start, char* s);
int isAlpha(int type);
int isDight(int type);
int isUnderscore(int type);

List l1, l2;
static int listid;


#endif
