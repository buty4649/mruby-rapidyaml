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

namespace writer
{
#define mrb_obj_to_s(mrb, obj) mrb_funcall_id(mrb, obj, MRB_SYM(to_s), 0)

    class MrbYamlWriter
    {
        mrb_state *mrb;

    public:
        bool colorize;
        bool header;

    public:
        MrbYamlWriter(mrb_state *mrb) : mrb(mrb), colorize(false), header(true) {}
        ~MrbYamlWriter() {}

        mrb_value emit_yaml(mrb_value obj)
        {
            ryml::Tree tree;
            auto root = tree.rootref();
            struct RException *exc = mrb_value_to_yaml(obj, &root, 0);

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
            auto yaml = mrb_str_new(mrb, output.str, output.len - 1);
            if (!header) {
                return yaml;
            }

            auto is_scalar = !(tree.rootref().is_seq() || tree.rootref().is_map());
            auto header = mrb_str_new_cstr(mrb, is_scalar ? "--- " : "---\n");
            return mrb_str_append(mrb, header, yaml);
        }

        mrb_value yaml_module()
        {
            return mrb_obj_value(mrb_module_get_id(mrb, MRB_SYM(YAML)));
        }

    private:
        struct RException *mrb_value_to_yaml(mrb_value obj, ryml::NodeRef *node, size_t depth)
        {
            if (mrb_array_p(obj))
            {
                *node |= ryml::SEQ;
                mrb_int len = RARRAY_LEN(obj);
                for (mrb_int i = 0; i < len; i++)
                {
                    mrb_value v = mrb_ary_ref(mrb, obj, i);

                    auto c = node->append_child();
                    auto exc = mrb_value_to_yaml(v, &c, depth + 1);
                    if (exc != NULL)
                    {
                        return exc;
                    }
                }
            }
            else if (mrb_hash_p(obj))
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
                        // depth is 0-based, so we need to add 1
                        auto color_map_key = mrb_funcall_id(mrb, yaml_module(), MRB_SYM(color_map_key), 1, mrb_int_value(mrb, depth + 1));
                        auto key = mrb_str_set_color(mrb, mrb_str_new(mrb, k.str, k.len), color_map_key, mrb_nil_value(), mrb_nil_value());
                        k = c4::csubstr(RSTRING_PTR(key));
                    }
                    c << ryml::key(k);
                    c |= ryml::KEY_PLAIN;

                    if (k.find("\n") != c4::yml::npos)
                    {
                        c |= ryml::KEY_LITERAL;
                    }

                    auto exc = mrb_value_to_yaml(value, &c, depth + 1);
                    if (exc != NULL)
                    {
                        return exc;
                    }
                }
            }
            else
            {
                auto s = mrb_value_to_scalar(obj);
                *node = s;

                if (s.find("\n") == c4::yml::npos)
                {
                    *node |= ryml::VAL | ryml::VAL_PLAIN;
                }
                else
                {
                    *node |= ryml::VAL | ryml::VAL_LITERAL;
                }
            }

            return NULL;
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
                result = c4::csubstr(colorize ? set_color(MRB_SYM(color_boolean), "true") : "true");
                break;

            case MRB_TT_FALSE:
                result = c4::csubstr(colorize ? set_color(MRB_SYM(color_boolean), "false") : "false");
                break;

            case MRB_TT_INTEGER:
            {
                auto s = RSTRING_PTR(mrb_obj_to_s(mrb, obj));
                result = c4::csubstr(colorize ? set_color(MRB_SYM(color_number), s) : s);
                break;
            }

            case MRB_TT_FLOAT:
            {
                mrb_float f = mrb_float(obj);
                c4::csubstr s;
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
                result = c4::csubstr(colorize ? set_color(MRB_SYM(color_number), s) : s);
                break;
            }

            case MRB_TT_STRING:
                result = c4::csubstr(colorize ? set_color(MRB_SYM(color_string), RSTRING_PTR(obj)) : RSTRING_PTR(obj));
                break;

            case MRB_TT_SYMBOL:
            {
                std::string sym;
                ryml::formatrs(&sym, ":{}", RSTRING_CSTR(mrb, mrb_obj_to_s(mrb, obj)));
                auto s = mrb_str_new_cstr(mrb, sym.c_str());
                result = c4::csubstr(colorize ? set_color(MRB_SYM(color_string), RSTRING_PTR(s)) : RSTRING_PTR(s));
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

}
