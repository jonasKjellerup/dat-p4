grammar eel;

IntegerLiteral: DecDigit+ | '0x' HexDigit+;
FloatLiteral: '.' DecDigit+ | DecDigit+ '.' DecDigit+;
BoolLiteral: 'true' | 'false';
CharLiteral: '\'' ('\\n' | '\\r' | '\\t' | '\\v'
                       | '\\x' HexDigit+ | '\\\\'
                       | '\\\'' | [\u0020-\u0026\u0028-\u007E])? '\'';
StringLiteral: '"' ('\\n' | '\\r' | '\\t' | '\\v'
                        | '\\x' HexDigit+ | '\\\\'
                        | '\\"' | [\u0020-\u0021\u0023-\u007E])* '"';

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

DecDigit: [0-9];
HexDigit: [0-9a-fA-F];


Comment: '//' ~[\r\n]* -> skip;
BlockComment: '/*' .*? '*/' -> skip;
Ignore: [ \r\n] -> skip;


trailingComma: ',' |;

fqn:
    fqn '::' Identifier     # NamespaceIdentifier
    | fqn '.'  Identifier   # ObjectIdentifier
    | Identifier            # Identifier
;

type:
     IntegerTypes
    | FloatingTypes
    | CharType
    | StringType
    | PinType
    | fqn
;

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
    | arrayDecl
    | referenceDecl
    | pointerDecl
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
    Pin Identifier PinType '(' expr ')' ';'
;

typedIdentifier: type Identifier;

variableDecl:
    typedIdentifier ';'
    | typedIdentifier '=' expr ';'
;

constDecl:
    Const typedIdentifier '=' expr ';'
;

staticDecl:
    Static variableDecl
;

arrayDecl:
    typedIdentifier '[' ']' '=' arrayInit ';'
    | typedIdentifier '[' expr ']' ('=' arrayInit)? ';'
;

referenceDecl: type '&' Identifier ('=' expr)? ';';
pointerDecl: type '*' Identifier ('=' expr)? ';';

arrayInit: '{' exprList '}';

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

fnParam: typedIdentifier;



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
    Enum Identifier (':' type)? '{' (Identifier ('=' expr)? ';')* '}'
;

fieldList:
    field fieldList |
    field
;

field:
    typedIdentifier ';'
    | typedIdentifier'[' expr ']' ';'
    | type ('&' | '*')? Identifier ';'
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
returnStmt: Return expr? ';';

elseStmt: Else stmt | Else stmtBlock ;
ifStmt: If conditionBlock stmt elseStmt? | If conditionBlock stmtBlock elseStmt? ;

switchStmt:
    Switch conditionBlock '{' ((caseStmt)* | (caseStmt* defaultStmt?)) '}'
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
    Do stmtBlock While conditionBlock
;

forStmt:
    For '(' stmtsOrLDecls  expr ';'  expr ')' stmtBlock
;

forEachStmt:
    Foreach '(' x1=Identifier In x3=Identifier ')'
    | Foreach '(' x1=Identifier ',' x2=Identifier In x3=Identifier ')'
;


lockStmt:
    Lock lockList stmtBlock
;

lockList:
    fqn ',' lockList
    | fqn trailingComma
;

awaitStmt:
    Await expr ;

pinStmt:
    Set fqn expr
    | Set fqn Mode expr
    | Set fqn Pin expr
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
    | CharLiteral # CharLiteral
    | StringLiteral # StringLiteral

    | fqn '(' params=exprList ')' # FnCallExpr
    | fqn '(' '&' Self ',' params=exprList ')' # InstanceAssociatedFnCallExpr

    | type '{' fieldInit* '}' # StructExpr

    | fqn '[' expr ']' # ArrayExpr
    | fqn '*' # PointerExpr
    | fqn '&' # ReferenceExpr
    | fqn # FqnExpr

    | Read fqn # ReadPinExpr
    | expr As type # CastExpr

    | <assoc=right> '+' left=expr # Pos
    | <assoc=right> '-' left=expr # Neg
    | <assoc=right> '!' left=expr # Not
    | <assoc=right> '~' left=expr # BitComp
    | <assoc=right> '*' left=expr # Deref

    | <assoc=left> right=expr op=('/'|'*'|'%') left=expr # ScalingExpr
    | <assoc=left> right=expr op=('+'|'-') left=expr # AdditiveExpr
    | <assoc=left> right=expr op=('>>'|'>>>' |'<<') left=expr # ShiftingExpr
    | <assoc=left> right=expr op=('>'|'>='|'<='|'=='|'!=') left=expr # ComparisonExpr

    | <assoc=left> right=expr '&' left=expr # AndExpr
    | <assoc=left> right=expr '^' left=expr # XorExpr
    | <assoc=left> right=expr '|' left=expr # OrExpr

    | <assoc=left> right=expr '&&' left=expr # LAndExpr
    | <assoc=left> right=expr '||' left=expr # LOrExpr

    | <assoc=right> right=expr '=' left=expr # AssignExpr

    | <assoc=right> right=expr op=('-='|'+=') left=expr # AdditiveAssignExpr
    | <assoc=right> right=expr op=('/='|'*='|'%=') left=expr # ScalingAssignExpr
    | <assoc=right> right=expr op=('>>='|'>>>='|'<<=') left=expr # ShiftingAssignExpr

    | <assoc=right> right=expr '|=' left=expr # OrAssignExpr
    | <assoc=right> right=expr '&=' left=expr # AndAssignExpr
    | <assoc=right> right=expr '^=' left=expr # XorAssignExpr
;