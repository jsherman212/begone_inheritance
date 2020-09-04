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
    bool ih_hide_inheritance;
    inheritance_hider_ah_t ih_ah;       /* action handler for this plugin */
    netnode ih_node;                       /* node where we persist info in the db */
    ssize_t (idaapi *ih_callback)(void *, hexrays_event_t, va_list);

    inheritance_hider_t(void);
    virtual ~inheritance_hider_t(void);

    virtual bool idaapi run(size_t) override {
        return false;
    }

    bool hide_inheritance(cfunc_t *);

    /* bool is_enabled(void){ */
    /*     msg("Inside of is_enabled for inheritance_hider_t!\n"); */
    /*     return this->ih_node.altval(nodeidx_isenabled) == true; */
    /* } */
};

/* called when user clicks on Omit Simulated Inheritance menu item
 *
 * Just sets ih_hide_inheritance and refreshes the pseudocode text
 */
int inheritance_hider_ah_t::activate(action_activation_ctx_t *ctx){
    /* msg("Inside of inheritance_hider_ah_t::activate!\n"); */

    vdui_t &vu = *get_widget_vdui(ctx->widget);
    /* bool ret = vu.get_current_item(USE_KEYBOARD); */

    /* if(!ret){ */
    /*     msg("%s: vu.get_current_item(USE_KEYBOARD) returned false, bailing\n", */
    /*             __func__); */
    /*     return 1; */
    /* } */

    /* cfunc_t *decompiled_func = vu.cfunc; */

    /* if(!this->ih->hide_inheritance(decompiled_func)){ */
    /*     msg("%s: inheritance_hider_t::hide_inheritance failed\n", __func__); */
    /*     return 1; */
    /* } */

    /* if(!vu){ */
    /*     msg("%s: vu NULL, bailing\n", __func__); */
    /*     return 0; */
    /* } */

    /* bool citem = vu.item.is_citem(); */

    /* ctree_item_t cursor_item = vu.item; */
    /* cursor_item_type_t cursor_type = cursor_item.citype; */

    /* if(cursor_type == VDI_NONE) */
    /*     msg("%s: VDI_NONE\n", __func__); */
    /* else if(cursor_type == VDI_EXPR) */
    /*     msg("%s: VDI_EXPR\n", __func__); */
    /* else if(cursor_type == VDI_LVAR) */
    /*     msg("%s: VDI_LVAR\n", __func__); */
    /* else if(cursor_type == VDI_FUNC) */
    /*     msg("%s: VDI_FUNC\n", __func__); */
    /* else if(cursor_type == VDI_TAIL) */
    /*     msg("%s: VDI_TAIL\n", __func__); */
    /* else */
    /*     msg("%s: unknown cursor type: %d\n", __func__, cursor_type); */



    this->ih->ih_hide_inheritance = !this->ih->ih_hide_inheritance;
    msg("Omit selected inheritance: %d\n", this->ih->ih_hide_inheritance);
    vu.refresh_ctext();

    return 1;
}

action_state_t inheritance_hider_ah_t::update(action_activation_ctx_t *ctx){
    /* msg("Inside of inheritance_hider_ah_t::update!\n"); */

    vdui_t *vu = get_widget_vdui(ctx->widget);

    if(!vu){
        /* msg("%s: vu is NULL, disabling\n", __func__); */
        return AST_DISABLE_FOR_WIDGET;
    }

    /* msg("%s: vu is not NULL, enabling\n", __func__); */

    return AST_ENABLE_FOR_WIDGET;
}

static bool is_simulated_inheritance_field(char **line){
    if(!line)
        return false;

    char *_line = *line;

    while(!isalpha(*_line))
        _line++;

    *line = _line;

    /* check if this field isn't simulating inheritance
     *
     * field name will just be "f" if it is
     */
    return *_line == 'f' && !isalpha(*(_line + 1));
}

