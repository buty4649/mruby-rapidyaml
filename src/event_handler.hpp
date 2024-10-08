#ifndef _RYML_SINGLE_HEADER_AMALGAMATED_HPP_
#include "ryml_all.hpp"
#endif

#include <mruby.h>
#include <mruby/presym.h>

namespace event_handler
{

#define E_YAML_ALIASES_NOT_ENABLED mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(YAML)), MRB_SYM(AliasesNotEnabled))
#define E_YAML_ANCHOR_NOT_DEFINED mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(YAML)), MRB_SYM(AnchorNotDefined))
#define E_YAML_SYNTAX_ERROR mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(YAML)), MRB_SYM(SyntaxError))

#define RSTRING_CSUBSTR(str) c4::csubstr(RSTRING_PTR(str), RSTRING_LEN(str))

    struct MrbEventHandlerState : public c4::yml::ParserState
    {
        c4::yml::NodeData ev_data;
        mrb_value value;
        mrb_value key;
        mrb_value anchor;

        MrbEventHandlerState() : ParserState()
        {
            value = mrb_nil_value();
            key = mrb_nil_value();
            anchor = mrb_nil_value();
        }

        c4::csubstr type_str();

        bool is_map() const { return ev_data.m_type.is_map(); }
        bool is_seq() const { return ev_data.m_type.is_seq(); }
        bool has_anchor() const { return ev_data.m_type.has_anchor(); }
        bool has_val() const { return ev_data.m_type.has_val(); }
    };

    // Prevents inlining to allow evaluation in debug watch expressions
    c4::csubstr MrbEventHandlerState::type_str()
    {
        return ev_data.m_type.type_str();
    }

    struct MrbEventHandler : public c4::yml::EventHandlerStack<MrbEventHandler, MrbEventHandlerState>
    {
        using state = MrbEventHandlerState;

#define _enable_(bits) _enable__<bits>()
#define _disable_(bits) _disable__<bits>()
#define _has_any_(bits) _has_any__<bits>()

#define _NOT_IMPLEMENTED_MSG(msg) mrb_raise(mrb, E_NOTIMP_ERROR, msg)

    private:
        mrb_state *mrb;
        char *arena;
        size_t arena_size;
        mrb_value anchors;

    public:
        bool aliases;
        bool symbolize_names;

        MrbEventHandler(mrb_state *mrb, ryml::Callbacks const &cb) : EventHandlerStack(cb),
                                                                     mrb(mrb), arena(nullptr), arena_size(0), anchors(mrb_hash_new(mrb)),
                                                                     aliases(false), symbolize_names(false)
        {
            _stack_reset_root();
            m_curr->flags |= c4::yml::RUNK | c4::yml::RTOP;
        }

        ~MrbEventHandler()
        {
            if (arena != nullptr)
            {
                _RYML_CB_FREE(m_stack.m_callbacks, arena, char, arena_size);
            }
        }

        mrb_value result()
        {
            return m_curr->value;
        }

    public:
        void start_parse(const char *filename, c4::yml::detail::pfn_relocate_arena relocate_arena, void *relocate_arena_data)
        {
            this->_stack_start_parse(filename, relocate_arena, relocate_arena_data);
        }

        void finish_parse()
        {
            this->_stack_finish_parse();
        }

        void cancel_parse()
        {
            while (m_stack.size() > 1)
                _pop();
        }

    public:
        void begin_stream() {}
        void end_stream() {}

        void begin_doc() {}
        void end_doc() {}

        void begin_doc_expl() {}
        void end_doc_expl() {}

        void begin_map_key_block()
        {
            push_new_hash(c4::yml::BLOCK);
        }

        void begin_map_val_block()
        {
            push_new_hash(c4::yml::BLOCK);
        }

        void begin_map_key_flow()
        {
            push_new_hash(c4::yml::FLOW_SL);
        }

        void begin_map_val_flow()
        {
            push_new_hash(c4::yml::FLOW_SL);
        }

        void end_map()
        {
            _pop();
        }

        void begin_seq_key_block()
        {
            push_new_array(c4::yml::BLOCK);
        }

        void begin_seq_val_block()
        {
            push_new_array(c4::yml::BLOCK);
        }

