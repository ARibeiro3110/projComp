#ifndef __TIL_AST_PROGRAM_NODE_H__
#define __TIL_AST_PROGRAM_NODE_H__

#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing program nodes.
   */
  class program_node : public cdk::expression_node {
    til::block_node *_statements;

  public:
    program_node(int lineno, til::block_node *statements) :
        cdk::expression_node(lineno), _statements(statements) {
      this->type(cdk::functional_type::create(cdk::primitive_type::create(1, cdk::TYPE_INT)));
    }

    til::block_node *statements() {
      return _statements;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_program_node(this, level);
    }

  };

} // til

#endif
