grammar eel;

Identifier:  [_a-zA-Z][_a-zA-Z0-9]*;
TrailingComma: ',' |;
DecDigit: [0-9];
HexDigit: [0-9a-fA-F];
CharSymbol:
    '\\n' | '\\r' | '\\t' | '\\v'
    | '\\x' HexDigit+ | '\\\\'
    | '\\\'' | '\\"' | [ -~]
;

IntegerLiteral: DecDigit+ | '0x' HexDigit+;
FloatLiteral: '.' DecDigit+ | DecDigit+ '.' DecDigit+;
BoolLiteral: 'true' | 'false';
CharLiteral: '\'' CharSymbol '\'';
StringLiteral: '"' CharSymbol* '"';
Comment: '//' ~[\r\n]* -> skip;
BlockComment: '/*' .*? '*/' -> skip;

fqn: fqn '::' Identifier | Identifier;

type: fqn | arrayType | pointerType | referenceType;
arrayType: arrayType '[' expr ']' | fqn '[' expr ']';
pointerType: fqn '*';
referenceType: fqn '&';

program: tlDecl;

tlDecl:
    decls
    | loopDecl
    | setupDecl
    | includeDirective
    | traitImplBlock
    | implBlock
;

lDecl:
    variableDecl
    | constDecl
    | staticDecl
    | pinDecl
;

typeDecl:
    structDecl
    | unionDecl
    | untaggedUnionDecl
    | enumDecl
    | eventDecl
    | intervalDecl
    | onDecl
    | traitDecl
;

decls: decl*;
decl:
    lDecl
    | fnDecl
    | typeDecl
    | namespaceDecl
;


includeDirective:
    'include' Identifier ';'
    | 'include' Identifier 'as' Identifier ';'
;

loopDecl:
    'loop' stmtBlock
;

setupDecl:
    'setup' stmtBlock
;

pinDecl:
    'pin' Identifier type '(' expr ')' ';'
;

variableDecl:
    type Identifier ';'
    | type Identifier '=' expr ';'
;

constDecl:
    'const' type Identifier '=' expr ';'
;

staticDecl:
    'static' variableDecl
;


associatedMember:
    instanceAssociatedFn
    | partialInstanceAssociatedFn
    | typeAssociatedFn
    | partialTypeAssociatedFn
;

instanceAssocParamList:
    '&' 'self' ',' paramList
    | '&' 'self'
;

instanceAssociatedFn:
    'fn' Identifier '(' instanceAssocParamList TrailingComma ')' stmtBlock
    | 'fn' Identifier '(' instanceAssocParamList TrailingComma ')' '->' type stmtBlock
;

partialInstanceAssociatedFn:
    'fn' Identifier '(' instanceAssocParamList TrailingComma ')' ';'
    | 'fn' Identifier '(' instanceAssocParamList TrailingComma ')' '->' type ';'
;

typeAssociatedFn: fnDecl;

partialTypeAssociatedFn:
    'fn' Identifier '(' paramList ')' ';'
    | 'fn' Identifier '(' paramList ')' '->' type ';'
;

traitImplBlock:
    'impl' type 'for' type '{' associatedMember* '}'
;

implBlock:
    'impl' type '{' associatedMember* '}'
;

fnDecl:
    'fn' Identifier '(' paramList ')' stmtBlock
    | 'fn' Identifier '(' paramList ')' '->' type stmtBlock
;

paramList:
    fnParam ',' paramList
    | fnParam TrailingComma;

fnParam: type Identifier;

eventDecl:
    'event' Identifier ';'
    | 'event' Identifier stmtBlock
;

intervalDecl:
    'interval' Identifier '=' expr ';'
;

traitDecl:
    'trait' Identifier '{' associatedMember* '}'
;

structDecl:
    'struct' Identifier '{' fieldList '}'
;

unionDecl:
    'union' Identifier '{' fieldList '}'
;

untaggedUnionDecl:
    'untagged' unionDecl
;

enumDecl:
    'enum' Identifier ':' type '{' Identifier '}' ';'
    | 'enum' Identifier ':' type '{' expr '}' ';'
;

fieldList:
    field fieldList |
    field
;

field:
    type Identifier ';'
;


namespaceDecl:
    'namespace' Identifier '{' decls '}'
;

onDecl:
    'on' Identifier stmtBlock
;

