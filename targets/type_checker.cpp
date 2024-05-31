#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#include "til_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

bool til::type_checker::typecmp(std::shared_ptr<cdk::basic_type> left, std::shared_ptr<cdk::basic_type> right, bool allowCovariance) {
  if (left->name() == cdk::TYPE_UNSPEC || right->name() == cdk::TYPE_UNSPEC) {
    return false;
  }

  else if (left->name() == cdk::TYPE_FUNCTIONAL && right->name() == cdk::TYPE_FUNCTIONAL) {
    auto left_func = cdk::functional_type::cast(left);
    auto right_func = cdk::functional_type::cast(right);

    if (left_func->input_length() != right_func->input_length()
        || left_func->output_length() != right_func->output_length())
      return false;

    for (size_t i = 0; i < left_func->input_length(); i++)
      if (!typecmp(right_func->output(i), left_func->output(i), allowCovariance))
        return false;

    for (size_t i = 0; i < left_func->output_length(); i++)
      if (!typecmp(left_func->input(i), right_func->input(i), allowCovariance))
        return false;

    return true;
  }

  else if (left->name() == cdk::TYPE_POINTER && right->name() == cdk::TYPE_POINTER) {
    auto left_ref = cdk::reference_type::cast(left);
    auto right_ref = cdk::reference_type::cast(right);

    return typecmp(left_ref->referenced(), right_ref->referenced(), false);
  }

  else {
    return (allowCovariance && left->name() == cdk::TYPE_DOUBLE && right->name() == cdk::TYPE_INT)
           || (left == right
               && left->name() != cdk::TYPE_FUNCTIONAL && right->name() != cdk::TYPE_FUNCTIONAL
               && left->name() != cdk::TYPE_POINTER && right->name() != cdk::TYPE_POINTER);
  }
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

  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->argument()->is_typed(cdk::TYPE_INT)
             && !(allowDoubles && node->argument()->is_typed(cdk::TYPE_DOUBLE)))
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

  if (symbol != nullptr)
    node->type(symbol->type());
  else
    throw std::string("undeclared variable '" + id + "'");
}

void til::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void til::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl);
  node->rvalue()->accept(this, lvl + 2);

  if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    node->rvalue()->type(node->lvalue()->type());
  } else if (node->rvalue()->is_typed(cdk::TYPE_POINTER) && node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    auto lvalue_ref = cdk::reference_type::cast(node->lvalue()->type());
    auto rvalue_ref = cdk::reference_type::cast(node->rvalue()->type());

    if (lvalue_ref->referenced()->name() == cdk::TYPE_VOID
        || rvalue_ref->referenced()->name() == cdk::TYPE_VOID
        || rvalue_ref->referenced()->name() == cdk::TYPE_UNSPEC) {
      node->rvalue()->type(node->lvalue()->type());
    }
  }

  if (!typecmp(node->lvalue()->type(), node->rvalue()->type(), true))
    throw std::string("wrong type in right value of assignment");

  node->type(node->lvalue()->type());
}

//---------------------------------------------------------------------------

void til::type_checker::do_program_node(til::program_node *const node, int lvl) {
  auto symbol = std::make_shared<til::symbol>(node->type(), "@", 0);

  if (!_symtab.insert(symbol->name(), symbol)) {
    throw std::string("function symbol is redeclared");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_evaluation_node(til::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl);

  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->argument()->is_typed(cdk::TYPE_POINTER)) {
    auto arg_ref = cdk::reference_type::cast(node->argument()->type());
    if (arg_ref != nullptr && arg_ref->referenced()->name() == cdk::TYPE_UNSPEC) {
      node->argument()->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
    }
  }
}

