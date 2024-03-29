#include "9cc.h"


// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); // pos 個の空白を出力
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
	    memcmp(token->str, op, token->len))
	  return false;
	  
	token = token->next;
	return true;
}
//	次のトークンが期待している識別子のときには、トークンを１つ読み進めて
//	Token を返す。それ以外の場合には NULL を返す。
Token *consume_ident() {
	if (token->kind != TK_IDENT)
		return NULL;
	Token *tok = token;
	token = token->next;
	return tok;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。

void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
	    memcmp(token->str, op, token->len))
	  error_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM)
	  error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

//	変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next)
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
			return var;
	return NULL;
}

//	与えられた英数字がトークンを構成する英数字かアンダースコアか
//	判定する関数
int is_alnum(char c) {
	return	('a' <= c && 'z') ||
			('A' <= c && 'Z') ||
			('0' <= c && '9') ||
			(c == '_');
}

bool at_eof() {
	return token->kind == TK_EOF;
}

//新しいトークンを作成して cur につなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	
	return tok;
}

// 入力文字列 p をトークナイズしてそれを返す
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	locals = calloc(1, sizeof(LVar));
	locals->offset = 0;

	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		//	return をトークナイズ
		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			cur = new_token(TK_RESERVED, cur, p, 6);
			p += 6;
			continue;
		}

		// アルファベットの小文字ならば TK_IDENT 型のトークンを作成
		if ('a' <= *p && *p <= 'z') {
			int len = 0;
			char *tmp = p;
			//	現在の位置から何文字か数える
			while ('a' <= *p && *p <= 'z') {
				len++;
				p++;
			}
			
			cur = new_token(TK_IDENT, cur, tmp, len);
			continue;
		}

		// 比較演算子は算術演算子よりも先に書く
		if(memcmp(p, "==", 2) == 0 ||
		   memcmp(p, "!=", 2) == 0 ||
		   memcmp(p, "<=", 2) == 0 ||
		   memcmp(p, ">=", 2) == 0
		  ) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}
		
		if (*p == '+' || *p == '-' || *p == '*' || 
		    *p == '/' || *p == '(' || *p == ')' ||
			*p == '<' || *p == '>' || *p == ';' ||
			*p == '=') {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "tokenize できません");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *code[100];

Node *program() {
	int i = 0;
	while (!at_eof())
	  code[i++] = stmt();
	code[i] = NULL;
}

Node *stmt() {
	Node *node;

	if (consume("return")) {
		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	} else {
		node = expr();
	}
	expect(";");
	return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
	Node *node = equality();
	if (consume("="))
	  node = new_node(ND_ASSIGN, node, assign()); 
	return node;
}

Node *equality() {
  Node *node = relational();
  
  	for (;;) {
		if (consume("=="))
		  node = new_node(ND_EQ, node, relational());
		if (consume("!="))
		  node = new_node(ND_NEQ, node, relational());
		else
		  return node;
	}
}

Node *relational() {
	Node *node = add();
  
  	for (;;) {
		if (consume("<"))
		  node = new_node(ND_LT, node, relational());
		if (consume("<="))
		  node = new_node(ND_LE, node, relational());
		if (consume(">"))
		  node = new_node(ND_GT, node, relational());
		if (consume(">="))
		  node = new_node(ND_GE, node, relational());
		else
		  return node;
	}
}

Node *add() {
	Node *node = mul();
    
	for (;;) {
      if (consume("+"))
        node = new_node(ND_ADD, node, mul());
      else if (consume("-"))
        node = new_node(ND_SUB, node, mul());
      else
        return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
	if (consume("+"))
	  return primary();
	if(consume("-"))
	  return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}


Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  // 次のトークンが変数（識別子）の場合
  Token *tok = consume_ident();
  if(tok) {
	  Node *node = calloc(1, sizeof(Node));
	  node->kind = ND_LVAR;

	  LVar *lvar = find_lvar(tok);
	  if (lvar) {
		  node->offset = lvar->offset;
	  } else {
		  lvar = calloc(1, sizeof(LVar));
		  lvar->next = locals;
		  lvar->name = tok->str;
		  lvar->len = tok->len;
		  lvar->offset = locals->offset + 8;
		  node->offset = lvar->offset;
		  locals = lvar;
	  }
	  return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}