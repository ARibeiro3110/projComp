#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

bool til::type_checker::typecmp(std::shared_ptr<cdk::basic_type> left, std::shared_ptr<cdk::basic_type> right, bool covariant) {
  // TODO: implement
  return false;
}

//---------------------------------------------------------------------------

void til::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void til::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void til::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl, false);
}
void til::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, false, false);
}
void til::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, false, false);
}

//---------------------------------------------------------------------------

void til::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void til::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void til::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl, bool allowDoubles) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl + 2);

  if (!node->argument()->is_typed(cdk::TYPE_INT)
      && !(allowDoubles && node->argument()->is_typed(cdk::TYPE_DOUBLE))
      && !node->argument()->is_typed(cdk::TYPE_UNSPEC))
    throw std::string("wrong type in argument of unary expression");

  node->type(node->argument()->type());
}

void til::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl, true);
}

void til::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl, true);
}

//---------------------------------------------------------------------------

void til::type_checker::processBinaryArithmeticExpression(cdk::binary_operation_node *const node, int lvl,
                                                          bool allowDoubles, bool allowOnePointer, bool allowTwoPointers) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(node->left()->type());
      node->type(node->left()->type());
    }
    else if (node->right()->is_typed(cdk::TYPE_INT)
             || (allowDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))) {
      node->type(node->right()->type());
    }
    else if (allowOnePointer && node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->right()->type());
    }
    else {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else if (allowDoubles && node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(node->left()->type());
      node->type(node->left()->type());
    }
    else if (node->right()->is_typed(cdk::TYPE_INT)
             || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(node->left()->type());
    }
    else {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else if (allowOnePointer && node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->type(node->left()->type());
    }
    else if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    }
    else if (allowTwoPointers
             && typecmp(node->left()->type(), node->right()->type(), false)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else if (node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->right()->type(node->left()->type());
      node->type(node->left()->type());
    }
    else if (node->right()->is_typed(cdk::TYPE_INT)
             || (allowDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))) {
      node->left()->type(node->right()->type());
      node->type(node->right()->type());
    }
    else if (allowOnePointer && node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->type(node->right()->type());
    }
    else {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else {
    throw std::string("wrong type in left argument of binary expression");
  }
}

void til::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, true, false);
}
void til::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, true, true);
}
void til::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, false, false);
}
void til::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, false, false);
}
void til::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, false, false, false);
}

void til::type_checker::processBinaryBooleanExpression(cdk::binary_operation_node *const node, int lvl,
                                                       bool allowDoubles, bool allowPointers) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(node->left()->type());
    }
    else if (!node->right()->is_typed(cdk::TYPE_INT)
             && !(allowDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))
             && !(allowPointers && node->right()->is_typed(cdk::TYPE_POINTER))) {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else if (allowDoubles && node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(node->left()->type());
    }
    else if (!node->right()->is_typed(cdk::TYPE_INT)
             && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else if (allowPointers && node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else if (!node->right()->is_typed(cdk::TYPE_INT)
             && !node->right()->is_typed(cdk::TYPE_POINTER)) {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else if (node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else if (node->right()->is_typed(cdk::TYPE_INT)
             || (allowDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))) {
      node->left()->type(node->right()->type());
    }
    else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else {
      throw std::string("wrong type in right argument of binary expression");
    }
  }

  else {
    throw std::string("wrong type in left argument of binary expression");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, true, false);
}
void til::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, true, false);
}
void til::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, true, false);
}
void til::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, true, false);
}
void til::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, true, true);
}
void til::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryBooleanExpression(node, lvl, true, true);
}

//---------------------------------------------------------------------------

void til::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<til::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void til::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void til::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    auto symbol = std::make_shared<til::symbol>(cdk::primitive_type::create(4, cdk::TYPE_INT), id, 0);
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl);  //DAVID: bah!
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of assignment expression");

  // in Simple, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void til::type_checker::do_program_node(til::program_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_evaluation_node(til::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void til::type_checker::do_print_node(til::print_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_read_node(til::read_node *const node, int lvl) {
  try {
    // node->argument()->accept(this, lvl); // FIXME: commented to compile
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_loop_node(til::loop_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void til::type_checker::do_if_node(til::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void til::type_checker::do_if_else_node(til::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void til::type_checker::do_declaration_node(til::declaration_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_function_call_node(til::function_call_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_function_node(til::function_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_return_node(til::return_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_next_node(til::next_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_stop_node(til::stop_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_block_node(til::block_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_sizeof_node(til::sizeof_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_objects_node(til::objects_node *const node, int lvl) {
  processUnaryExpression(node, lvl, false);
}

//---------------------------------------------------------------------------

void til::type_checker::do_null_ptr_node(til::null_ptr_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_ptr_index_node(til::ptr_index_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_address_of_node(til::address_of_node *const node, int lvl) {
  // EMPTY
}
