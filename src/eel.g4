grammar eel;

IntegerLiteral: DecDigit+ | '0x' HexDigit+;
FloatLiteral: '.' DecDigit+ | DecDigit+ '.' DecDigit+;
BoolLiteral: 'true' | 'false';
CharLiteral: '\'' CharSymbol? '\'';
StringLiteral: '"' CharSymbol* '"';

DecDigit: [0-9];
HexDigit: [0-9a-fA-F];

IntegerTypes: 'i8'
    | 'i16'
    | 'i32'
    | 'i64'
    | 'u8'
    | 'u16'
    | 'u32'
    | 'u64'
;


FloatingTypes: 'f32' | 'f64' ;
CharType: 'char';
StringType: 'string';
PinType: 'analog' | 'digital' ;

Include: 'include';
Pin: 'pin';
Loop: 'loop';
Setup: 'setup';

Struct: 'struct';
Union: 'union';
Enum: 'enum';
Untagged: 'untagged';

Static: 'static';
Const: 'const';
As: 'as';


Fn: 'fn';
Return: 'return';
Break: 'break';
Continue: 'continue';
Self: 'self';
Impl: 'impl';

Event: 'event';
Interval: 'interval';
Trait: 'trait';

On: 'on';
In: 'in';
Await: 'await';
Lock: 'lock';
Set: 'set';
Mode: 'mode';
Read: 'read';


Namespace: 'namespace';

If: 'if';
Else: 'else';

Switch: 'switch';
Case: 'case';
Default: 'default';

While: 'while';
Do: 'do';
For: 'for';
Foreach: 'foreach';

Identifier:  [_a-zA-Z][_a-zA-Z0-9]*;
CharSymbol:
    '\\n' | '\\r' | '\\t' | '\\v'
    | '\\x' HexDigit+ | '\\\\'
    | '\\\'' | '\\"' | [\u0028-\u002E\u0032-\u0085\u0087-\u00B0]
;

Comment: '//' ~[\r\n]* -> skip;
BlockComment: '/*' .*? '*/' -> skip;
Ignore: [ \r\n] -> skip;


trailingComma: ',' |;

fqn: fqn '::' Identifier | Identifier;

type:
      arrayType
    | pointerType
    | referenceType
    | IntegerTypes
    | FloatingTypes
    | CharType
    | StringType
    | PinType
    | fqn
;

arrayType: arrayType '[' expr ']' | fqn '[' expr ']';
pointerType: fqn '*';
referenceType: fqn '&';

program: tlDecl* ;

tlDecl:
    decl
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
    Include Identifier ';'
    | Include Identifier As Identifier ';'
;

loopDecl:
    Loop stmtBlock
;

setupDecl:
    Setup stmtBlock
;

pinDecl:
    Pin Identifier type conditionBlock ';'
;

variableDecl:
    type Identifier ';'
    | type Identifier '=' expr ';'
;

constDecl:
    Const type Identifier '=' expr ';'
;

staticDecl:
    Static variableDecl
;


associatedMember:
    instanceAssociatedFn
    | partialInstanceAssociatedFn
    | typeAssociatedFn
    | partialTypeAssociatedFn
;

instanceAssocParamList:
    '&' Self ',' paramList
    | '&' Self
;

instanceAssociatedFn:
    Fn Identifier '(' instanceAssocParamList trailingComma ')' stmtBlock
    | Fn Identifier '(' instanceAssocParamList trailingComma ')' '->' type stmtBlock
;

partialInstanceAssociatedFn:
    Fn Identifier '(' instanceAssocParamList trailingComma ')' ';'
    | Fn Identifier '(' instanceAssocParamList trailingComma ')' '->' type ';'
;

typeAssociatedFn: fnDecl;

partialTypeAssociatedFn:
    Fn Identifier '(' paramList ')' ';'
    | Fn Identifier '(' paramList ')' '->' type ';'
;

traitImplBlock:
    Impl type For type '{' associatedMember* '}'
;

implBlock:
    Impl type '{' associatedMember* '}'
;

fnDecl:
    Fn Identifier '(' paramList ')' stmtBlock
    | Fn Identifier '(' paramList ')' '->' type stmtBlock
;

paramList:
    fnParam ',' paramList
    | fnParam trailingComma
    | // Used for empty parameter list
    ;

fnParam: type Identifier;

eventDecl:
    Event Identifier ';'
    | Event Identifier stmtBlock
;

intervalDecl:
    Interval Identifier '=' expr ';'
;

traitDecl:
    Trait Identifier '{' associatedMember* '}'
;

structDecl:
    Struct Identifier '{' fieldList '}'
;

unionDecl:
    Union Identifier '{' fieldList '}'
;

untaggedUnionDecl:
    Untagged unionDecl
;

enumDecl:
    Enum Identifier ':' type '{' Identifier '}' ';'
    | Enum Identifier ':' type '{' expr '}' ';'
;

fieldList:
    field fieldList |
    field
;

field:
    type Identifier ';'
;


namespaceDecl:
    Namespace Identifier '{' decls '}'
;

onDecl:
    On Identifier stmtBlock
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
    | continueStmt
    | breakStmt
    | returnStmt
    | expr ';'
;
stmts: stmt*;
stmtsOrLDecls:
    stmt stmtsOrLDecls
    | lDecl stmtsOrLDecls
    |
;
stmtBlock: '{' stmtsOrLDecls '}';
conditionBlock: '(' expr ')';

breakStmt: Break ';';
continueStmt: Continue ';';
returnStmt: Return expr ';';

elseStmt: Else stmt | Else stmtBlock ;
ifStmt: If conditionBlock stmt elseStmt? | If conditionBlock stmtBlock elseStmt? ;

switchStmt:
    Switch conditionBlock '{' (caseStmt | defaultStmt)* '}'
;

caseStmt:
    Case expr ':' stmts
;

defaultStmt:
    Default ':' stmts
;

whileStmt:
    While conditionBlock stmtBlock
;

doWhileStmt:
    Do stmtBlock While ( expr )
;

forStmt:
    For '(' stmt ';' expr ';' stmt ')' stmtBlock
;

forEachStmt:
    Foreach '(' Identifier In Identifier ')'
    | Foreach '(' Identifier ',' Identifier 'in' Identifier ')'
;


lockStmt:
    Lock lockList stmtBlock
;

lockList:
    Identifier ',' lockList
    | Identifier trailingComma
;

awaitStmt:
    Await expr ;

pinStmt:
    Set Identifier expr
    | Set Identifier Mode expr
    | Set Identifier Pin expr
;

fieldInit:
    Identifier '=' expr ';'
;

exprList:
    expr ',' exprList
    | expr trailingComma
;


expr:
    '(' expr ')' # ParenExpr
    | IntegerLiteral # IntegerLiteral
    | FloatLiteral # FloatLiteral
    | BoolLiteral # BoolLiteral
    | CharLiteral # CharLiterals
    | StringLiteral # StringLiteral

    | type '{' fieldInit* '}' # StructExpr
    | '{' exprList '}' # ArrayExpr

    | fqn # FqnExpr
    | readPin # ReadPinExpr
    | expr As type # CastExpr

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
    Read Identifier
;