#ifndef __TIL_AST_FUNCTION_NODE_H__
#define __TIL_AST_FUNCTION_NODE_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/types/functional_type.h>

namespace til {

  /**
   * Class for describing function nodes.
   */
  class function_node: public cdk::expression_node {
    cdk::sequence_node *_arguments;
    til::block_node *_block;

    public:
      function_node(int lineno, cdk::sequence_node *arguments, std::shared_ptr<cdk::basic_type> returnType, til::block_node *block) :
        cdk::expression_node(lineno), _arguments(arguments), _block(block) {

        std::vector<std::shared_ptr<cdk::basic_type>> argTypes;
        for (size_t i = 0; i < arguments->size(); i++) {
          argTypes.push_back(dynamic_cast<cdk::typed_node*>(arguments->node(i))->type());
        }

        this->type(cdk::functional_type::create(argTypes, returnType));
      }

      cdk::sequence_node *arguments() {
        return _arguments;
      }

      til::block_node *block() {
        return _block;
      }

      void accept(basic_ast_visitor *sp, int level) {
        sp->do_function_node(this, level);
      }

  };

} // til

#endif