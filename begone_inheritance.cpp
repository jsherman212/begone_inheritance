#include <hexrays.hpp>

hexdsp_t *hexdsp = NULL;

struct inheritance_hider_t;

struct inheritance_hider_ah_t : public action_handler_t {
    inheritance_hider_t *ih;             /* plugin object for this action handler */

    inheritance_hider_ah_t(inheritance_hider_t *_ih) : ih(_ih) {}

    virtual int idaapi activate(action_activation_ctx_t *ctx) override;
    virtual action_state_t idaapi update(action_activation_ctx_t *ctx) override;
};

struct inheritance_hider_t : public plugmod_t {
    const char *ih_nodename = "$ Begone Inheritance";
    const int ih_nodeidx_isenabled = 0;
    const char *ih_action_name = "begone:inheritance";

    inheritance_hider_ah_t ih_ah;       /* action handler for this plugin */
    netnode ih_node;                    /* node where we persist info in the db */
    ssize_t (idaapi *ih_callback)(void *, hexrays_event_t, va_list);

    inheritance_hider_t(void);
    virtual ~inheritance_hider_t(void){}

    virtual bool idaapi run(size_t) override {
        return false;
    }

    bool enabled(void){
        return this->ih_node.altval(this->ih_nodeidx_isenabled) == true;
    }
};

/* called when user clicks on Omit Simulated Inheritance menu item
 *
 * Just sets ih_hide_inheritance and refreshes the pseudocode text
 */
int inheritance_hider_ah_t::activate(action_activation_ctx_t *ctx){
    this->ih->ih_node.altset(this->ih->ih_nodeidx_isenabled,
            !this->ih->enabled());

    vdui_t &vu = *get_widget_vdui(ctx->widget);
    vu.refresh_view(false);

    return 1;
}

action_state_t inheritance_hider_ah_t::update(action_activation_ctx_t *ctx){
    vdui_t *vu = get_widget_vdui(ctx->widget);

    if(!vu)
        return AST_DISABLE_FOR_WIDGET;

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

    /* save (what seems like) a field ID for this field to write
     * back later. Otherwise, when we go to set the type or rename
     * a field after removing simulated inheritance, we'll just be
     * prompted to rename the field which simulates inheritance
     *
     * The field ID will be right after the first '(' we see while
     * backtracking from `dot`. They always seem to be sixteen characters.
     */
    char *paren = dot;

    while(*paren != '(')
        paren--;

    char field_id[16];
    memcpy(field_id, paren + 1, sizeof(field_id));

    size_t moveback_len = strlen(dot + 1);
    memmove(line, dot + 1, moveback_len);

    /* null terminate before what we cut off */
    *(line + moveback_len) = '\0';

    paren = line;

    while(*paren != '(')
        paren--;

    /* write correct field ID */
    memcpy(paren + 1, field_id, sizeof(field_id));
}

static void begone_simulated_inheritance(const simpleline_t &sl){
    /* const char *tester = "\x01(0000000000000000\x01(0000000000000001  \x01(0000000000000005\x01\rv0\x02\r\x01(0000000000000003\x01\t->\x02\t\x01\t\x02\t\x01\tparent_a\x02\t \x01(0000000000000002\x01\t=\x02\t \x01(0000000000000006\x01 4\x02 \x01\t;\x02\t\x01(0000000000000001"; */
    char *line = (char *)&sl.line[0];

    /* if(strlen(line) <= strlen(tester)) */
    /*     return; */

    /* qstrncpy(line, tester, strlen(tester)); */
    /* line[strlen(tester)-1] = '\0'; */

    /* return; */
    const char *orig_line = line;

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

        line++;
    }
}

ssize_t idaapi callback(void *ob, hexrays_event_t event, va_list va){
    inheritance_hider_t *ih = (inheritance_hider_t *)ob;

    switch(event){
        case hxe_populating_popup:
            {
                TWidget *widget = va_arg(va, TWidget *);
                TPopupMenu *popup = va_arg(va, TPopupMenu *);
                attach_action_to_popup(widget, popup, ih->ih_action_name);

                break;
            }
        case hxe_func_printed:
            {
                if(!ih->enabled())
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

    return 0;
}

inheritance_hider_t::inheritance_hider_t(void) : ih_ah(this) {
    this->ih_node.create(this->ih_nodename);
    this->ih_callback = callback;

    /* needs plugin_terminate so we don't touch dlclose'ed memory */
    install_hexrays_callback(this->ih_callback, this);
    register_action(ACTION_DESC_LITERAL_PLUGMOD(this->ih_action_name,
                "Omit Simulated Inheritance", &this->ih_ah, this, NULL, NULL, -1));
    update_action_checkable(this->ih_action_name, true);
    update_action_checked(this->ih_action_name, this->enabled());
}

plugmod_t *idaapi plugin_init(void){
    if(!init_hexrays_plugin())
        return PLUGIN_SKIP;

    return new inheritance_hider_t();
}

static const char plugin_comment[] = "Visually omit f.f.f noise from"
                                     " simulating inheritance";

plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX | PLUGIN_MULTI | PLUGIN_HIDE,
    plugin_init,
    NULL,
    NULL,
    plugin_comment,
    "",
    "Begone Inheritance",
    "",                     /* hotkey to run the plugin */
};
