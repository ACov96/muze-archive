// Playing around with syntax here, feel free to add ideas

mod Scratch
  // Syntax for generics, maybe?
  type linkedlist(T) =
    rec {
      value : T ;
      next : linkedlist(T)
    }

    mu array(T)
      size : integer;
      temp : linkedlist(T);

      size = 0;

      // This feels weird...
      temp = linkedlist;

      // pre-count elements
      loop
        if temp == Nil then
          break
        fi

        n += 1;
        temp = temp.next;
      pool

      // I guess?
      array = new_array(T)(n);
      temp = linkedlist;

      // exclusive '..'
      fa i in 0..n do
        array[i] = temp.value;
        temp = temp.next;
      af
    um
  epyt

  fun fibonacci(n : integer): array(integer)
    fun fib_inner(a, b, n : integer): linkedlist(integer)
      ret : linkedlist(integer);
      ret_copy : linkedlist(integer);

      if n == 0 then
        // Maybe this line is syntactic sugar
        ret := rec { a, Nil } ;
        // for this line
        ret = morph rec { a, Nil } ;

        // Deep copy sugar
        ret_copy :: ret;
        ret_copy = copy ret;

        // When should does this deep copy (see below)?
        ret_copy := ret;
      else
        ret := rec { a, fib_inner(b, a + b, n - 1) }
      fi

      return ret;
    nuf

    // Uses morph from linkedlist to array in the return
    return fib_inner(0, 1, n);
  nuf

  type fahrenheit = integer
  epyt

  type celsius = integer
    // maybe put a 'bijective' keyword on functions which are a single
    // expression and only call other bijective functions
    mu bijective fahrenheit
      5/9 * celsius + 32 ;
    um
  epyt

  type fahrenheit = not integer
  epyt

  // Do we want a 'type alias', like the following, to NOT deep copy when morphed?
  
  // The following is an example where this might be the least astonishing behavior

  type celsius = integer
  epyt

  fun inc(x : integer)
    x = x + 1;
  nuf

  a : celsius ;
  a := 1;

  // Here, `a` is morphed into an integer for the function call, but would be 
  // expected to still be incremented as a result of the call.
  // If morphing always deep copies, then this is not the case.
  inc(a);
dom

