#ifndef __TIL_AST_NULL_PTR_NODE_H__
#define __TIL_AST_NULL_PTR_NODE_H__

#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing null pointer nodes.
   */
  class null_ptr_node: public cdk::expression_node {

  public:
    null_ptr_node(int lineno) :
        cdk::expression_node(lineno) {
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_null_ptr_node(this, level);
    }

  };

} // til

#endif