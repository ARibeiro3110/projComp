#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

#include "til_parser.tab.h"

//---------------------------------------------------------------------------

void til::postfix_writer::acceptAndCast(std::shared_ptr<cdk::basic_type> const type, cdk::expression_node *const node, int lvl) {
  if (!node->is_typed(cdk::TYPE_FUNCTIONAL)) { // TODO: || type->name() != cdk::TYPE_FUNCTIONAL
    node->accept(this, lvl);

    if (type->name() == cdk::TYPE_DOUBLE && node->is_typed(cdk::TYPE_INT)) {
      _pf.I2D();
    }

    return;
  }

  std::shared_ptr<cdk::functional_type> intended_type = cdk::functional_type::cast(type);
  std::shared_ptr<cdk::functional_type> node_type = cdk::functional_type::cast(node->type());

  bool neededI2Dconversion = false;

  if (intended_type->output(0)->name() == cdk::TYPE_DOUBLE
      && node_type->name() == cdk::TYPE_INT) {
    neededI2Dconversion = true;
  } else {
    for (size_t i = 0; i < node_type->input()->size(); i++) {
      if (intended_type->input(i)->name() == cdk::TYPE_DOUBLE
          && node_type->input(i)->name() == cdk::TYPE_INT) {
        neededI2Dconversion = true;
        break;
      }
    }
  }

  if (!neededI2Dconversion) {
    node->accept(this, lvl);
    return;
  }

  // Needed conversion from int to double in arguments and/or return

  int lineno = node->lineno();

  std::string aux_name = "aux_" + std::to_string(++_lbl);
  til::declaration_node *aux_decl = new til::declaration_node(lineno, tPRIVATE, node_type, aux_name, nullptr);
  cdk::variable_node *aux_var = new cdk::variable_node(lineno, aux_name);

  _outsideFunction = true;
  aux_decl->accept(this, lvl);
  _outsideFunction = false;

  if (inFunction()) { 
    _pf.TEXT(_functionLabels.top());
  } else {
    _pf.DATA();
  }
  _pf.ALIGN();

  cdk::assignment_node *aux_assignment = new cdk::assignment_node(lineno, aux_var, node);
  aux_assignment->accept(this, lvl);
  cdk::rvalue_node *aux_rvalue = new cdk::rvalue_node(lineno, aux_var);
  
  cdk::sequence_node *arguments = new cdk::sequence_node(lineno);
  cdk::sequence_node *call_arguments = new cdk::sequence_node(lineno);

  for (size_t i = 0; i < intended_type->input_length(); i++) {
    std::string argument_name = "_arg" + std::to_string(i);
    til::declaration_node *argument_declaration = new til::declaration_node(lineno, tPRIVATE, intended_type->input(i), argument_name, nullptr);
    arguments = new cdk::sequence_node(lineno, argument_declaration, arguments);
    
    cdk::rvalue_node *argument_rvalue = new cdk::rvalue_node(lineno, new cdk::variable_node(lineno, argument_name));
    call_arguments = new cdk::sequence_node(lineno, argument_rvalue, call_arguments);
  }

  til::function_call_node *call = new til::function_call_node(lineno, aux_rvalue, call_arguments);
  til::return_node *return_node = new til::return_node(lineno, call);
  til::block_node *block = new til::block_node(lineno, new cdk::sequence_node(lineno), new cdk::sequence_node(lineno, return_node));
  til::function_node *aux_function = new til::function_node(lineno, arguments, intended_type->output(0), block);

  aux_function->accept(this, lvl);
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void til::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void til::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
}
void til::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl = ++_lbl));
  node->right()->accept(this, lvl);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}
void til::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl = ++ _lbl));
  node->right()->accept(this, lvl);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (inFunction()) {
    _pf.INT(node->value()); // push an integer
  } else {
    _pf.SINT(node->value());
  }
}


void til::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (inFunction()) {
    _pf.DOUBLE(node->value());
  } else {
    _pf.SDOUBLE(node->value());
  }
}

void til::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  if (inFunction()) {
    _pf.TEXT(_functionLabels.top());
    _pf.ADDR(mklbl(lbl));
  } else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl));
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl); // determine the value

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DNEG();
  else
    _pf.NEG();
}

void til::postfix_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_POINTER)) {
    std::shared_ptr<cdk::reference_type> ref = cdk::reference_type::cast(node->type());
    _pf.INT(std::max(ref->referenced()->size(), static_cast<size_t>(1)));
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  } else if (node->right()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_POINTER)) {
    std::shared_ptr<cdk::reference_type> ref = cdk::reference_type::cast(node->type());
    _pf.INT(std::max(ref->referenced()->size(), static_cast<size_t>(1)));
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DADD();
  } else {
    _pf.ADD();
  }
}

