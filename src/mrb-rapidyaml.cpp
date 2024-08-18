#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/presym.h>

#define RYML_SINGLE_HDR_DEFINE_NOW
#define RYML_NO_DEFAULT_CALLBACKS
#define RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS
#include "ryml_all.hpp"
#include "event_handler.hpp"
#include "writer.hpp"

struct RymlCallbacks
{
    RymlCallbacks(mrb_state *mrb) : mrb(mrb) {}
    ~RymlCallbacks() { ryml::reset_callbacks(); }

    mrb_state *mrb;

    void set_callbacks()
    {
        ryml::Callbacks c;
        c.m_user_data = this;
        c.m_allocate = &RymlCallbacks::on_allocate;
        c.m_free = &RymlCallbacks::on_free;
        c.m_error = &RymlCallbacks::on_error;

        ryml::set_callbacks(c);
    }

    static void *on_allocate(size_t len, void *hint, void *user_data)
    {
        mrb_state *mrb = ((RymlCallbacks *)user_data)->mrb;
        void *mem = mrb_malloc(mrb, len);
        if (mem == NULL)
        {
            mrb_raise(mrb, E_RUNTIME_ERROR, "could not allocate memory");
        }
        return mem;
    }

    static void on_free(void *mem, size_t size, void *user_data)
    {
        mrb_state *mrb = ((RymlCallbacks *)user_data)->mrb;
        mrb_free(mrb, mem);
    }

    static void on_error(const char *err_msg, size_t len, ryml::Location loc, void *user_data)
    {
        mrb_state *mrb = ((RymlCallbacks *)user_data)->mrb;
        struct RClass *err = mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(YAML)), MRB_SYM(SyntaxError));

        // Remove the location information from the error message
        c4::csubstr msg(err_msg, len);
        c4::csubstr m = msg.sub(0, msg.find("\n"));
        mrb_value e = mrb_str_new(mrb, m.str, m.len);

        mrb_raise(mrb, err, RSTRING_PTR(e));
    }
};

mrb_value mrb_ryaml_dump(mrb_state *mrb, mrb_value self)
{
    mrb_value obj;
    mrb_value opts = mrb_nil_value();

    mrb_get_args(mrb, "o|H", &obj, &opts);

    RymlCallbacks cb(mrb);
    cb.set_callbacks();

    writer::MrbYamlWriter writer(mrb);
    if (mrb_hash_p(opts) && mrb_hash_size(mrb, opts) > 0)
    {
        mrb_value colorize = mrb_hash_get(mrb, opts, mrb_symbol_value(MRB_SYM(colorize)));
        writer.colorize = mrb_test(colorize);

        mrb_value header = mrb_hash_get(mrb, opts, mrb_symbol_value(MRB_SYM(header)));
        if (!mrb_nil_p(header))
        {
            writer.header = mrb_test(header);
        }
    }
    return writer.emit_yaml(obj);
}

mrb_value mrb_ryaml_load(mrb_state *mrb, mrb_value self)
{
    char *yaml;
    mrb_value opts = mrb_nil_value();
    mrb_get_args(mrb, "z|H", &yaml, &opts);

    RymlCallbacks cb(mrb);
    cb.set_callbacks();
    event_handler::MrbEventHandler handler(mrb, ryml::get_callbacks());

    if (mrb_hash_p(opts) && mrb_hash_size(mrb, opts) > 0)
    {
        mrb_value symbolize_names = mrb_hash_get(mrb, opts, mrb_symbol_value(MRB_SYM(symbolize_names)));
        if (mrb_test(symbolize_names))
        {
            handler.symbolize_names = true;
        }

        mrb_value aliases = mrb_hash_get(mrb, opts, mrb_symbol_value(MRB_SYM(aliases)));
        if (mrb_test(aliases))
        {
            handler.aliases = true;
        }
    }

    c4::yml::ParseEngine<event_handler::MrbEventHandler> parser(&handler);
    parser.parse_in_place_ev("-", c4::to_substr(yaml));

    return handler.result();
}

extern "C"
{
    void mrb_mruby_rapidyaml_gem_init(mrb_state *mrb)
    {
        struct RClass *yaml_mod = mrb_define_module_id(mrb, MRB_SYM(YAML));
        mrb_define_module_function_id(mrb, yaml_mod, MRB_SYM(dump), mrb_ryaml_dump, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));
        mrb_define_module_function_id(mrb, yaml_mod, MRB_SYM(load), mrb_ryaml_load, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));
    }

    void mrb_mruby_rapidyaml_gem_final(mrb_state *mrb)
    {
    }
}
