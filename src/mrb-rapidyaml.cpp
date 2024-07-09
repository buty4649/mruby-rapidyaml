#include <mruby.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/presym.h>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "ryml_all.hpp"

#define scalar_to_mrb_str(v) mrb_str_new(mrb, v.str, v.len)
#define scalar_is_true(v) (v == "true" || v == "True" || v == "TRUE")
#define scalar_is_false(v) (v == "false" || v == "False" || v == "FALSE")

#define PARSER_FLAG_NONE 0
#define PARSER_FLAG_SYMBOLIZE_NAMES 1

typedef uint32_t parser_flag;

#define mrb_obj_to_s(mrb, obj) mrb_funcall_id(mrb, obj, MRB_SYM(to_s), 0)

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

mrb_value scalar_to_mrb_value(mrb_state *mrb, c4::csubstr scalar)
{
    if (ryml::scalar_is_null(scalar))
    {
        return mrb_nil_value();
    }

    if (scalar.is_integer())
    {
        return mrb_str_to_integer(mrb, scalar_to_mrb_str(scalar), 10, false);
    }

    if (scalar.is_real())
    {
        mrb_value f = scalar_to_mrb_str(scalar);
        return mrb_float_value(mrb, mrb_str_to_dbl(mrb, f, false));
    }
    if (scalar == ".nan" || scalar == ".NaN" || scalar == ".NAN")
    {
        return mrb_float_value(mrb, NAN);
    }
    if (scalar == ".inf" || scalar == ".Inf" || scalar == ".INF")
    {
        return mrb_float_value(mrb, INFINITY);
    }

    if (scalar == "-.inf" || scalar == "-.Inf" || scalar == "-.INF")
    {
        return mrb_float_value(mrb, -INFINITY);
    }

    if (scalar_is_true(scalar))
    {
        return mrb_true_value();
    }
    if (scalar_is_false(scalar))
    {
        return mrb_false_value();
    }

    ryml::NodeType type = ryml::scalar_style_choose(scalar);
    if (scalar.begins_with(":") && scalar.len > 1 && !type.is_quoted())
    {
        return mrb_symbol_value(mrb_intern(mrb, scalar.str + 1, scalar.len - 1));
    }

    return scalar_to_mrb_str(scalar);
}

mrb_value yaml_value_to_mrb_value(mrb_state *mrb, ryml::ConstNodeRef node, parser_flag flg)
{
    if (node.is_seq())
    {
        mrb_value ary = mrb_ary_new(mrb);
        for (auto child : node.children())
        {
            mrb_ary_push(mrb, ary, yaml_value_to_mrb_value(mrb, child, flg));
        }
        return ary;
    }

    if (node.is_map())
    {
        mrb_value hash = mrb_hash_new(mrb);
        for (auto c : node.children())
        {
            mrb_value key = scalar_to_mrb_value(mrb, c.key());
            if ((flg & PARSER_FLAG_SYMBOLIZE_NAMES) > 0 && mrb_string_p(key))
            {
                key = mrb_symbol_value(mrb_intern_str(mrb, key));
            }
            mrb_value val = yaml_value_to_mrb_value(mrb, c, flg);
            mrb_hash_set(mrb, hash, key, val);
        }

        return hash;
    }

    return scalar_to_mrb_value(mrb, node.val());
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

    ryml::Tree tree = ryml::parse_in_arena((const char *)yaml);
    ryml::ConstNodeRef root = tree.crootref();

    return yaml_value_to_mrb_value(mrb, root, flg);
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