        void begin_seq_key_flow()
        {
            push_new_array(c4::yml::FLOW_SL);
        }

        void begin_seq_val_flow()
        {
            push_new_array(c4::yml::FLOW_SL);
        }

        void end_seq()
        {
            _pop();
        }

    public:
        void set_key_scalar_plain(c4::csubstr scalar)
        {
            set_key(scalar, c4::yml::KEY_PLAIN);
        }

        void set_key_scalar_dquoted(c4::csubstr scalar)
        {
            set_key(scalar, c4::yml::KEY_DQUO);
        }

        void set_key_scalar_squoted(c4::csubstr scalar)
        {
            set_key(scalar, c4::yml::KEY_SQUO);
        }

        void set_key_scalar_folded(c4::csubstr scalar)
        {
            set_key(scalar, c4::yml::KEY_FOLDED);
        }

        void set_key_scalar_literal(c4::csubstr scalar)
        {
            set_key(scalar, c4::yml::KEY_LITERAL);
        }

        void set_key_anchor(c4::csubstr scalar)
        {
            m_curr->anchor = validate_and_convert_anchor(scalar);
            _enable_(c4::yml::KEY | c4::yml::KEYANCH);
        }

        void set_key_ref(c4::csubstr scalar)
        {
            mrb_value ref = validate_and_convert_anchor(scalar.triml("*"));
            if (!mrb_hash_key_p(mrb, anchors, ref))
            {
                raise_error(E_YAML_ANCHOR_NOT_DEFINED, "anchor not defined: %.*s", scalar.len, scalar.str);
            }
            set_key(mrb_hash_get(mrb, anchors, ref), c4::yml::KEYREF);
        }

        void set_key_tag(c4::csubstr scalar)
        {
            _NOT_IMPLEMENTED_MSG("set_key_tag");
        }

        void set_key(c4::csubstr scalar, c4::yml::NodeType_e type)
        {
            mrb_value key;
            if (type == c4::yml::KEY_PLAIN)
            {
                key = scalar_to_mrb_value(scalar);
            }
            else
            {
                key = scalar_to_mrb_str(scalar);
            }
            set_key(key, type);
        }

        void set_key(mrb_value key, c4::yml::NodeType_e type)
        {
            m_curr->key = key;

            if (_has_any_(c4::yml::KEYANCH))
            {
                mrb_hash_set(mrb, anchors, m_curr->anchor, key);
                m_curr->anchor = mrb_nil_value();
                _disable_(c4::yml::KEYANCH);
            }

            m_curr->ev_data.m_type.type |= c4::yml::KEY | type;
        }

    public:
        void set_val_scalar_plain(c4::csubstr scalar)
        {
            mrb_value v = scalar_to_mrb_value(scalar);
            set_mrb_value(v, c4::yml::VAL_PLAIN);
        }

        void set_val_scalar_dquoted(c4::csubstr scalar)
        {
            mrb_value v = scalar_to_mrb_str(scalar);
            set_mrb_value(v, c4::yml::VAL_DQUO);
        }

        void set_val_scalar_squoted(c4::csubstr scalar)
        {
            mrb_value v = scalar_to_mrb_str(scalar);
            set_mrb_value(v, c4::yml::VAL_SQUO);
        }

        void set_val_scalar_folded(c4::csubstr scalar)
        {
            mrb_value v = scalar_to_mrb_str(scalar);
            set_mrb_value(v, c4::yml::VAL_FOLDED);
        }

        void set_val_scalar_literal(c4::csubstr scalar)
        {
            mrb_value v = scalar_to_mrb_str(scalar);
            set_mrb_value(v, c4::yml::VAL_LITERAL);
        }

        void set_val_anchor(c4::csubstr scalar)
        {
            m_curr->anchor = validate_and_convert_anchor(scalar);
            if (m_curr->has_val())
            {
                _enable_(c4::yml::KEYANCH);
            }
            else
            {
                _enable_(c4::yml::VALANCH);
            }
        }