static void remove_simulated_inheritance_field(char *line){
    if(!line)
        return;

    /* find the next dot, indicating the end of the field
     * which simulates inheritance
     */
    char *dot = strchr(line, '.');

    if(!dot)
        return;

    size_t moveback_len = strlen(dot + 1);
    memmove(line, dot + 1, moveback_len);

    /* null terminate before what we cut off */
    *(line + moveback_len) = '\0';
}

static void begone_simulated_inheritance(const simpleline_t &sl){
    char *line = (char *)&sl.line[0];

    while(*line){
        /* look for a '>' (var->f.f.x, var->f.x->f.f.y) and delete
         * each 'f.'
         */
        char *sym = strchr(line, '>');

        if(sym){
            line = sym + 1;

            if(!is_simulated_inheritance_field(&line))
                continue;

            remove_simulated_inheritance_field(line);

            /* we chopped off an inheritance simulator field, so
             * go back to '>' to see if there's more
             */
            line = sym;

            continue;
        }

        /* char *dot = strchr(line, '.'); */

        /* if(dot){ */
        /*     line = dot + 1; */

        /*     if(!is_simulated_inheritance_field(&line)) */
        /*         continue; */

        /*     remove_simulated_inheritance_field(line); */

        /*     continue; */
        /* } */

        /* line = (char *)tag_advance(line, 1); */
        line++;
    }

    /* msg("%s\n", line); */
}

ssize_t idaapi callback(void *ob, hexrays_event_t event, va_list va){
    /* msg("Inside of callback!\n"); */

    inheritance_hider_t *ih = (inheritance_hider_t *)ob;

    /* msg("%p\n", ih); */

    switch(event){
        case hxe_populating_popup:
            {
                TWidget *widget = va_arg(va, TWidget *);
                TPopupMenu *popup = va_arg(va, TPopupMenu *);
                vdui_t &vu1 = *va_arg(va, vdui_t *);
                bool ret = vu1.get_current_item(USE_KEYBOARD);
                bool citem = vu1.item.is_citem();

                /* vdui_t &vu2 = *va_arg(va, vdui_t *); */
                /* vdui_t vu3 = *va_arg(va, vdui_t *); */
                attach_action_to_popup(widget, popup, action_name);

                break;
            }
        case hxe_maturity:
            {
                /* ctree_maturity_t new_maturity = va_argi(va, ctree_maturity_t); */
                /* msg("%s: hxe_maturity, new_maturity = %d\n", __func__, */
                /*         new_maturity); */
                break;
            }
        case hxe_func_printed:
            {
                if(!ih->ih_hide_inheritance)
                    break;

                cfunc_t *func = va_arg(va, cfunc_t *);
                const strvec_t &sv = func->get_pseudocode();
                size_t num_lines = sv.size();

                for(int i=0; i<num_lines; i++)
                    begone_simulated_inheritance(sv[i]);

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
    this->ih_callback = callback;

    /* needs plugin_terminate so we don't touch dlclose'ed memory */
    install_hexrays_callback(this->ih_callback, this);
    register_action(ACTION_DESC_LITERAL_PLUGMOD(action_name, "Omit Simulated Inheritance",
                &this->ih_ah, this, NULL, NULL, -1));

    update_action_checkable(action_name, true);

    this->ih_inited = true;
    this->ih_hide_inheritance = false;
}

inheritance_hider_t::~inheritance_hider_t(void){
    this->ih_inited = false;
    this->ih_hide_inheritance = false;
    remove_hexrays_callback(this->ih_callback, this);
    this->ih_callback = NULL;
}

bool inheritance_hider_t::hide_inheritance(cfunc_t *fxn){
    /* msg("%s: here\n", __func__); */
    const strvec_t &code_text = fxn->get_pseudocode();
    size_t num_lines = code_text.size();

    /* for(int i=0; i<num_lines; i++){ */
    /*     const simpleline_t *sl = &code_text[i]; */
    /* } */
    


    return true;
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
