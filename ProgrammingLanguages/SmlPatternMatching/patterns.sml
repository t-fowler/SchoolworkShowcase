(*
 * Your name: Tyler Fowler
 * Your student id: V00752565
 *)

structure Patterns =

struct

exception NoAnswer
exception NotFound

datatype tree = emptyTree |
                nodeTree of int * tree * tree


datatype pattern = Wildcard
		 | Variable of string
		 | UnitP
		 | ConstP of int
		 | TupleP of pattern list
		 | ConstructorP of string * pattern

datatype value = Const of int
	       | Unit
	       | Tuple of value list
	       | Constructor of string * value


(* write your tree functions here *)

(*
 * Insert in order. If the node is duplicated, insert to the left. This
 * is a recursive function.
 *)
fun tree_insert_in_order(t, v) =
  case t of
    emptyTree => 
      nodeTree (v, emptyTree, emptyTree)
  | nodeTree (n, left, right) =>
    (
      case Int.compare(v, n) of
        GREATER => nodeTree(n, left, tree_insert_in_order(right, v))
      | _ => nodeTree(n, tree_insert_in_order(left, v), right)
    )

(* 
 * Write a “fold” function that traverses the tree in preorder
 * (node, left children, right children) "folding" the tree using the function f. Use acc as the starting
 * value. 
 *)
fun tree_fold_pre_order f acc t =
    case t of
      emptyTree => acc
    | nodeTree (n, emptyTree, emptyTree) => f(n, acc)
    | nodeTree (n, emptyTree, right) => tree_fold_pre_order f (f(n, acc)) right
    | nodeTree (n, left, emptyTree) => tree_fold_pre_order f (f(n, acc)) left
    | nodeTree (n, left, right) => tree_fold_pre_order f (tree_fold_pre_order f (f(n, acc)) left) right

(* 
 * Find the maximum value in the tree (returns an option). Use a val expression and
 * tree_fold_pre_order to write this function.I know, I know, this is the most inefficient way to
 * find the maximum, and I agree; we are learning functional programming, not algorithms. Restriction:
 * define it using val and use tree_fold_pre_order
 *)
val tree_max = 
  let
    fun max (n, NONE) = SOME n
    |   max (n, a) = SOME (Int.max(n, valOf a))
  in
    tree_fold_pre_order max NONE
  end

(* 
 * First, find the node to delete (if v appears more than once, find the first instance).
 * If v does not exist, raise NotFound. Second, if the node to delete has one child only, then
 * simply delete the node and reconnect the child to the parent; otherwise, find the maximum node in the
 * left child, remove it from this subtree, and create a note to replace the one you are deleting (with the
 * new subtree as its left child). Use tree_max and a recursive call to tree_delete. 
 *)
fun tree_delete(t, v) =
  case t of
    emptyTree => raise NotFound
  | nodeTree (n, left, right) =>
    (
      case Int.compare(v, n) of
       LESS => nodeTree (n, tree_delete(left, v), right)
      | GREATER => nodeTree (n, left, tree_delete(right, v))
      | EQUAL => 
        (
          case (left, right) of
            (emptyTree, emptyTree) => emptyTree
          | (emptyTree, _) => right
          | (_, emptyTree) => left
          | (_, _) => let val max = valOf (tree_max left) in nodeTree (max, tree_delete(left, max), right) end
        )
    )

(*
 * Return the maximum height of the tree. 0 for EmptyTree. Use recursion.
 *)
fun tree_height t =
  case t of
    emptyTree => 0
  | nodeTree(n, left, right) => 1 + (Int.max(tree_height left, tree_height right))

(*
 * Convert the tree to a list, in pre-order. Restriction: define it using val and use
 * tree_fold_pre_order
 *)
val tree_to_list = 
  let
    fun insert (n, lst) =
      lst @ [n]
  in
    tree_fold_pre_order insert []
  end

(*
 * Write a function to “filter” the tree. The function should return a tree with only
 * nodes for which f returns true. Use tree_delete. It will result in a very simple implementation
 * (though inefficient).
 *)
fun tree_filter f t =
  case t of
    emptyTree => t
  | nodeTree(n, left, right) => 
      if (f n) then
        nodeTree(n, tree_filter f left, tree_filter f right)
      else
        tree_filter f (tree_delete(t, n))

(*
 * Using a val expression, tree_filter and tree_fold_pre_order,
 * and function composition to write a function that sums the nodes that are are even. Use the mod
 * operator. Restriction: use function composition and a val expression to define this function.
 *)
val tree_sum_even =
  let
    fun is_even n =
      (n mod 2) = 0
  in
    (tree_fold_pre_order (op +) 0) o (tree_filter is_even)
  end

