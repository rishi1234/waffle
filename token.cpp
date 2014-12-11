
#include "token.hpp"

#include "lang/debug.hpp"

void
init_tokens() {
  // Keywords
  init_token(def_tok, "def");
  init_token(else_tok, "else");
  init_token(false_tok, "false");
  init_token(if_tok, "if");
  init_token(iszero_tok, "iszero");
  init_token(print_tok, "print");
  init_token(pred_tok, "pred");
  init_token(succ_tok, "succ");
  init_token(then_tok, "then");
  init_token(true_tok, "true");
  init_token(ls_tok, "ls");//rishi
  init_token(mkdir_tok, "mkdir");//rishi
  init_token(rmdir_tok, "rmdir");//rishi
  init_token(cd_tok, "cd");//rishi
  init_token(mv_tok, "mv");//rishi
  init_token(typeof_tok, "typeof");
  init_token(unit_tok, "unit");
  // Type names
  init_token(bool_type_tok, "Bool");
  init_token(nat_type_tok, "Nat");
  init_token(unit_type_tok, "Unit");
  // Identifiers and literals
  init_token(identifier_tok, "identifier");
  init_token(decimal_literal_tok, "decimal");
}
