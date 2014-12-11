
#include "eval.hpp"
#include "ast.hpp"
#include "scope.hpp"
#include "type.hpp"
#include "value.hpp"
#include "subst.hpp"
#include "boost/filesystem.hpp"
#include "lang/debug.hpp"

#include <iostream>
#include <unistd.h>

// -------------------------------------------------------------------------- //
// Evaluator class

Term*
Evaluator::operator()(Term* t) {
  return eval(t);
}


// -------------------------------------------------------------------------- //
// Multi-step evaluation
//
// The following function computes the multi-step evaluation (or
// simply evaluation) of a term t. Note that the evaluation is reflexive, 
// meaning that the evaluation of a value (or normal form) is simply
// an identity operation.

Term* eval(Term*);

namespace {

// Compute the multistep evaluation of an if term
//
//             t1 ->* true
//    ---------------------------- E-if-true
//    if t1 then t2 else t3 ->* t2
//
//             t1 ->* true
//    ---------------------------- E-if-false
//    if t1 then t2 else t3 ->* t2
Term*
eval_if(If* t) {
  Term* bv = eval(t->cond());
  if (is_true(bv))
    return eval(t->if_true());
  if (is_false(bv))
    return eval(t->if_false());
  lang_unreachable(format("'{}' is not a boolean value", pretty(bv)));
}

// Compute the multi-step evaluation of a successor term.
//
//         t ->* n
//    ---------------- E-succ
//    succ t ->* n + 1
//
// Here, 'n' is an integer value.
Term*
eval_succ(Succ* t) {
  Term* t1 = eval(t->arg());
  if (Int* n = as<Int>(t1)) {
    const Integer& z = n->value();
    return new Int(t->loc, get_type(t), z + 1);
  }
  lang_unreachable(format("'{}' is not a numeric value", pretty(t1)));
}


//Evaluate a ls term. 
Term*
eval_ls(Ls* t) {
  Term* t1 = eval(t->t1);
  Term_seq* op=new Term_seq();
  if (Str* n = as<Str>(t1))
   {
    String z = n->value();
    String k=sub_str(z);
    boost::filesystem::path p = k.str();
    Type* type=new Path_type(get_path_type());
    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
    {   
      {
        String z1=itr->path().string();
      	op->push_back(new Path(get_path_type(),z1));
      }
    }
    return new List(new List_type(get_path_type(),type),op);
   }
  lang_unreachable(format("'{}' is not a string value", pretty(t1)));
}

// Mkdir Evaluation
Term*
eval_mkdir(Mkdir* t) {
  Term* t1 = eval(t->t1);
  if (Str* n = as<Str>(t1))
   {
    String z = n->value();
    String k=sub_str(z);
    boost::filesystem::path p = k.str();
    if(!boost::filesystem::is_directory(p))
    {
    boost::filesystem::create_directories(p);
   	return get_true();
    }
    else
    return get_false();
   }
  lang_unreachable(format("'{}' is not a string value", pretty(t1)));
}


// Rmdir Evaluation
Term*
eval_rmdir(Rmdir* t) {
  Term* t1 = eval(t->t1);
  if (Str* n = as<Str>(t1))
   {
    String z = n->value();
    String k=sub_str(z);
    boost::filesystem::path p = k.str();
    if(is_directory(p))
    {
    boost::filesystem::remove_all(p);
    return  get_true();
    }
   else
   	return  get_false();
   }
  lang_unreachable(format("'{}' is not a string value", pretty(t1)));
}

// Rmdir Evaluation
Term*
eval_cd(Cd* t) {
  Term* t1 = eval(t->t1);
  if (Str* n = as<Str>(t1))
   {
    String z = n->value();
    String k=sub_str(z);
    boost::filesystem::path p = k.str();
    if(is_directory(p))
    {
    chdir(p.string().data());
    std::cout<<boost::filesystem::current_path()<<'\n';
    return get_true();
    }
    else
    return  get_false();
   }
  lang_unreachable(format("'{}' is not a string value", pretty(t1)));
}


// Rename Evaluation
Term*
eval_mv(Mv* t) {
  Term* t1 = eval(t->t1);
  Term* t2 = eval(t->t2);
  if (Str* n = as<Str>(t1))
   {
    String z = n->value();
    String k=sub_str(z);
    if(Str* n1=as<Str>(t2))
    {
    String z1 = n1->value();
    String k1 =sub_str(z1);
    boost::filesystem::path p  = k.str();
    boost::filesystem::path p1 = k1.str();
    boost::filesystem::rename(p,p1);
    return  get_true();
   }
   return get_false();
  }
  lang_unreachable(format("'{}' is not a string value", pretty(t1)));
}

// Evalutae a predecessor term.
//
//      t ->* 0
//    ------------ E-pred-0
//    pred t ->* 0
//
//         t ->* n
//    ---------------- E-pred-succ
//    pred t ->* n - 1
//
// Here, 'n' is an integer value.
Term*
eval_pred(Pred* t) {
  Term* t1 = eval(t->arg());
  if (Int* n = as<Int>(t1)) {
    const Integer& z = n->value();
    if (z == 0)
      return n;
    else
      return new Int(t->loc, get_type(t), z - 1);
  }
  lang_unreachable(format("'{}' is not a numeric value", pretty(t1)));
}

// Evaluate an iszero term.
//
//         t ->* 0
//    ----------------- E-iszero-0
//    iszero t ->* true
//
//         t ->* n
//    ------------------ E-iszero-succ
//    iszero t ->* false
Term*
eval_iszero(Iszero* t) {
  Term* t1 = eval(t->arg());
  if (Int* n = as<Int>(t1)) {
    const Integer& z = n->value();
    if (z == 0)
      return new True(t->loc, get_bool_type());
    else
      return new False(t->loc, get_bool_type());
  }
  lang_unreachable(format("'{}' is not a numeric value", pretty(t1)));
}

// Evaluate an application.
//
//        t1 ->* \x:T.t
//    --------------------- E-app-1
//    t1 t2 ->* (\x:T.t) t2
//
//          t2 ->* v
//    --------------------- E-app-2
//    \x:T.t t2 ->* [x->v]t
Term*
eval_app(App* t) {
  Abs* fn = as<Abs>(eval(t->abs())); // E-app-1
  lang_assert(fn, format("ill-formed application target '{}'", pretty(t->abs())));

  Term* arg = eval(t->arg()); // E-app-2
    
  // Perform a beta reduction and evaluate the result.
  Subst sub {fn->var(), arg};
  Term* res = subst_term(fn->term(), sub);
  return eval(res);
}

// Evaluate a function call. This is virtually identical to
// application except that all arguments are evaluated in turn.
//
// TODO: Document the semantics of these operations.
Term*
eval_call(Call* t) {
  // Evaluate the function.
  Fn* fn = as<Fn>(eval(t->fn()));
  lang_assert(fn, format("ill-formed call target '{}'", pretty(t->fn())));

  // Evaluate arguments in place. That is, we're not creating
  // a new sequence of arguments, just replacing the entries
  // in the existing sequence.
  Term_seq* args = t->args();
  for (Term*& a : *args)
    a = eval(a);

  // Beta reduce and evaluate.
  Subst sub {fn->parms(), args};
  Term* result = subst_term(fn->term(), sub);
  return eval(result);
}

// Elaborate a declaration reference. When the reference
// is to a definition, replace it with the definition's value.
// Otherwise, preserve the reference.
//
// If the reference is to a type, then we can't evaluate this.
// Just return nullptr and hope that the caller knows how to
// handle the results.
Term*
eval_ref(Ref* t) {
  if (Def* def = as<Def>(t->decl())) {
    if (Term* replace = as<Term>(def->value()))
      return replace;
    else
      return nullptr;
  } else {
    return t;
  }
}

// Evaluate the definition by evaluating the defined term. When the
// definition's value is not a term, then there isn't anything
// interesting that we can do. Just return the value.
Term*
eval_def(Def* t) {
  if (Term* t0 = as<Term>(t->value())) {
    // This is a little weird. We're actually going to update
    // the defined term with its evaluated initializer. We do this
    // because other expressions may already refer to t and we don't
    // really want to re-resolve all of those things.
    //
    // Note that we could choose to do this during elaboration
    // in order to avoid the weirdness.
    t->t2 = eval(t0);
  }
  return t;
}

// Elaborate a print statement.
//
//          t ->* v
//    ------------------- E-print-term
//    print t ->* print v
//
//    --------------- E-print-value
//    print v -> unit
//
//    --------------- E-print-type
//    print T -> unit
//
Term*
eval_print(Print* t) {
  // Try to evaluate the expression.
  Term* val = nullptr;
  if (Term* term = as<Term>(t->expr()))
    val = eval(term);

  // Print the result, or if the expression is not
  // evaluable, just print the expression.
  if (val)
    std::cout << pretty(val) << '\n';
  else
    std::cout << pretty(t->expr()) << '\n';

  return new Unit(t->loc, get_unit_type());
}

// FIXME: Actually evaluate each expression in turn.
Term*
eval_comma(Comma* t) {
  return get_unit();
}

// Evaluate each statement in turn; the result of the program is
// the result of the last statemnt. 
//
//    for each i ei ->* vi
//    -------------------- E-prog
//     e1; ...; en ->* vn
Term*
eval_prog(Prog* t) {
  Term* tn;
  for (Term* ti : *t->stmts())
    tn = eval(ti);
  return tn;
}

} // namespace

// Compute the multi-step evaluation of the term t. 
Term*
eval(Term* t) {
  switch (t->kind) {
  case if_term: return eval_if(as<If>(t));
  case succ_term: return eval_succ(as<Succ>(t));
  case ls_term: return eval_ls(as<Ls>(t));
  case mkdir_term: return eval_mkdir(as<Mkdir>(t));
  case rmdir_term: return eval_rmdir(as<Rmdir>(t));
  case cd_term: return eval_cd(as<Cd>(t));
  case mv_term: return eval_mv(as<Mv>(t));
  case pred_term: return eval_pred(as<Pred>(t));
  case iszero_term: return eval_iszero(as<Iszero>(t));
  case app_term: return eval_app(as<App>(t));
  case call_term: return eval_call(as<Call>(t));
  case ref_term: return eval_ref(as<Ref>(t));
  case print_term: return eval_print(as<Print>(t));
  case def_term: return eval_def(as<Def>(t));
  case prog_term: return eval_prog(as<Prog>(t));
  case comma_term: return eval_comma(as<Comma>(t));
  default: break;
  }
  return t;
}


// Compute the one-step evaluation of the term t.
Term*
step(Term* t) {
  lang_unreachable("not implemented");
}

