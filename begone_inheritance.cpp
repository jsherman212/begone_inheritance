#include <hexrays.hpp>

hexdsp_t *hexdsp = NULL;

static const char *nodename = "$ Begone Inheritance";
static const int nodeidx_isenabled = 0;

static const char *action_name = "begone:inheritance";

struct inheritance_hider_t;

struct inheritance_hider_ah_t : public action_handler_t {
    inheritance_hider_t *ih;             /* plugin object for this action handler */

    inheritance_hider_ah_t(inheritance_hider_t *_ih) : ih(_ih) {}

    virtual int idaapi activate(action_activation_ctx_t *ctx) override;
    virtual action_state_t idaapi update(action_activation_ctx_t *ctx) override;
};

struct inheritance_hider_t : public plugmod_t {
    bool ih_inited;
    inheritance_hider_ah_t ih_ah;       /* action handler for this plugin */
    netnode ih_node;                       /* node where we persist info in the db */
    ssize_t (idaapi *ih_callback)(void *, hexrays_event_t, va_list);

    inheritance_hider_t(void);
    virtual ~inheritance_hider_t(void);

    virtual bool idaapi run(size_t) override {
        msg("Hello from inheritance_hider_t::run\n");
        return true;
    }

    bool is_enabled(void){
        msg("Inside of is_enabled for inheritance_hider_t!\n");
        return this->ih_node.altval(nodeidx_isenabled) == true;
    }
};

int inheritance_hider_ah_t::activate(action_activation_ctx_t *ctx){
    msg("Inside of inheritance_hider_ah_t::activate!\n");
    return 0;
}

action_state_t inheritance_hider_ah_t::update(action_activation_ctx_t *ctx){
    msg("Inside of inheritance_hider_ah_t::update!\n");
    
    return AST_DISABLE_FOR_WIDGET;
}

ssize_t idaapi callback(void *ob, hexrays_event_t event, va_list va){
    msg("Inside of callback!\n");

    inheritance_hider_t *ih = (inheritance_hider_t *)ob;

    /* msg("%p\n", ih); */

    switch(event){
        case hxe_populating_popup:
            {
                TWidget *widget = va_arg(va, TWidget *);
                TPopupMenu *popup = va_arg(va, TPopupMenu *);
                vdui_t *vu = va_arg(va, vdui_t *);
                attach_action_to_popup(widget, popup, action_name);

                break;
            }
        default:
            break;
    };


    /* asm volatile("int3"); */

    /* if(event == hxe_maturity){ */
    /*     cfunc_t *cfunc = va_arg(va, cfunc_t *); */
    /*     ctree_maturity_t mat = va_argi(va, ctree_maturity_t); */

    /*     if(mat == CMAT_FINAL) */
    /*         plugin_do(cfunc); */
    /* } */

    return 0;
}

inheritance_hider_t::inheritance_hider_t(void) : ih_ah(this) {
    /* bool node_exists_in_db = this->ih_node.create(nodename); */

    /* if(!node_exists_in_db){ */
    /*     // TODO read back information stored in this node */
    /* } */

    this->ih_callback = callback;

    /* needs plugin_terminate so we don't touch dlclose'ed memory */
    install_hexrays_callback(this->ih_callback, this);
    register_action(ACTION_DESC_LITERAL_PLUGMOD(action_name, "Omit Simulated Inheritance",
                &this->ih_ah, this, NULL, NULL, -1));
    /* msg("register_action %d\n", ret); */

    this->ih_inited = true;
}

inheritance_hider_t::~inheritance_hider_t(void){
    this->ih_inited = false;
    remove_hexrays_callback(this->ih_callback, this);
    this->ih_callback = NULL;
}

plugmod_t *idaapi plugin_init(void){
    msg("Inside of plugin_init!\n");

    if(!init_hexrays_plugin()){
        msg("Plugin skipped\n");
        return PLUGIN_SKIP;
    }

    msg("Plugin inited\n");

    return new inheritance_hider_t();
}

/* bool idaapi plugin_invoke(size_t arg0){ */
/*     msg("Inside of plugin_invoke!\n"); */

    /* int code = ask_buttons("Enable", "Disable", "Close", -1, */
    /*         "plugin_invoke called.\n" */
    /*         "The current state of the plugin is %s\n", */
    /*         is_enabled() ? "ENABLED" : "DISABLED"); */

    /* switch(code){ */
    /*     case ASKBTN_BTN3:       /1* close *1/ */
    /*         break; */
    /*     case ASKBTN_BTN2:       /1* disable *1/ */
    /*     case ASKBTN_BTN1:       /1* enable *1/ */
    /*         bool enabled = code == ASKBTN_BTN1; */

    /*         netnode n; */
    /*         n.create(nodename); */
    /*         n.altset(nodeidx_isenabled, enabled); */

    /*         if(enabled) */
    /*             install_hexrays_callback(callback, NULL); */
    /*         else */
    /*             remove_hexrays_callback(callback, NULL); */

    /*         msg("%s has been %s\n", PLUGIN.wanted_name, enabled ? "enabled" : */
    /*                 "disabled"); */
    /* }; */

    /* return true; */
/* } */

static const char plugin_comment[] = "Visually omit f.f.f noise";

plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX | PLUGIN_MULTI | PLUGIN_HIDE,
    plugin_init,
    NULL,
    NULL,
    plugin_comment,
    "Multiline help",
    "Begone Inheritance",
    "",                     /* hotkey to run the plugin */
};