        void set_val_ref(c4::csubstr scalar)
        {
            mrb_value anchor = validate_and_convert_anchor(scalar.triml("*"));
            if (!mrb_hash_key_p(mrb, anchors, anchor))
            {
                raise_error(E_YAML_ANCHOR_NOT_DEFINED, "anchor not defined: %.*s", scalar.len, scalar.str);
            }

            auto ref = mrb_hash_get(mrb, anchors, anchor);

            if (aliases && RSTRING_CSUBSTR(m_curr->key).compare("<<") >= 0 && mrb_hash_p(ref))
            {
                mrb_hash_merge(mrb, m_curr->value, ref);
            }
            else
            {
                set_mrb_value(ref, c4::yml::VALREF);
            }
        }

        void set_val_tag(c4::csubstr scalar)
        {
            _NOT_IMPLEMENTED_MSG("set_val_tag");
        }

        void actually_val_is_first_key_of_new_map_flow()
        {
            _NOT_IMPLEMENTED_MSG("actually_val_is_first_key_of_new_map_flow");
        }
        void actually_val_is_first_key_of_new_map_block()
        {
            m_curr->key = m_curr->value;
            m_curr->value = mrb_nil_value();
            push_new_hash(c4::yml::BLOCK);
        }

        void add_directive(c4::csubstr directive)
        {
            _NOT_IMPLEMENTED_MSG("add_directive");
        }

        void mark_key_scalar_unfiltered()
        {
            _NOT_IMPLEMENTED_MSG("mark_key_scalar_unfiltered");
        }
        void mark_val_scalar_unfiltered()
        {
            _NOT_IMPLEMENTED_MSG("mark_val_scalar_unfiltered");
        }

        void set_mrb_value(mrb_value v, c4::yml::NodeType_e type)
        {
            if (m_parent != nullptr && m_parent->is_map())
            {
                mrb_hash_set(mrb, m_parent->value, m_curr->key, v);
            }
            else if (m_parent != nullptr && m_parent->is_seq())
            {
                mrb_ary_push(mrb, m_parent->value, v);
            }
            else
            {
                m_curr->value = v;
            }

            if (_has_any_(c4::yml::VALANCH))
            {
                mrb_hash_set(mrb, anchors, m_curr->anchor, v);
                m_curr->anchor = mrb_nil_value();
                _disable_(c4::yml::VALANCH);
            }

            m_curr->ev_data.m_type.type |= c4::yml::VAL | type;
        }

    public:
        void _push()
        {
            _stack_push();
            m_curr->ev_data = {};
        }

        void _pop()
        {
            _stack_pop();

            if (m_curr->has_anchor())
            {
                mrb_hash_set(mrb, anchors, m_curr->anchor, m_curr->value);
                m_curr->anchor = mrb_nil_value();
                m_curr->ev_data.m_type.type &= ~(c4::yml::KEYANCH | c4::yml::VALANCH);
            }

            if (m_parent != nullptr && m_parent->is_map())
            {
                // if the key is not set, then the value is the key
                if (_has_any_(c4::yml::KEY))
                {
                    mrb_hash_set(mrb, m_parent->value, m_curr->key, m_curr->value);
                }
                else
                {
                    m_curr->key = m_curr->value;
                    m_curr->value = mrb_nil_value();
                    m_curr->ev_data.m_type.type = c4::yml::KEY;
                }
            }
            else if (m_parent != nullptr && m_parent->is_seq())
            {
                mrb_ary_push(mrb, m_parent->value, m_curr->value);
            }
        }

        void add_sibling()
        {
            _RYML_CB_ASSERT(m_stack.m_callbacks, m_parent);
            m_curr->ev_data = {};
        }

        c4::substr alloc_arena(size_t len, c4::substr *relocated)
        {
            char *new_arena = (char *)_RYML_CB_ALLOC(m_stack.m_callbacks, char, len);
            char *prev = arena;

            if (prev != nullptr)
            {
                _stack_relocate_to_new_arena(prev, new_arena);
                _RYML_CB_FREE(m_stack.m_callbacks, prev, char, arena_size);
            }
            arena = new_arena;
            arena_size = len;
            return {new_arena, len};
        }