stmt:
    ifStmt
    | switchStmt
    | whileStmt
    | forStmt
    | stmtBlock
    | doWhileStmt
    | forEachStmt
    | lockStmt
    | awaitStmt
    | pinStmt
    | expr ';'
;
stmts: stmt*;
stmtsOrLDecls:
    stmt stmtsOrLDecls
    | lDecl stmtsOrLDecls
    |
;
stmtBlock: '{' stmtsOrLDecls '}';

breakStmt: 'break';
continueStmt: 'continue';
returnStmt: 'return' expr;

elseStmt: 'else' stmt | 'else' '{' stmts '}' ;
ifStmt: 'if' '(' expr ')' stmt elseStmt? | 'if' '(' expr ')' '{' stmts '}' elseStmt? ;

switchStmt:
    'switch' '(' expr ')' '{' caseStmt+ '}'
;


caseStmt:
    'case' expr stmts
    | defaultStmts
;

defaultStmts:
    'default' stmts
;

whileStmt:
    'while' ( expr ) stmtBlock
;

doWhileStmt:
    'do' stmtBlock 'while' ( expr )
;

forStmt:
    'for' '(' stmt ';' expr ';' stmt ')' stmtBlock
;

forEachStmt:
    'foreach' '(' Identifier 'in' 'Identifier' ')'
    | 'foreach' '(' Identifier ',' Identifier 'in' 'Identifier' ')'
;


lockStmt:
    'lock' lockList stmtBlock
;

lockList:
    Identifier ',' lockList
    | Identifier TrailingComma
;

awaitStmt:
    'await' expr ;

pinStmt:
    'set' Identifier expr
    | 'set' Identifier 'mode' expr
    | 'set' Identifier 'pin' expr
;

fieldInit:
    Identifier '=' expr ';'
;

exprList:
    expr ',' exprList
    | expr TrailingComma
;


expr:
    '(' expr ')' # ParenExpr
    | IntegerLiteral # IntegerLiteral
    | FloatLiteral # FloatLiteral
    | BoolLiteral # BoolLiteral
    | CharLiteral # CharLiteral
    | StringLiteral # StringLiteral

    | type '{' fieldInit* '}' # StructExpr
    | '{' exprList '}' # ArrayExpr

    | fqn # FqnExpr
    | readPin # ReadPinExpr
    | expr 'as' type # CastExpr

    | <assoc=right> '+' expr # Pos
    | <assoc=right> '-' expr # Neg
    | <assoc=right> '!' expr # Not
    | <assoc=right> '~' expr # BitComp
    | <assoc=right> '*' expr # Deref

    | <assoc=left> expr ('/'|'*'|'%') expr # Scaling
    | <assoc=left> expr ('+'|'-') expr # Addition

    | <assoc=left> expr '>>' expr # ArithmeticRightShift
    | <assoc=left> expr '>>>' expr # LogicalRightShift
    | <assoc=left> expr '<<' expr # LeftShift

    | <assoc=left> expr '>' expr # GreaterThan
    | <assoc=left> expr '<' expr # LessThan
    | <assoc=left> expr '>=' expr # GreaterThanEquals
    | <assoc=left> expr '<=' expr # LessThanEquals
    | <assoc=left> expr '==' expr # Equals
    | <assoc=left> expr '!=' expr # NotEquals

    | <assoc=left> expr '&' expr # And
    | <assoc=left> expr '^' expr # Xor
    | <assoc=left> expr '|' expr # Or

    | <assoc=left> expr '&&' expr # LAnd
    | <assoc=left> expr '||' expr # LOr

    | <assoc=right> expr '=' expr # Assign
    | <assoc=right> expr '-=' expr # SubAssign
    | <assoc=right> expr '+=' expr # AddAssign
    | <assoc=right> expr '/=' expr # DivAssign
    | <assoc=right> expr '*=' expr # MulAssign
    | <assoc=right> expr '%=' expr # ModAssign
    | <assoc=right> expr '>>=' expr # ArsAssign
    | <assoc=right> expr '>>>=' expr # LrsAssign
    | <assoc=right> expr '<<=' expr # LsAssign
    | <assoc=right> expr '|=' expr # OrAssign
    | <assoc=right> expr '&=' expr # AndAssign
    | <assoc=right> expr '^=' expr # XorAssign
;

readPin:
    'read' Identifier
;