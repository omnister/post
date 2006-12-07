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
#define XL 264
#define YL 265
#define LS 266
#define PR 267
#define CI 268
#define DI 269
#define QUIT 270
#define UNARYMINUS 271
#define UNARYPLUS 272
typedef union {
    DATUM  *y_datum;	/* pointer to a complex number type */
    double  y_num;	/* a double precision number */
    Symbol *y_sym;	/* pointer to Symbol Table */
} YYSTYPE;
extern YYSTYPE yylval;
