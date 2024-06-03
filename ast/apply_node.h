#ifndef __TIL_AST_APPLY_NODE_H__
#define __TIL_AST_APPLY_NODE_H__

#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing apply nodes.
   */
  class apply_node : public cdk::basic_node {
    cdk::expression_node *_function, *_vector, *_low, *_high;

  public:
    apply_node(int lineno, cdk::expression_node *function, cdk::expression_node *vector,
                           cdk::expression_node *low, cdk::expression_node *high) :
        basic_node(lineno), _function(function), _vector(vector),
                            _low(low), _high(high) {
    }

    cdk::expression_node *function() {
      return _function;
    }

    cdk::expression_node *vector() {
      return _vector;
    }

    cdk::expression_node *low() {
      return _low;
    }

    cdk::expression_node *high() {
      return _high;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_apply_node(this, level);
    }

  };

} // til

#endif
