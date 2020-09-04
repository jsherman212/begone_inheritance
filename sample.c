#include "sample.h"

static void func1(void){
    struct s *s;
    s->f.parent_a = 4;
    s->f.f.pp_a = 99;
    s->f.f.pp_f = 1.0f;
    s->f.f.pp_d = 1.0;
    s->a = 4;
    s->f.parent_float = 1.0f;
    s->f.parent_d = -1.0;
    s->b = 100;
    s->f.r->r_z = 2.0f;
    s->f.r->r_d = 2.0;
    s->f.r->f.r_parent_f = 1.0f;
    s->f.r->f.r_parent_d = 1.0;
    s->c = (void *)0x41414141;
    s->f.r->x = 99;
    s->f.r->f.py = 11;
    s->d = 1.0f;
    s->e = 1.0;
}

static void func2(void){
    struct s *s;
    s->f.parent_b = 4;
    s->f.f.pp_b = 99;
    s->b = 32;
    s->c = (void *)0x111111;
}

int main(int argc, char **argv, const char **envp){
    func1();
    func2();
    return 0;
}