void til::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_POINTER)) {
    std::shared_ptr<cdk::reference_type> ref = cdk::reference_type::cast(node->type());
    _pf.INT(std::max(static_cast<size_t>(1), ref->referenced()->size()));
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    std::shared_ptr<cdk::reference_type> ref = cdk::reference_type::cast(node->type());
    _pf.INT(std::max(static_cast<size_t>(1), ref->referenced()->size()));
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DSUB();
  } else {
    _pf.SUB();
  }

  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    std::shared_ptr<cdk::reference_type> lref = cdk::reference_type::cast(node->left()->type());
    _pf.INT(std::max(static_cast<size_t>(1), lref->referenced()->size()));
    _pf.DIV();
  }
}

void til::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DMUL();
  else
    _pf.MUL();
}

void til::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DDIV();
  else
    _pf.DIV();
}

void til::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

void til::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.LT();
}

void til::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.LE();
}

void til::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.GE();
}

void til::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.GT();
}

void til::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.NE();
}

void til::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.EQ();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  
  auto symbol = _symtab.find(node->name());

  if (symbol->qualifier() == tEXTERNAL) {
    _externalFunctionName  = symbol->name();
  } else if (symbol->global()) {
    _pf.ADDR(node->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void til::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  
  if (_externalFunctionName) {
    return;
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  } else {
    _pf.LDINT();
  }
}

void til::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  acceptAndCast(node->type(), node->rvalue(), lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_program_node(til::program_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  _functionLabels.push("_main");

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");

  int oldOffset = _offset;
  _offset = 8;  // space for frame pointer and return address
  _symtab.push(); // enter a new scope

  frame_size_calculator lsc(_compiler, _symtab);
  node->statements()->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  std::string _oldFunctionReturnLabel = _functionReturnLabel;
  _functionReturnLabel = mklbl(++_lbl);

  std::vector<std::string> oldFunctionLoopConditionLabels = _functionLoopConditionLabels;
  std::vector<std::string> oldFunctionLoopEndLabels = _functionLoopEndLabels;

  _functionLoopConditionLabels.clear();
  _functionLoopEndLabels.clear();

  _offset = 0;

  node->statements()->accept(this, lvl);

  // end the main function
  _pf.INT(0);
  _pf.STFVAL32();
  _pf.ALIGN();
  _pf.LABEL(_functionReturnLabel);
  _pf.LEAVE();
  _pf.RET();

  _offset = oldOffset;
  _symtab.pop();
  _functionLoopConditionLabels = oldFunctionLoopConditionLabels;
  _functionLoopEndLabels = oldFunctionLoopEndLabels;
  _functionReturnLabel = _oldFunctionReturnLabel;
  _functionLabels.pop();

  for (std::string s : _externalFunctions) {
    _pf.EXTERN(s);
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_evaluation_node(til::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl);
  if (node->argument()->type()->size() > 0) {
    _pf.TRASH(node->argument()->type()->size());
  }
}

void til::postfix_writer::do_print_node(til::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  for (size_t i = 0; i < node->argument()->size(); i++) {
    auto expr = dynamic_cast<cdk::expression_node*>(node->argument()->node(i));

    expr->accept(this, lvl);

    if (expr->is_typed(cdk::TYPE_INT)) {
      _externalFunctions.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4);
    } else if (expr->is_typed(cdk::TYPE_DOUBLE)) {
      _externalFunctions.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8);
    } else if (expr->is_typed(cdk::TYPE_STRING)) {
      _externalFunctions.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4);
    }
  }

  if (node->newline()){
    _externalFunctions.insert("println");
    _pf.CALL("println");
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_read_node(til::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _externalFunctions.insert("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    _externalFunctions.insert("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_loop_node(til::loop_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  
  int condition_lbl, end_lbl;

  condition_lbl = ++_lbl;
  end_lbl = ++_lbl;
  _functionLoopConditionLabels.push_back(mklbl(condition_lbl));
  _functionLoopEndLabels.push_back(mklbl(end_lbl));

  _pf.ALIGN();
  _pf.LABEL(mklbl(condition_lbl));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(end_lbl));

  node->block()->accept(this, lvl + 2);

  _pf.JMP(mklbl(condition_lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(end_lbl));

  _functionLoopConditionLabels.pop_back();
  _functionLoopEndLabels.pop_back();

  _controlFlowAltered = false;
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_apply_node(til::apply_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto lineno = node->lineno();

  std::string _iter_name = "_iter_name";

  auto decl = new til::declaration_node(lineno, tPRIVATE, cdk::primitive_type::create(4, cdk::TYPE_INT), _iter_name, node->low());
  decl->apply(this, lvl + 4);

  auto iter_var = new cdk::variable_node(lineno, _iter_name);
  auto iter_rvalue = new cdk::rvalue_node(lineno, iter_var);
  auto condition = new cdk::le_node(lineno, iter_rvalue, node->high());

  auto _element = new til::ptr_index_node(lineno, node->vector(), iter_rvalue);
  auto element = new cdk::rvalue_node(lineno, _element);

  auto _function_node = til::function_call_node(lineno, node->function(), new cdk::sequence_node(lineno, element));
  auto function_call = new til::evaluation_node(lineno, _function_node);

  auto add = new cdk::add_node(lineno, iter_rvalue, new cdk::integer_node(lineno, 1));

  auto _increment = new cdk::assignment_node(lineno, iter_rvalue, add);
  auto increment = new til::evaluation_node(lineno, _increment);

  auto instrs = new cdk::sequence_node(lineno, increment, new cdk::sequence_node(lineno, function_call));

  auto block = new til::block_node(lineno, new cdk::sequence_node(lineno), instrs);

  auto loop = new til::loop_node(lineno, condition, block);
  loop->accept(this, lvl + 4);
}


//---------------------------------------------------------------------------

void til::postfix_writer::do_if_node(til::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _controlFlowAltered = false;
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
}

void til::postfix_writer::do_if_else_node(til::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _controlFlowAltered = false;
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _controlFlowAltered = false;
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1 = lbl2));
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_declaration_node(til::declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = new_symbol();
  reset_new_symbol();

  int offset = 0;
  int typesize = node->type()->size();
  if (_inFunctionArgs) {
    offset = _offset;
    _offset += typesize;
  } else if (inFunction()) {
    _offset -= typesize;
    offset = _offset;
  } else {
    offset = 0;
  }

  symbol->offset(offset);

  if (inFunction()) {
    if (_inFunctionArgs || node->initialValue() == nullptr) {
      return;
    }

    acceptAndCast(node->type(), node->initialValue(), lvl);
    if (node->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.LOCAL(symbol->offset());
      _pf.STDOUBLE();
    } else {
      _pf.LOCAL(symbol->offset());
      _pf.STINT();
    }

    return;
  }

  if (symbol->qualifier() == tFORWARD || symbol->qualifier() == tEXTERNAL) {
    _externalFunctions.insert(symbol->name());
    return;
  }

  _externalFunctions.erase(symbol->name());

  if (node->initialValue() == nullptr) {
    _pf.BSS();
    _pf.ALIGN();

    if (symbol->qualifier() == tPUBLIC) {
      _pf.GLOBAL(symbol->name(), _pf.OBJ());
    }

    _pf.LABEL(symbol->name());
    _pf.SALLOC(typesize);
    return;
  }

  _pf.DATA();
  _pf.ALIGN();

  if (symbol->qualifier() == tPUBLIC) {
    _pf.GLOBAL(symbol->name(), _pf.OBJ());
  }

  _pf.LABEL(symbol->name());

  if (node->is_typed(cdk::TYPE_DOUBLE) && node->initialValue()->is_typed(cdk::TYPE_INT)) {
    auto int_node = dynamic_cast<cdk::integer_node*>(node->initialValue());
    _pf.SDOUBLE(int_node->value());
  } else node->initialValue()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_function_call_node(til::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::shared_ptr<cdk::functional_type> func_type = 
    (node->identifier() == nullptr) ? 
    cdk::functional_type::cast(_symtab.find("@", 1)->type()) : 
    cdk::functional_type::cast(node->identifier()->type());

  int args_size = 0;

  // arguments are pushed right-to-left
  for (size_t i = node->arguments()->size(); i > 0; i--) {
    cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i - 1));
    acceptAndCast(func_type->input(i - 1), arg, lvl + 2);
    args_size += func_type->input(i - 1)->size();
  }

  _externalFunctionName = std::nullopt;
  if (node->identifier() == nullptr) {
    _pf.ADDR(_functionLabels.top());
  } else {
    node->identifier()->accept(this, lvl);
  }

    // Generate call instruction
  if (_externalFunctionName) {
    _pf.CALL(*_externalFunctionName);
    _externalFunctionName = std::nullopt;
  } else {
    _pf.BRANCH();
  }

  // Clean up arguments from stack
  if (args_size > 0) {
    _pf.TRASH(args_size);
  }

  // Load the function's return value
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else if (!node->is_typed(cdk::TYPE_VOID)) {
    _pf.LDFVAL32();
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_function_node(til::function_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::string newfunctionLabel = mklbl(++_lbl);
  _functionLabels.push(newfunctionLabel);

  _pf.TEXT();
  _pf.ALIGN();
  _pf.LABEL(_functionLabels.top());

  int oldOffset = _offset;
  _offset = 8;  // space for frame pointer and return address
  _symtab.push(); // enter a new scope

  _inFunctionArgs = true;
  node->arguments()->accept(this, lvl);
  _inFunctionArgs = false;

  frame_size_calculator lsc(_compiler, _symtab);
  node->block()->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  std::string _oldFunctionReturnLabel = _functionReturnLabel;
  _functionReturnLabel = mklbl(++_lbl);

  std::vector<std::string> oldFunctionLoopConditionLabels = _functionLoopConditionLabels;
  std::vector<std::string> oldFunctionLoopEndLabels = _functionLoopEndLabels;

  _functionLoopConditionLabels.clear();
  _functionLoopEndLabels.clear();

  _offset = 0;

  node->block()->accept(this, lvl);

  _pf.ALIGN();
  _pf.LABEL(_functionReturnLabel);
  _pf.LEAVE();
  _pf.RET();

  _functionReturnLabel = _oldFunctionReturnLabel;

  _offset = oldOffset;
  _functionLoopConditionLabels = oldFunctionLoopConditionLabels;
  _functionLoopEndLabels = oldFunctionLoopEndLabels;
  _functionLabels.pop();
  _symtab.pop();
  
  if (_inFunctionBody){
    _pf.TEXT(_functionLabels.top());
    _pf.ADDR(newfunctionLabel);
    return;
  }

  _pf.DATA();
  _pf.SADDR(newfunctionLabel);

}

//---------------------------------------------------------------------------

void til::postfix_writer::do_return_node(til::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto symbol = _symtab.find("@", 1);
  std::shared_ptr<cdk::basic_type> return_type = cdk::functional_type::cast(symbol->type())->output(0);

  if (return_type->name() != cdk::TYPE_VOID) {
    acceptAndCast(return_type, node->value(), lvl + 2);

    if (return_type->name() == cdk::TYPE_DOUBLE) {
      _pf.STFVAL64();
    } else {
      _pf.STFVAL32();
    }
  }

  _pf.JMP(_functionReturnLabel);

  _controlFlowAltered = true;
}

//---------------------------------------------------------------------------
void til::postfix_writer::handleLoopControlInstruction(int level, const std::vector<std::string>& labels,
                                                          const std::string& instructionName) {
  if (level <= 0) {
    std::cerr << "ERROR: Invalid " << instructionName << " instruction level" << std::endl;
    exit(1);
  }

  if (labels.size() < static_cast<size_t>(level)) {
    std::cerr << "ERROR: Insufficient loop labels for " << instructionName << " instruction" << std::endl;
    exit(1);
  }

  auto index = labels.size() - static_cast<size_t>(level);
  auto label = labels[index];
  _pf.JMP(label);

  _controlFlowAltered = true;
}

void til::postfix_writer::do_next_node(til::next_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  handleLoopControlInstruction(node->level(), _functionLoopConditionLabels, "next");
}

void til::postfix_writer::do_stop_node(til::stop_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  handleLoopControlInstruction(node->level(), _functionLoopEndLabels, "stop");
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_block_node(til::block_node * const node, int lvl) {
  _symtab.push();

  node->declarations()->accept(this, lvl + 2);

  _controlFlowAltered = false;
  for (size_t i = 0; i < node->instructions()->size(); i++) {
    auto instr = node->instructions()->node(i);

    if (_controlFlowAltered)
      throw std::string("found instructions after a final instruction");

    instr->accept(this, lvl + 2);
  }
  _controlFlowAltered = false;

  _symtab.pop();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_sizeof_node(til::sizeof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  _pf.INT(node->expression()->type()->size());
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_objects_node(til::objects_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto referenced = cdk::reference_type::cast(node->type())->referenced();
  node->argument()->accept(this, lvl);

  _pf.INT(referenced->size());
  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_null_ptr_node(til::null_ptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_ptr_index_node(til::ptr_index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);
  _pf.INT(node->type()->size());
  _pf.MUL();
  _pf.ADD();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_address_of_node(til::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl + 2);
}
