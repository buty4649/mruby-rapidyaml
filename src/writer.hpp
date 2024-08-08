#ifndef _RYML_SINGLE_HEADER_AMALGAMATED_HPP_
#include "ryml_all.hpp"
#endif

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/presym.h>

#include "mrb_terminal_color.h"

#define mrb_obj_to_s(mrb, obj) mrb_funcall_id(mrb, obj, MRB_SYM(to_s), 0)

class MrbYamlWriter
{
    mrb_state *mrb;

public:
    bool colorize;

public:
    MrbYamlWriter(mrb_state *mrb) : mrb(mrb), colorize(false) {}
    ~MrbYamlWriter() {}

    mrb_value emit_yaml(mrb_value obj)
    {
        ryml::Tree tree;
        auto root = tree.rootref();
        struct RException *exc = mrb_value_to_yaml(obj, &root);

        if (exc != NULL)
        {
            mrb_exc_raise(mrb, mrb_obj_value(exc));
        }

        // estimate the size of the output
        auto output = ryml::emit_yaml(tree, tree.root_id(), ryml::substr{}, false);
        std::string buf;
        buf.resize(output.len);
        output = ryml::emit_yaml(tree, tree.root_id(), ryml::to_substr(buf), true);

        // remove the trailing newline
        return mrb_str_new(mrb, output.str, output.len - 1);
    }

    mrb_value yaml_module()
    {
        return mrb_obj_value(mrb_module_get_id(mrb, MRB_SYM(YAML)));
    }

private:
    struct RException *mrb_value_to_yaml(mrb_value obj, ryml::NodeRef *node)
    {
        mrb_vtype t = mrb_type(obj);
        if (mrb_nil_p(obj) || t == MRB_TT_TRUE || t == MRB_TT_FALSE || t == MRB_TT_INTEGER || t == MRB_TT_FLOAT || t == MRB_TT_STRING)
        {
            auto s = mrb_value_to_scalar(obj);
            *node = s;

            if (s.find("\n") == c4::yml::npos) {
                *node |= ryml::VAL | ryml::VAL_PLAIN;
            } else {
                *node |= ryml::VAL | ryml::VAL_LITERAL;
            }
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
                auto exc = mrb_value_to_yaml(v, &c);
                if (exc != NULL)
                {
                    return exc;
                }
            }

            return NULL;
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

                auto old_colorize = colorize;
                colorize = FALSE;
                auto k = mrb_value_to_scalar(key);
                colorize = old_colorize;

                if (colorize)
                {
                    k = set_color(MRB_SYM(color_object_key), k);
                }
                c << ryml::key(k);
                c |= ryml::KEY_PLAIN;

                auto exc = mrb_value_to_yaml(value, &c);
                if (exc != NULL)
                {
                    return exc;
                }
            }

            return NULL;
        }

        return mrb_value_to_yaml(mrb_obj_to_s(mrb, obj), node);
    }

    c4::csubstr mrb_value_to_scalar(mrb_value obj)
    {

        if (mrb_nil_p(obj))
        {
            return c4::csubstr(colorize ? set_color(MRB_SYM(color_null), "null") : "null");
        }

        c4::csubstr result;

        switch (mrb_type(obj))
        {
        case MRB_TT_TRUE:
            result = c4::csubstr("true");
            break;

        case MRB_TT_FALSE:
            result = c4::csubstr("false");
            break;

        case MRB_TT_INTEGER:
            result = c4::csubstr(RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
            break;

        case MRB_TT_FLOAT:
        {
            mrb_float f = mrb_float(obj);
            if (isnan(f))
            {
                result = c4::csubstr(".nan");
            }
            else if (isinf(f))
            {
                result = c4::csubstr(f > 0 ? ".inf" : "-.inf");
            }
            else
            {
                result = c4::csubstr(RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
            }
            break;
        }

        case MRB_TT_STRING:
            result = c4::csubstr(colorize ? set_color(MRB_SYM(color_string), RSTRING_PTR(obj)) : RSTRING_PTR(obj));
            break;

        case MRB_TT_SYMBOL:
        {
            std::string sym;
            ryml::formatrs(&sym, ":{}", RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
            result = c4::csubstr(sym.c_str());
            break;
        }

        default:
        {
            auto e = mrb_class_get_under_id(mrb, mrb_class_ptr(yaml_module()), MRB_SYM(GeneratorError));
            mrb_raise(mrb, e, "invalid type");
        }
        }

        return result;
    }

private:
    mrb_value set_color(mrb_sym type, mrb_value str)
    {
        mrb_value color = mrb_funcall_id(mrb, yaml_module(), type, 0);
        return mrb_str_set_color(mrb, str, color, mrb_nil_value(), mrb_nil_value());
    }

    const char *set_color(mrb_sym type, const char *str)
    {
        return RSTRING_PTR(set_color(type, mrb_str_new_cstr(mrb, str)));
    }

    c4::csubstr set_color(mrb_sym type, c4::csubstr str)
    {
        return c4::csubstr(RSTRING_PTR(set_color(type, mrb_str_new(mrb, str.str, str.len))));
    }
};
