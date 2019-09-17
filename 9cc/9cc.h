#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c

//トークンの種類
typedef enum {
	TK_RESERVED,	// 記号
	TK_NUM,			// 整数トークン
	TK_EOF,			// 入力の終わりを表すトークン
} TokenKind;

//	トークン型
typedef struct Token Token;
struct Token {
	TokenKind kind;	// トークンの型
	Token *next;	// 次の入力トークン
	int val;		// kindがTK_NUMの場合、その数値
	char *str;		// トークン文字列
	int len;		// トークンの長さ
};


// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD,	// +
	ND_SUB,	// -
	ND_MUL,	// *
	ND_DIV,	// /
	ND_NUM,	// 数字
	ND_EQ,	// ==
	ND_NEQ,	// !=
	ND_LT,	// <
	ND_GT,	// >
	ND_LE,	// <=
	ND_GE,	// >=
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;	//kind が ND_NUM の場合のみ使う
};

Node *expr();

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *tokenize(char *p);
void gen(Node *node);