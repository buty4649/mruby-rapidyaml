#ifndef _RYML_SINGLE_HEADER_AMALGAMATED_HPP_
#include "ryml_all.hpp"
#endif

#ifndef MRUBY_H
#include <mruby.h>
#endif

struct MrbEventHandlerState : public c4::yml::ParserState
{
    c4::yml::NodeData ev_data;
    mrb_value value;
    mrb_value key;
};

struct MrbEventHandler : public c4::yml::EventHandlerStack<MrbEventHandler, MrbEventHandlerState>
{
    using state = MrbEventHandlerState;

    mrb_state *mrb;
    char *arena;
    size_t arena_size;

#define _enable_(bits) _enable__<bits>()
#define _disable_(bits) _disable__<bits>()
#define _has_any_(bits) _has_any__<bits>()

public:
    MrbEventHandler(mrb_state *mrb) : EventHandlerStack(), mrb(mrb), arena(nullptr), arena_size(0)
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
    void begin_stream()
    {
    }

    void end_stream()
    {
    }

    void begin_doc()
    {
    }

    void end_doc()
    {
    }

    void begin_doc_expl()
    {
    }

    void end_doc_expl()
    {
    }

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
    void set_key_anchor(c4::csubstr scalar)
    {
    }

    void set_key_ref(c4::csubstr scalar)
    {
    }

    void set_key_scalar_dquoted(c4::csubstr scalar)
    {
    }

    void set_key_scalar_folded(c4::csubstr scalar)
    {
    }

    void set_key_scalar_literal(c4::csubstr scalar)
    {
    }

    void set_key_scalar_plain(c4::csubstr scalar)
    {
        m_curr->key = mrb_str_new(mrb, scalar.str, scalar.len);
    }

    void set_key_scalar_squoted(c4::csubstr scalar)
    {
    }

    void set_key_tag(c4::csubstr scalar)
    {
    }

    void set_val_anchor(c4::csubstr scalar)
    {
    }

    void set_val_ref(c4::csubstr scalar)
    {
    }

    void set_val_scalar_plain(c4::csubstr scalar)
    {
        mrb_value v = scalar_to_mrb_value(scalar);
        set_mrb_value(v, c4::yml::VAL_PLAIN);
    }

    void set_val_scalar_squoted(c4::csubstr scalar)
    {
        mrb_value v = scalar_to_mrb_str(scalar);
        set_mrb_value(v, c4::yml::VAL_SQUO);
    }

    void set_val_scalar_dquoted(c4::csubstr scalar)
    {
        mrb_value v = scalar_to_mrb_str(scalar);
        set_mrb_value(v, c4::yml::VAL_DQUO);
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

    void set_val_tag(c4::csubstr scalar)
    {
        C4_NOT_IMPLEMENTED_MSG("set_val_tag");
    }

    void actually_val_is_first_key_of_new_map_flow()
    {
        C4_NOT_IMPLEMENTED_MSG("actually_val_is_first_key_of_new_map_flow");
    }
    void actually_val_is_first_key_of_new_map_block()
    {
        m_curr->key = m_curr->value;
        m_curr->value = mrb_nil_value();
        push_new_hash(c4::yml::BLOCK);
    }

    void add_directive(c4::csubstr directive)
    {
    }

    void mark_key_scalar_unfiltered()
    {
        C4_NOT_IMPLEMENTED_MSG("mark_key_scalar_unfiltered");
    }
    void mark_val_scalar_unfiltered()
    {
        C4_NOT_IMPLEMENTED_MSG("mark_val_scalar_unfiltered");
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
    }

    void add_sibling()
    {
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
        if (mrb_nil_p(m_curr->value))
        {
            m_curr->value = new_hash;
        }
        else
        {
            mrb_hash_set(mrb, m_curr->value, m_curr->key, new_hash);
            m_curr->key = mrb_nil_value();
        }

        // _enable_(c4::yml::MAP | type);
        m_curr->ev_data.m_type.type |= c4::yml::MAP | type;

        _push();
        m_curr->value = new_hash;
    }

    void push_new_array(c4::yml::NodeType_e type)
    {
        mrb_value new_ary = mrb_ary_new(mrb);
        if (mrb_nil_p(m_curr->value))
        {
            m_curr->value = new_ary;
        }
        else
        {
            mrb_ary_push(mrb, m_curr->value, new_ary);
        }

        // _enable_(c4::yml::SEQ | type);
        m_curr->ev_data.m_type.type |= c4::yml::SEQ | type;

        _push();
        m_curr->value = new_ary;
    }

    C4_ALWAYS_INLINE bool scalar_is_null(c4::csubstr scalar)
    {
        return scalar.len == 0 ||
               scalar == "null" || scalar == "Null" || scalar == "NULL" ||
               scalar == "~";
    }

    C4_ALWAYS_INLINE bool scalar_is_true(c4::csubstr scalar)
    {
        return scalar == "true" || scalar == "True" || scalar == "TRUE";
    }

    C4_ALWAYS_INLINE bool scalar_is_false(c4::csubstr scalar)
    {
        return scalar == "false" || scalar == "False" || scalar == "FALSE";
    }

    C4_ALWAYS_INLINE mrb_value scalar_to_mrb_str(c4::csubstr scalar)
    {

        return mrb_str_new(mrb, scalar.str, scalar.len);
    }

    mrb_value scalar_to_mrb_value(c4::csubstr scalar)
    {
        if (scalar_is_null(scalar))
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

        if (scalar == ".inf" || scalar == ".Inf" || scalar == ".INF")
        {
            return mrb_float_value(mrb, INFINITY);
        }

        if (scalar == "-.inf" || scalar == "-.Inf" || scalar == "-.INF")
        {
            return mrb_float_value(mrb, -INFINITY);
        }

        return scalar_to_mrb_str(scalar);
    }

    void set_mrb_value(mrb_value v, c4::yml::NodeType_e type)
    {
        if (mrb_hash_p(m_curr->value))
        {
            mrb_hash_set(mrb, m_curr->value, m_curr->key, v);
            m_curr->key = mrb_nil_value();
        }
        else if (mrb_array_p(m_curr->value))
        {
            mrb_ary_push(mrb, m_curr->value, v);
        }
        else
        {
            m_curr->value = v;
        }

        // _enable_(c4::yml::VAL | type);
        m_curr->ev_data.m_type.type |= c4::yml::VAL | type;
    }

#undef _enable_
#undef _disable_
#undef _has_any_
};
