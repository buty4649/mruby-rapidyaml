#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/presym.h>

#define RYML_SINGLE_HDR_DEFINE_NOW
#define RYML_NO_DEFAULT_CALLBACKS
#define RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS
#define C4_ALWAYS_INLINE
#include "ryml_all.hpp"
#include "event_handler.hpp"

#define PARSER_FLAG_NONE 0
#define PARSER_FLAG_SYMBOLIZE_NAMES 1

typedef uint32_t parser_flag;

#define mrb_obj_to_s(mrb, obj) mrb_funcall_id(mrb, obj, MRB_SYM(to_s), 0)

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

c4::csubstr mrb_value_to_scalar(mrb_state *mrb, mrb_value obj)
{
    if (mrb_nil_p(obj))
    {
        return c4::csubstr("null");
    }

    c4::csubstr s;
    switch (mrb_type(obj))
    {
    case MRB_TT_TRUE:
        s = c4::csubstr("true");
        break;

    case MRB_TT_FALSE:
        s = c4::csubstr("false");
        break;

    case MRB_TT_INTEGER:
        s = c4::csubstr(RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
        break;

    case MRB_TT_FLOAT:
    {
        mrb_float f = mrb_float(obj);
        if (isnan(f))
        {
            s = c4::csubstr(".nan");
        }
        else if (isinf(f))
        {
            s = c4::csubstr(f > 0 ? ".inf" : "-.inf");
        }
        else
        {
            s = c4::csubstr(RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
        }
        break;
    }

    case MRB_TT_STRING:
        s = c4::csubstr(RSTRING_CSTR(mrb, obj));
        break;

    case MRB_TT_SYMBOL:
    {
        std::string sym;
        ryml::formatrs(&sym, ":{}", RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
        s = c4::csubstr(sym.c_str());
        break;
    }

    default:
        mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid type");
    }

    return s;
}

struct RException *mrb_value_to_yaml_node(mrb_state *mrb, mrb_value obj, ryml::NodeRef *node)
{
    mrb_vtype t = mrb_type(obj);
    if (mrb_nil_p(obj) || t == MRB_TT_TRUE || t == MRB_TT_FALSE || t == MRB_TT_INTEGER || t == MRB_TT_FLOAT || t == MRB_TT_STRING)
    {
        *node |= ryml::VAL | ryml::VAL_PLAIN;
        *node = mrb_value_to_scalar(mrb, obj);
        return NULL;
    }

    if (t == MRB_TT_ARRAY)
    {
        *node |= ryml::SEQ;
        mrb_int len = RARRAY_LEN(obj);
        for (mrb_int i = 0; i < len; i++)
        {
            mrb_value v = mrb_ary_ref(mrb, obj, i);

            auto c = node->append_child();
            mrb_value_to_yaml_node(mrb, v, &c);
        }
    }
    else if (t == MRB_TT_HASH)
    {
        *node |= ryml::MAP;
        mrb_value keys = mrb_hash_keys(mrb, obj);
        mrb_int len = RARRAY_LEN(keys);
        for (mrb_int i = 0; i < len; i++)
        {
            mrb_value key = mrb_ary_ref(mrb, keys, i);
            mrb_value value = mrb_hash_get(mrb, obj, key);

            auto c = node->append_child();
            auto k = mrb_value_to_scalar(mrb, key);
            c << ryml::key(k);
            c |= ryml::KEY_PLAIN;
            mrb_value_to_yaml_node(mrb, value, &c);
        }
    }
    else
    {
        return mrb_value_to_yaml_node(mrb, mrb_obj_to_s(mrb, obj), node);
    }

    return NULL;
}

mrb_value mrb_ryaml_dump(mrb_state *mrb, mrb_value self)
{
    mrb_value obj;
    mrb_value opts = mrb_nil_value();

    mrb_get_args(mrb, "o|H", &obj, &opts);

    RymlCallbacks cb(mrb);
    cb.set_callbacks();

    ryml::Tree tree;
    auto root = tree.rootref();
    mrb_value_to_yaml_node(mrb, obj, &root);

    auto output = ryml::emit_yaml(tree, tree.root_id(), ryml::substr{}, false);
    std::string buf;
    buf.resize(output.len);
    output = ryml::emit_yaml(tree, tree.root_id(), ryml::to_substr(buf), true);

    // remove trailing newline
    return mrb_str_new(mrb, output.str, output.len - 1);
}

mrb_value mrb_ryaml_load(mrb_state *mrb, mrb_value self)
{
    char *yaml;
    mrb_value opts = mrb_nil_value();
    parser_flag flg = PARSER_FLAG_NONE;

    mrb_get_args(mrb, "z|H", &yaml, &opts);

    if (yaml == NULL || strlen(yaml) == 0)
    {
        return mrb_nil_value();
    }

    if (mrb_hash_p(opts) && mrb_hash_size(mrb, opts) > 0)
    {
        mrb_value symbolize_names = mrb_hash_get(mrb, opts, mrb_symbol_value(MRB_SYM(symbolize_names)));
        if (mrb_test(symbolize_names))
        {
            flg |= PARSER_FLAG_SYMBOLIZE_NAMES;
        }
    }

    RymlCallbacks cb(mrb);
    cb.set_callbacks();

    MrbEventHandler handler(mrb);
    c4::yml::ParseEngine<MrbEventHandler> parser(&handler);
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
