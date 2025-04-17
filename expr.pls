# this is NOT pls code (yet)

ops=["+","-","*","/","^","==","!=","<",">","<=",">="]

uops=["-"]

precedences=object
precedences = objset(precedences, "+", 2)
precedences = objset(precedences, "-", 2)
precedences = objset(precedences, "*", 3)
precedences = objset(precedences, "/", 3)
precedences = objset(precedences, "^", 4)
precedences = objset(precedences, "==", 1)
precedences = objset(precedences, "!=", 1)
precedences = objset(precedences, "<", 1)
precedences = objset(precedences, ">", 1)
precedences = objset(precedences, "<=", 1)
precedences = objset(precedences, ">=", 1)

# token types
INT="INT"   # literal
STR="STR"   # literal
LPAR="LPAR" # left paren
RPAR="RPAR" # right paren
OP="OP"     # binary operator
UOP="UOP"   # unary operator
SYM="SYM"   # symbol (variable or function)
EXPR="EXPR" # expression (output of evaluate)
CALL="CALL" # left paren after function name
IDX="IDX"
COMMA="COMMA"
LBR="LBR"
RBR="RBR"
DOT="DOT"

numregex = /[0-9]+/y

stringregex = /"(([^"\\]|\\([n\\"]|x[0-9a-fA-F]{2}))*)"/y;

fn getDigit(s) {
  if ord(s) >= ord("0") then if ord(s) <= ord("9") then {
	return [ord(s) - ord("0"), strmid(s, 1)]
  }
}

fn repeat(p) {
  fn addnull(s, p) {
    for x in gcall(p, s) do {
	  def v = x[0]
	  def s = x[1]
	  yield [[true, v], s]
	}
	return [[false, null], s]
  }
  fn repeat(s, addnull) {
    def l = []
    while true do {
      def x = addnull(s)
	  def v = x[0]
	  s = x[1]
      l = concat(l, [v[1]])
	  yield ([l, s])
	  if v[0] then {} else return
      s = x[1]
    }
  }
  return bind(repeat, bind(addnull, p))
}

fn atomic(p) {
  fn atomic(s, p) {
    for x in gcall(p, s) {
	  return x
	}
  }
  return bind(atomic, p)
}

def getInt = atomic(repeat(getDigit))

getStr = function(s) {
  # not a variable
  stringregex.lastIndex = 0;
  var m = stringregex.exec(s);
  if (m != null) {
    let instr = [...m[1]]
    let outstr = ""
    while (instr.length > 0) {
      const c = instr.shift()
      if (c != "\\") {
        outstr += c;
      } else {
        const c = instr.shift();
        if (c == "x") {
          outstr += String.fromCharCode(parseInt(instr[0] + instr[1], 16));
          instr = instr.slice(0, 2)
        } else {
          outstr += {"\\": "\\", "\"": "\"", "n": "\n"}[c];
        }
      }
    }
    return [outstr, s.slice(m[0].length)];
  }
  return [null, s];
}

getSym = function(s) {
  # not a variable
  var m;
  if ((m = /[_a-zA-Z][_a-zA-Z0-9]*/y.exec(s)) != null) {
    return [m[0],s.slice(m[0].length)];
  }
  return [null, s];
}

getToken = function(s, lastType, comma) {
  const ss = s.trim();
  if (lastType == DOT) {
    const [sym,snew] = getSym(ss);
    if (sym != null) {
      return [[SYM, sym], snew]
    }
    return [null, ss];
  } else if ([INT, STR, SYM, RPAR, RBR].includes(lastType)) {
    for (const op of ops) {
      if (ss.startsWith(op)) {
        return [[OP, op], ss.slice(op.length)];
      }
    }
    if (comma && ss.startsWith(",")) {
      return [[COMMA, ","], ss.slice(1)];
    }
    if (ss.startsWith(")")) {
      return [[RPAR], ss.slice(1)];
    }
    if (ss.startsWith("]")) {
      return [[RBR], ss.slice(1)];
    }
    if (ss.startsWith("(")) {
      return [[CALL], ss.slice(1)];
    }
    if (ss.startsWith("[")) {
      return [[IDX], ss.slice(1)];
    }
    if (ss.startsWith(".")) {
      return [[DOT], ss.slice(1)];
    }
    return [null, ss];
  } else if ([LPAR, CALL, IDX, OP, UOP, LBR, COMMA].includes(lastType)) {
    if (ss.startsWith("(")) {
      return [[LPAR],ss.slice(1)];
    }
    if (ss.startsWith("[")) {
      return [[LBR],ss.slice(1)];
    }
    if ([CALL,COMMA].includes(lastType) && ss.startsWith(")")) {
      return [[RPAR],ss.slice(1)];
    }
    if ([LBR,COMMA].includes(lastType) && ss.startsWith("]")) {
      return [[RBR],ss.slice(1)];
    }
    for (const uop of uops) {
      if (ss.startsWith(uop)) {
        return [[UOP, uop], ss.slice(uop.length)];
      }
    }
    {
      let [intlit, snew] = getInt(ss);
      if (intlit != null) {
        return [[INT, intlit], snew];
      }
    }
    {
      let [strlit, snew] = getStr(ss);
      if (strlit != null) {
        return [[STR, strlit], snew];
      }
    }
    {
      let [sym, snew] = getSym(ss);
      if (sym != null) {
        return [[SYM, sym], snew];
      }
    }
    return [null, ss];
  }
  return [null, ss];
}

precedence = function(token) {
  # get the precedence of a binary operator
  return precedences[token[1]];
}

apply = function(op, v1, v2) {
  return [EXPR, op[1], v1, v2];
}

applyuop = function(op, v) {
  return [EXPR, op[1], v];
}

addToken = function(token, lastType, values, ops, parens) {
  if ([INT,STR,SYM].includes(token[0])) { # literal or symbol token
    values.push(token);
  }
  if ([INT,STR,SYM,EXPR].includes(token[0])) { # literal, symbol, or expression
    while (ops.length > 0 && ops.at(-1)[0] == UOP) { # apply all unary operators on the stack
      op = ops.pop();
      values[values.length - 1] = applyuop(op, values.at(-1));
    }
  }
  if ([LPAR,CALL,IDX,LBR].includes(token[0])) { # left paren
    ops.push(token);
    parens.push(token);
    if (token[0] == CALL) {
      # unapply all unary operators
      par = ops.pop();
      while (values.at(-1).length == 3) {
        const [, op, val] = values.at(-1);
        ops.push([UOP, op]);
        values[values.length - 1] = val;
      }
      ops.push(par);
      values[values.length - 1] = [EXPR, "(", values.at(-1)];
    }
    if (token[0] == IDX) {
      values[values.length - 1] = [EXPR, "[", values.at(-1)];
    }
    if (token[0] == LBR) {
      values.push([EXPR, "[]"]);
    }
  }
  if (token[0] == UOP) { # unary operator
    ops.push(token);
  }
  if ([RPAR,RBR].includes(token[0])) { # right paren
    while (!([LPAR,CALL,IDX,LBR].includes(ops.at(-1)[0]))) { # finish the parenthesized expression
      op = ops.pop();
      if (op[0] == DOT) {
        op = [OP, "."];
      }
      v2 = values.pop();
      v1 = values.pop();
      values.push(apply(op, v1, v2));
    }
    if ([CALL,IDX,LBR].includes(ops.at(-1)[0])) {
      if (!([ops.at(-1)[0],COMMA].includes(lastType))) {
        arg = values.pop();
        values.at(-1).push(arg);
      }
    }
    ops.pop();
    parens.pop();
  }
  if (token[0] == OP) {
    while (ops.length > 0 && (ops.at(-1)[0] == DOT || (!([LPAR,CALL,IDX].includes(ops.at(-1)[0])) && precedence(ops.at(-1)) >= precedence(token)))) {
      # apply all operators to the left with a lower precedence
      op = ops.pop();
      if (op[0] == DOT) {
        op = [OP, "."];
      }
      v2 = values.pop();
      v1 = values.pop();
      values.push(apply(op, v1, v2));
    }
    ops.push(token) # push this operator
  }
  if (token[0] == DOT) {
    ops.push(token) # push this operator
  }
  if (token[0] == COMMA) {
    while (!([CALL,IDX,LBR].includes(ops.at(-1)[0]))) {
      # apply all operators to the left with a lower precedence
      op = ops.pop();
      if (op[0] == DOT) {
        op = [OP, "."];
      }
      v2 = values.pop();
      v1 = values.pop();
      values.push(apply(op, v1, v2));
    }
    arg = values.pop();
    values.at(-1).push(arg);
  }
  return [values, ops, parens];
}

evaluate = function(expr) {
  values = [];
  let ops = [];
  parens = [];

  s = expr

  # basic shunting yard parser
  lastType = OP # a valid expression can always come after an operator
  while (true) { # parse all the tokens
    [token,s] = getToken(s,lastType,parens.length > 0 && [CALL,LBR].includes(parens.at(-1)[0]));
    if (token == null) {
      break;
    }
    [values,ops,parens] = addToken(token,lastType,values,ops,parens)
    lastType = token[0] # type of last token
  }
  while (ops.length > 0) { # apply the rest of the operators
    if (ops.at(-1)[0] == UOP) {
      op = ops.pop();
      values[values.length - 1] = applyuop(op, values.at(-1));
    } else if ([OP,DOT].includes(ops.at(-1)[0])) {
      op = ops.pop();
      if (op[0] == DOT) {
        op = [OP, "."];
      }
      v2 = values.pop();
      v1 = values.pop();
      values.push(apply(op, v1, v2));
    } else {
      return [null, expr];
    }
  }
  if (values.length != 1) {
    return [null, expr];
  }
  return [values[0], s];
}