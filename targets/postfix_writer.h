#ifndef __TIL_TARGETS_POSTFIX_WRITER_H__
#define __TIL_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <set>
#include <stack>
#include <cdk/emitters/basic_postfix_emitter.h>
#include <cdk/types/types.h>

namespace til {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<til::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;

    bool _outsideFunction = false; // make future declarations global
    std::string _functionReturnLabel; // Label used to return from the current function
    std::set<std::string> _externalFunctions; // External functions to declare
    std::stack<std::string> _functionLabels; // Stack used to fetch the current function label
    int _offset; // Current framepointer offset
    std::vector<std::string> _functionLoopConditionLabels;
    std::vector<std::string> _functionLoopEndLabels;
    bool _controlFlowAltered = false; // Instructions which alter control flow are stop, next and return
    bool _inFunctionBody = false; // Used to check if we are in a function's body
    bool _inFunctionArgs = false; // Used to check if we are in a function's arguments

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<til::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  protected:
    void handleLoopControlInstruction(int level, const std::vector<std::string>& labels,
                                         const std::string& instructionName);
    void acceptAndCast(std::shared_ptr<cdk::basic_type> const type, cdk::expression_node *const node, int lvl);
  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

    inline bool inFunction() {
      return !_outsideFunction && !_functionLabels.empty();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // til

#endif