(*
 * Write a function first_answer f lst that has type (’a -> ’b option) -> ’a list ->
 * ’b (notice that the 2 arguments are curried. f should be applied to elements lst in order, until
 * the first time f returns SOME v for some v; in that case v is the result of the function. If f returns
 * NONE for all list elements, then first_answer should raise the exception NoAnswer.
 *)
fun first_answer f lst =
  case lst of
    [] => raise NoAnswer
  | head::tail =>
      let
        val ans = f head
      in
        if isSome ans then
          valOf ans
        else
          first_answer f tail
      end

(*
 * Write a function all_answer f lst of type (’a -> ’b list option) -> ’a list -> ’b
 * list option (notice the 2 arguments are curried). Like first_answer. f should be applied
 * to elements of the second argument. If it returns NONE for any element, then all_answer returns
 * NONE. Otherwise the calls to f will have produced SOME lst1, SOME lst2, ... SOME lstn
 * and the result of all_answer is SOME lst where lst is lst1, lst2, ..., lstn (the order in
 * the result list should be preserved). Note that all_answer f [] should return SOME [] for any f.
 *)
 fun all_answers f lst =
  let
    fun all_answer_t(lst, acc) =
      case lst of
        [] => SOME acc
      | head::tail =>
        let
          val ans = f head
        in
          if isSome ans then
            all_answer_t(tail, acc @ (valOf ans))
          else
            NONE
        end
  in
    all_answer_t(lst, [])
  end

(*
 * Write a function check_pattern that takes a pattern and returns true if and only if all the variables
 * appearing in the pattern are distinct from each other (i.e., use different strings). The constructor
 * names are not relevant. Hints: use two helper functions. The first takes a pattern and returns a list of
 * all the strings it uses for variables (use foldl with a function that uses append). The second helper
 * function a list of strings and decides if it has repeats (List.exists may be useful).
 *)
fun check_pattern p =
  let
    fun get_variable_list p =
      case p of
        Wildcard => []
		  | Variable v => [v]
		  | UnitP => []
		  | ConstP _ => []
		  | TupleP plist  => List.concat(List.map get_variable_list plist)
		  | ConstructorP (_, pat) => get_variable_list(pat)

    fun dup_in_list [] = false
     |  dup_in_list (head::tail) = 
          if List.exists (fn el => (String.compare(el, head) = EQUAL)) tail then
            true
          else
            dup_in_list tail 
  in
    not (dup_in_list (get_variable_list p))
  end

(*
 * Write a function match that takes a value * pattern and returns a (string * value) list
 * option, namely NONE if the pattern does not match and SOME lst where lst is the list of bindings
 * if it does. Note that if the value matches but the pattern has no patterns of the form Variable s,
 * then the result is SOME []. Remember to look above for the rules for what patterns match what
 * values, and what bindings they produce. Hints: use a case expression with 7 branches (one per
 * rule). The branch for tuples uses all_answer and ListPair.zip.
 *)
fun match(v, p) =
  case p of
    Wildcard => SOME []
  
  | Variable vname => SOME [(vname, v)]
  
  | UnitP =>
    (
      case v of
        Unit => SOME []
      | _ => NONE
    )
  
  | ConstP i =>
    (
      case v of
        Const j => if i = j then SOME [] else NONE
      | _ => NONE
    )
  
  | TupleP plist =>
    (
      case v of
        Tuple vlist => 
          if List.length(plist) = List.length(vlist) then
            let
              val ans = all_answers match (ListPair.zip(vlist, plist))
            in
              if isSome ans then
                SOME (valOf ans)
              else 
                NONE
            end
          else NONE
      | _ => NONE
    )
  
  | ConstructorP (p_cname, p_pat) =>
    (
      case v of
        Constructor (v_cname, v_val) =>
          if (String.compare(p_cname, v_cname) = EQUAL) then
            match(v_val, p_pat)
          else NONE
      | _ => NONE
    )

(*
 * Write a function first_match that takes a value and a list of patterns and returns a (string *
 * value) list option, namely NONE if no pattern in the list matches or SOME lst where lst is
 * the list of bindings for the first pattern in the list that matches. Use first_answer and a handleexpression.
 * Notice that the 2 arguments are curried.
 *)
fun first_match v plist =
  let
    val args = List.map (fn x => (v, x)) plist
  in
    SOME (first_answer match args)
      handle NoAnswer => NONE
  end

(* leave the following functions untouched *)

fun tree_root t =
    case t of
        emptyTree => NONE
      | nodeTree (n, _, _) => SOME n

fun tree_equal t1 t2  =
    case (t1, t2) of
        (emptyTree, emptyTree) => true
      | (emptyTree, _) =>  false
      | (_, emptyTree) => false
      | (nodeTree(n1, l1, r1),nodeTree(n2, l2, r2)) =>
        n1 = n2 andalso (tree_equal l1 l2) andalso (tree_equal r1 r2)

infix 9 ++
infix 9 --
infix 7 == 

fun t ++ v = tree_insert_in_order(t, v)
fun t -- v = tree_delete(t, v)
fun t1 == t2 = tree_equal t1 t2

end

