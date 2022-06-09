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
#define LX 268
#define VS 269
#define PR 270
#define CI 271
#define DI 272
#define QUIT 273
#define UNARYMINUS 274
#define UNARYPLUS 275
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union YYSTYPE {
    DATUM  *y_datum;	/* pointer to a complex number type */
    double  y_num;	/* a double precision number */
    Symbol *y_sym;	/* pointer to Symbol Table */
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;
