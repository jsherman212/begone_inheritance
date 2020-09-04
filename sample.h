struct r_parent {
    int px;
    int py;
    float r_parent_f;
    double r_parent_d;
};

struct r {
    struct r_parent f;
    int x;
    int y;
    float r_z;
    double r_d;
};

struct s_parent_parent {
    int pp_a;
    int pp_b;
    float pp_f;
    double pp_d;
};

struct s_parent {
    struct s_parent_parent f;
    int parent_a;
    int parent_b;
    float parent_float;
    double parent_d;
    struct r *r;
};

struct s {
    struct s_parent f;
    int a;
    int b;
    void *c;
    float d;
    double e;
};

