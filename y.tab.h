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
#define XL 265
#define YL 266
#define LS 267
#define PR 268
#define CI 269
#define DI 270
#define QUIT 271
#define UNARYMINUS 272
#define UNARYPLUS 273
typedef union {
    DATUM  *y_datum;	/* pointer to a complex number type */
    double  y_num;	/* a double precision number */
    Symbol *y_sym;	/* pointer to Symbol Table */
} YYSTYPE;
extern YYSTYPE yylval;