void til::type_checker::do_print_node(til::print_node *const node, int lvl) {
  for (size_t i = 0; i < node->argument()->size(); i++) {
    auto expr = dynamic_cast<cdk::expression_node*>(node->argument()->node(i));

    expr->accept(this, lvl);

    if (expr->is_typed(cdk::TYPE_UNSPEC)) {
      expr->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (!expr->is_typed(cdk::TYPE_INT)
               && !expr->is_typed(cdk::TYPE_DOUBLE)
               && !expr->is_typed(cdk::TYPE_STRING))
      throw std::string("wrong type in argument of print instruction");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_read_node(til::read_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void til::type_checker::do_loop_node(til::loop_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("wrong type in condition of loop instruction");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_if_node(til::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("wrong type in condition of conditional instruction");
  }
}

void til::type_checker::do_if_else_node(til::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("wrong type in condition of conditional instruction");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_declaration_node(til::declaration_node *const node, int lvl) {
  if (node->type() == nullptr) { // var
    node->initialValue()->accept(this, lvl + 2);

    if (node->initialValue()->is_typed(cdk::TYPE_UNSPEC)) {
      node->initialValue()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->initialValue()->is_typed(cdk::TYPE_POINTER)) {
      std::shared_ptr<cdk::reference_type> init_ref = cdk::reference_type::cast(node->initialValue()->type());
      if (init_ref->referenced()->name() == cdk::TYPE_UNSPEC) {
        node->initialValue()->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
      }
    } else if (node->initialValue()->is_typed(cdk::TYPE_VOID))
      throw std::string("cannot assign void to variable");

    node->type(node->initialValue()->type());
  }
  else { // not var
    if (node->initialValue() != nullptr) {
      node->initialValue()->accept(this, lvl + 2);

      if (node->initialValue()->is_typed(cdk::TYPE_UNSPEC)) {
        if (node->is_typed(cdk::TYPE_DOUBLE)) {
          node->initialValue()->type(node->type());
        } else {
          node->initialValue()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        }
      } else if (node->initialValue()->is_typed(cdk::TYPE_POINTER)) {
        std::shared_ptr<cdk::reference_type> node_ref = cdk::reference_type::cast(node->type());
        std::shared_ptr<cdk::reference_type> init_ref = cdk::reference_type::cast(node->initialValue()->type());

        if (node_ref->referenced()->name() == cdk::TYPE_VOID
            || init_ref->referenced()->name() == cdk::TYPE_VOID
            || init_ref->referenced()->name() == cdk::TYPE_UNSPEC) {
          node->initialValue()->type(node->type());
        }
      }

      if (!typecmp(node->type(), node->initialValue()->type(), true))
        throw std::string("wrong type in initial value of declaration");
    }
  }

  if (node->qualifier() == tEXTERNAL && !node->is_typed(cdk::TYPE_FUNCTIONAL))
    throw std::string("external declaration of non-function");
  
  auto symbol = std::make_shared<til::symbol>(node->type(), node->identifier(), node->qualifier());
  if (_symtab.insert(node->identifier(), symbol)) {
    _parent->set_new_symbol(symbol);
    return;
  }

  auto previous = _symtab.find(node->identifier());
  if (previous != nullptr && previous->qualifier() == tFORWARD) {
    if (typecmp(previous->type(), symbol->type(), false)) {
      _symtab.replace(node->identifier(), symbol);
      _parent->set_new_symbol(symbol);
      return;
    }
  }
  
  throw std::string("redeclaration of '" + node->identifier() + "'");
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
  auto symbol = _symtab.find("@", 1);
  if (symbol == nullptr)
    throw std::string("return statement outside program");

  std::shared_ptr<cdk::functional_type> function_type = cdk::functional_type::cast(symbol->type());
  std::shared_ptr<cdk::basic_type> return_type = function_type->output(0);

  if (node->value() == nullptr) { // return statement without expression
    if (return_type->name() != cdk::TYPE_VOID)
      throw std::string("no return value but non-void return type");
    else
      return; // no returned value and void return type
  }
  else { // return statement with expression
    if (return_type->name() == cdk::TYPE_VOID)
      throw std::string("void return type but returned value");
  }

  // non-void return type and returned value
  node->value()->accept(this, lvl + 2);

  if (!typecmp(return_type, node->value()->type(), true))
    throw std::string("wrong type of returned expression");
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
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl + 2);

  if (node->expression()->is_typed(cdk::TYPE_UNSPEC)) {
    node->expression()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void til::type_checker::do_objects_node(til::objects_node *const node, int lvl) {
  processUnaryExpression(node, lvl, false);
}

//---------------------------------------------------------------------------

void til::type_checker::do_null_ptr_node(til::null_ptr_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void til::type_checker::do_ptr_index_node(til::ptr_index_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->base()->accept(this, lvl + 2);
  if (!node->base()->is_typed(cdk::TYPE_POINTER))
    throw std::string("expected pointer type in base");

  node->index()->accept(this, lvl + 2);
  if (node->index()->is_typed(cdk::TYPE_UNSPEC)) {
    node->index()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("expected integer in index");

  auto base_type = cdk::reference_type::cast(node->base()->type());

  if (base_type->referenced()->name() == cdk::TYPE_UNSPEC) {
    base_type = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->base()->type(base_type);
  }

  node->type(base_type->referenced());
}

//---------------------------------------------------------------------------

void til::type_checker::do_address_of_node(til::address_of_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl + 2);

  if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    auto ref_cast = cdk::reference_type::cast(node->lvalue()->type());
    // if lvalue is void!, then its address is void! as well
    if (ref_cast->referenced()->name() == cdk::TYPE_VOID) {
      node->type(node->lvalue()->type());
      return;
    }
  }

  // else, if lvalue is type!, then its address if type!!
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}