    public:
        template <c4::yml::type_bits bits>
        C4_ALWAYS_INLINE void
        _enable__() noexcept
        {
            m_curr->ev_data.m_type.type = static_cast<c4::yml::NodeType_e>(m_curr->ev_data.m_type.type | bits);
        }
        template <c4::yml::type_bits bits>
        C4_ALWAYS_INLINE void _disable__() noexcept
        {
            m_curr->ev_data.m_type.type = static_cast<c4::yml::NodeType_e>(m_curr->ev_data.m_type.type & (~bits));
        }
        template <c4::yml::type_bits bits>
        C4_ALWAYS_INLINE bool _has_any__() const noexcept
        {
            return (m_curr->ev_data.m_type.type & bits) != 0;
        }

    private:
        void push_new_hash(c4::yml::NodeType_e type)
        {
            mrb_value new_hash = mrb_hash_new(mrb);
            m_curr->value = new_hash;
            m_curr->ev_data.m_type.type |= c4::yml::MAP | type;

            _push();
        }

        void push_new_array(c4::yml::NodeType_e type)
        {
            mrb_value new_ary = mrb_ary_new(mrb);
            m_curr->value = new_ary;
            m_curr->ev_data.m_type.type |= c4::yml::SEQ | type;

            _push();
        }

        C4_ALWAYS_INLINE bool scalar_is_true(c4::csubstr scalar)
        {
            return scalar == "true" || scalar == "True" || scalar == "TRUE" ||
                   scalar == "yes" || scalar == "Yes" || scalar == "YES" ||
                   scalar == "on" || scalar == "On" || scalar == "ON";
        }

        C4_ALWAYS_INLINE bool scalar_is_false(c4::csubstr scalar)
        {
            return scalar == "false" || scalar == "False" || scalar == "FALSE" ||
                   scalar == "no" || scalar == "No" || scalar == "NO" ||
                   scalar == "off" || scalar == "Off" || scalar == "OFF";
        }

        C4_ALWAYS_INLINE mrb_value scalar_to_mrb_str(c4::csubstr scalar)
        {

            return mrb_str_new(mrb, scalar.str, scalar.len);
        }

        mrb_value scalar_to_mrb_value(c4::csubstr scalar)
        {
            if (ryml::scalar_is_null(scalar))
            {
                return mrb_nil_value();
            }

            if (scalar_is_true(scalar))
            {
                return mrb_true_value();
            }

            if (scalar_is_false(scalar))
            {
                return mrb_false_value();
            }

            if (scalar.begins_with(":") && scalar.len > 1)
            {
                return mrb_symbol_value(mrb_intern(mrb, scalar.str + 1, scalar.len - 1));
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

            if (scalar == ".inf" || scalar == ".Inf" || scalar == ".INF" ||
                scalar == "+.inf" || scalar == "+.Inf" || scalar == "+.INF")
            {
                return mrb_float_value(mrb, INFINITY);
            }

            if (scalar == "-.inf" || scalar == "-.Inf" || scalar == "-.INF")
            {
                return mrb_float_value(mrb, -INFINITY);
            }

            return scalar_to_mrb_str(scalar);
        }

        mrb_value validate_and_convert_anchor(c4::csubstr scalar)
        {
            if (!aliases)
            {
                raise_error(E_YAML_ALIASES_NOT_ENABLED, "aliases are not allowed");
            }

            if (!validate_anchor_name(scalar))
            {
                raise_error(E_YAML_SYNTAX_ERROR, "invalid anchor: %.*s", scalar.len, scalar.str);
            }

            return scalar_to_mrb_str(scalar);
        }

        bool validate_anchor_name(c4::csubstr scalar)
        {
            if (scalar.empty())
            {
                return false;
            }

            for (size_t i = 0; i < scalar.len; ++i)
            {
                char c = scalar[i];
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-')
                {
                    continue;
                }
                return false;
            }
            return true;
        }

        void raise_error(struct RClass *exc, const char *msg, ...)
        {
            va_list args;

            va_start(args, msg);
            size_t size = std::vsnprintf(nullptr, 0, msg, args);
            va_end(args);

            if (size < 0)
            {
                mrb_raise(mrb, E_RUNTIME_ERROR, "could not format error message");
            }

            std::string error_msg(size, '\0');
            va_start(args, msg);
            std::vsnprintf(&error_msg[0], size + 1, msg, args);
            va_end(args);

            mrb_raise(mrb, exc, error_msg.c_str());
        }

#undef _enable_
#undef _disable_
#undef _has_any_
    };

};
