#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "regexp.h"

char* re2post(char* re)
{
    int num_alt = 0, num_char = 0;
    static char buf[8000];
    char *dst;
    struct {
        int num_alt;
        int num_char;
    }paren_stack[MAX_PARAN_NEST_DEPTH], *cur_level;

    cur_level = paren_stack;
    dst = buf;
    num_alt = 0;
    num_char = 0;

    if (strlen(re) >= sizeof buf/2)
        return NULL;

    for (; *re ; re++) {
        switch(*re){
            case '(':
                if (num_char > 1){
                    *dst++ = '.';
                    num_char --;
                }
                if ( cur_level >= paren_stack + MAX_PARAN_NEST_DEPTH )
                    return NULL;
                cur_level->num_alt = num_alt;
                cur_level->num_char = num_char;
                cur_level ++;
                num_alt = 0;
                num_char = 0;
                break;
            case ')':
                if (cur_level == paren_stack || num_char == 0)
                    return NULL;
                if (num_char > 1){
                    *dst ++ = '.';
                    num_char --;
                }
                if (num_alt > 0){
                    *dst++ = '|';
                    num_alt --;
                }
                cur_level --;
                num_alt = cur_level->num_alt;
                num_char = cur_level->num_char;
                num_char ++;
                break;
            case '|':
                if (num_char == 0)
                    return NULL;
                if (num_char > 1)
                    *dst++ = '.';
                num_alt ++;
                break;
            case '+':
            case '*':
            case '?':
                if (num_char == 0)
                        return NULL;
                *dst++ = *re;
                break;
            default:
                if (num_char > 1){
                    *dst++ = '.';
                    num_char --;
                }
                *dst++ = *re;
                num_char ++;
                break;
        }
    }
    if (cur_level != paren_stack)
        return NULL;
    if (num_char > 1){
        *dst++ = '.';
    }
    if (num_alt > 0){
        *dst++ = '|';
    }
    *dst = '\0';
    return buf;
}

State matchstate = { Match };
int nstate;

State* state(int type, State* out1, State* out2)
{

    State* s = malloc(sizeof *s);

    nstate++;
    s->lastlist = 0;
    s->type = type;
    s->out1 = out1;
    s->out2 = out2;
    return s;
}

Frag frag(State* start, Statelist *path)
{
    Frag n ={start, path};
    return n;
}

Statelist* list(State **Pathlist)
{
    Statelist *l;
    l = (Statelist*)Pathlist;
    l->next = NULL;
    return l;
}

void patch(Statelist *l, State *s)
{
    Statelist *next;
    for (; l; l = next){
        next = l->next;
        l->s = s;
    }
}

Statelist* append(Statelist *l1, Statelist *l2)
{
    Statelist *ptr;

    ptr = l1;
    while(ptr->next)
        ptr = ptr->next;
    ptr->next = l2;
    return l1;
}

State* post2nfa(char* postfix)
{
    char *p;
    Frag stack[1000], *stackp, e1, e2, e;
    State* s;

    if (postfix == NULL)
        return NULL;

    #define push(s) *stackp++ = s
    #define pop() *--stackp

    stackp = stack;
    for (p = postfix; *p; p++){
        switch(*p){
            case '.':
                e2 = pop();
                e1 = pop();
                patch(e1.out, e2.start);
                push(frag(e1.start, e2.out));
                break;
            case '|':
                e2 = pop();
                e1 = pop();
                s = state(Split, e1.start, e2.start);
                push(frag(s, append(e1.out, e2.out)));
                break;
            case '*':
                e = pop();
                s = state(Split, e.start, NULL);
                patch(e.out, s);
                push(frag(s, list(&s->out2)));
                break;
            case '+':
                e = pop();
                s = state(Split, e.start, NULL);
                patch(e.out, s);
                push(frag(e.start, list(&s->out2)));
                break;
            default:
                s = state(*p, NULL, NULL);
                push(frag(s, list(&s->out1)));
                break;
        }
    }
    e = pop();
    if(stackp != stack)
		return NULL;
	patch(e.out, &matchstate);
	return e.start;
#undef pop
#undef push
}

List* startlist(State* start, List *l)
{
    l->n = 0;
    listid++;
    addstate(l, start);
    return l;
}

int ismatch(List *l)
{
    int i;
    for(i = 0; i < l->n; i++){
        if (l->s[i] == &matchstate)
            return 1;
    }
    return 0;
}

void addstate(List* l, State* s)
{
    if (s == NULL || s->lastlist == listid)
        return;

    s->lastlist = listid;
    if (s->type == Split){
        addstate(l, s->out1);
        addstate(l, s->out2);
        return;
    }
    l->s[l->n++] = s;
}

void step(List* clist, int type, List* nlist)
{
    int i;
    State* s;

    listid++;
    nlist->n = 0;
    for (i = 0; i < clist->n; i++){
        s = clist->s[i];
        if (s->type == type)
            addstate(nlist, s->out1);
    }
}

int match(State* start, char* s)
{
    int i, type;
    List* clist, *nlist, *t;

    clist = startlist(start, &l1);
    nlist = &l2;
    for (; *s; s++){
        type = *s & 0xFF;
        step(clist, type, nlist);
        t = clist; clist = nlist; nlist = t;
    }
    return ismatch(clist);
}

int main(int argc, char** argv ){
    char* post = re2post(argv[1]);
    State* start;
    if (post == NULL){
        printf("bad regexp %s\n", argv[1]);
        return 1;
    }
    //printf("postexp: %s\n", post);
    start = post2nfa(post);
    if (start == NULL){
        printf("Error in post2nfa %s\n", post);
        return 1;
    }

    l1.s = malloc(nstate * sizeof l1.s[0]);
    l2.s = malloc(nstate * sizeof l2.s[0]);
    if (match(start, argv[2]))
        printf("Matched !! %s\n", argv[2]);


    return 0;
}
