#include <string>
#include "targets/xml_writer.h"
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated

//---------------------------------------------------------------------------

void til::xml_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<"  << node->label() << ">"
       << "</" << node->label() << ">" << std::endl;
}
void til::xml_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<"  << node->label()
       << " size='" << node->size() << "'>"
       << "</" << node->label() << ">" << std::endl;
}
void til::xml_writer::do_not_node(cdk::not_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}
void til::xml_writer::do_and_node(cdk::and_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_or_node(cdk::or_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " size='" << node->size() << "'>" << std::endl;
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  process_literal(node, lvl);
}

void til::xml_writer::do_double_node(cdk::double_node * const node, int lvl) {
  process_literal(node, lvl);
}

void til::xml_writer::do_string_node(cdk::string_node * const node, int lvl) {
  process_literal(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_unary_operation(cdk::unary_operation_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void til::xml_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

void til::xml_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_binary_operation(cdk::binary_operation_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void til::xml_writer::do_add_node(cdk::add_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_div_node(cdk::div_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_le_node(cdk::le_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void til::xml_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<"  << node->label()
       << " name='" << node->name() << "'>"
       << "</" << node->label() << ">" << std::endl;
}

void til::xml_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->lvalue()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void til::xml_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);

  node->lvalue()->accept(this, lvl + 2);
  reset_new_symbol();

  node->rvalue()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_program_node(til::program_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->statements()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_evaluation_node(til::evaluation_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void til::xml_writer::do_print_node(til::print_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " newline='" << bool_to_str(node->newline()) << "'>" << std::endl;
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_read_node(til::read_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_loop_node(til::loop_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("block", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("block", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_if_node(til::if_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("then", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("then", lvl + 2);
  closeTag(node, lvl);
}

void til::xml_writer::do_if_else_node(til::if_else_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("then", lvl + 2);
  node->thenblock()->accept(this, lvl + 4);
  closeTag("then", lvl + 2);
  openTag("else", lvl + 2);
  node->elseblock()->accept(this, lvl + 4);
  closeTag("else", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_declaration_node(til::declaration_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " qualifier='" << qualifier_to_str(node->qualifier()) << "'"
       << " identifier='" << node->identifier() << "'>" << std::endl;

  if (node->initialValue() != nullptr) {
    openTag("initialValue", lvl + 2);
    node->initialValue()->accept(this, lvl + 4);
    closeTag("initialValue", lvl + 2);
  }

  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_function_call_node(til::function_call_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  os() << std::string(lvl, ' ') << "<" << node->label();

  if (node->identifier() != nullptr) {
    os() << ">" << std::endl;
    openTag("identifier", lvl + 2);
    node->identifier()->accept(this, lvl + 4);
    closeTag("identifier", lvl + 2);
  } else
    os() << std::string(lvl + 2, ' ') << " identifier='@'>" << std::endl; // TODO

  openTag("arguments", lvl + 2);
  node->arguments()->accept(this, lvl + 4);
  closeTag("arguments", lvl + 2);

  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_function_node(til::function_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  openTag(node, lvl);
  openTag("arguments", lvl + 2);
  node->arguments()->accept(this, lvl + 4);
  closeTag("arguments", lvl + 2);
  openTag("block", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("block", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_return_node(til::return_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  if (node->value() == nullptr) {
    os() << std::string(lvl, ' ') << "<"  << node->label() << ">"
         << "</" << node->label() << ">" << std::endl;
  } else {
    openTag(node, lvl);
    node->value()->accept(this, lvl + 2);
    closeTag(node, lvl);
  }
}

//---------------------------------------------------------------------------

void til::xml_writer::do_next_node(til::next_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  os() << std::string(lvl, ' ') << "<"  << node->label()
       << " level='" << node->level() << "'>"
       << "</" << node->label() << ">" << std::endl;
}

//---------------------------------------------------------------------------

void til::xml_writer::do_stop_node(til::stop_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  os() << std::string(lvl, ' ') << "<"  << node->label()
       << " level='" << node->level() << "'>"
       << "</" << node->label() << ">" << std::endl;
}

//---------------------------------------------------------------------------

void til::xml_writer::do_block_node(til::block_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  openTag(node, lvl);
  openTag("declarations", lvl + 2);
  node->declarations()->accept(this, lvl + 4);
  closeTag("declarations", lvl + 2);
  openTag("instructions", lvl + 2);
  node->instructions()->accept(this, lvl + 4);
  closeTag("instructions", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_sizeof_node(til::sizeof_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  openTag(node, lvl);
  node->expression()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_objects_node(til::objects_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_null_ptr_node(til::null_ptr_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  os() << std::string(lvl, ' ') << "<"  << node->label() << ">"
       << "</" << node->label() << ">" << std::endl;
}

//---------------------------------------------------------------------------

void til::xml_writer::do_ptr_index_node(til::ptr_index_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  openTag(node, lvl);
  openTag("base", lvl + 2);
  node->base()->accept(this, lvl + 4);
  closeTag("base", lvl + 2);
  openTag("index", lvl + 2);
  node->index()->accept(this, lvl + 4);
  closeTag("index", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void til::xml_writer::do_address_of_node(til::address_of_node * const node, int lvl) {
  // TODO: ASSERT_SAFE_EXPRESSIONS
  openTag(node, lvl);
  node->lvalue()->accept(this, lvl + 2);
  closeTag(node, lvl);
}
