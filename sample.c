struct r_parent {
    int px;
    int py;
};

struct r {
    struct r_parent rp;
    int x;
    int y;
};

struct s_parent_parent {
    int pp_a;
    int pp_b;
};

struct s_parent {
    struct s_parent_parent spp;
    int parent_a;
    int parent_b;
    struct r *r;
};

struct s {
    struct s_parent sp;
    int a;
    int b;
    void *c;
};

static void func1(void){
    struct s *s;
    s->sp.parent_a = 4;
    s->sp.spp.pp_a = 99;
    s->a = 4;
    s->b = 4;
    s->c = (void *)0x41414141;
    s->sp.r->x = 99;
    s->sp.r->rp.py = 11;
}

static void func2(void){
    struct s *s;
    s->sp.parent_b = 4;
    s->sp.spp.pp_b = 99;
    s->b = 32;
    s->c = (void *)0x111111;
}

int main(int argc, char **argv, const char **envp){
    func1();
    func2();
    return 0;
}
