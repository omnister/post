#ifndef YYERRCODE
#define YYERRCODE 256
#endif

#define I 257
#define NUMBER 258
#define VAR 259
#define BLTIN 260
#define UNDEF 261
#define STRING 262
#define GR 263
#define GS 264
#define GN 265
#define LS 266
#define CI 267
#define DI 268
#define UNARYMINUS 269
#define UNARYPLUS 270
typedef union {
    DATUM  *y_datum;	/* pointer to a complex number type */
    double  y_num;	/* a double precision number */
    Symbol *y_sym;	/* pointer to Symbol Table */
} YYSTYPE;
extern YYSTYPE yylval;
