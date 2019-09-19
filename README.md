# compiler-book
rui ueyama さんのオンラインブックの進捗

## 用語のメモ

### スタックポインタ

スタックはスタックの1番上のアドレスを保存する1つの変数のみを使って実装することができる。  
スタックトップを保持している記憶領域のことを __スタックポインタ__ という。  
x86-64 では関数を使ったプログラミングをサポートするために、スタックポインタ専用のレジスタと  
そのレジスタを利用する命令をサポートしている。  
スタックにデータを積むことを __プッシュ__ 、  
スタックに積まれたデータを取り出すことを __ポップ__  という。  

関数呼び出しの実例  

```
int plus(int x, int y) {
  return x + y;
}

int main() {
  return plus(3, 4);
}
main:
	mov rdi, 3
	mov rsi, 4
	call plus
	ret
```

上記のコードに対応するアセンブリ。  

アセンブラにおいては第一引数は RDI レジスタ  
第二引数は RSI レジスタに入れるという約束になっている  
call 命令が実行されると  

1. 次の命令のアドレスをスタックにプッシュ
2. call の引数として与えられたアドレスにジャンプ

```
.intel_syntax noprefix
.global plus, main

plus:

	add rsi, rdi
	mov rax, rsi
	ret
```

ret が行っていることは  

* スタックからアドレスを1つポップ
* そのアドレスにジャンプ

ret は call が行ったことを元に戻して、呼び出し元の関数の実行を再開する命令。  
このように call と ret は対になる命令として定義されている。  

上記の例では plus からリターンしたところにあるのは main の ret 命令。  
元の C コード では plus の返り値をそのまま main から返すということになっていた。  
ここでは plus の帰り値が RAX に入った状態になっているので、そのまま return することにより  
それをそのまま main からの返り値にすることができる。  


## ユニットテスト

テストコードを書くことにより、同じようなテストを手で毎回実行して確認をする作業を  
回避できる。  
ユニットテストというものは __自分の書いたコードを1発で動かして結果を機械的に比較できればよい__  


## トークナイザ（字句解析）

空白文字を読み飛ばす方法として、式を読む前に入力を単語に分割するという方法。  
日本語や英語と同じように算数の式やプログラミング言語も、単語の列から成り立つと考えることができる。  
例えば 5+20 は 5, +, 20, -, 4 という5つの単語でできていると考えることができる。  
この __単語__ を __token__ という。 token の間にある空白文字というのは token を区切るために  
存在しているだけで、単語を構成する一部分ではない。したがって文字列をトークン列に分割するときに  
空白文字を取り除くこのは自然なことである。文字列をトークン列に分割することを __tokenize する__ という。  

## BNF

__Backus-Naur form (バッカス・ナウア記法)__ とは生成規則をわかりやすく記述するための1つの記法。  

## 再帰下降構文解析

1つの生成規則を1つの関数にマップするという構文解析の手法。  
9cc で扱うパーサでは、トークンを1つだけ先読みしてどの関数を呼び出すか  
あるいはリターンするかということを決めている。このようにトークンを1つだけ先読みする  
再帰下降パーサのことを __LL(1)パーサ__ という。  
また LL(1)パーサが書ける文法のことを __LL(1)文法__ という。  

## スタックマシン

スタックマシンは、スタックをデータ保存領域として持っているコンピュータのこと。  
したがってスタックマシンでは「スタックにプッシュする」と「スタックからポップする」  
という2つの操作が基本操作になる。  
プッシュではスタックの1番上に新しい要素が積まれる。ポップではスタックの一番上から要素が取り除かれる。  

スタックマシンにおける演算命令はスタックトップの要素に作用する。  

```
例) 
ADD 命令
スタックトップから2つの要素をポップしそれを加算して結果をスタックにプッシュする  
SUB, MUL, DIV 命令
ADD 命令と同様にスタックトップから2つの要素を取り出し  
それらを減算・乗算、除算した1つの要素で置き換える  
```

PUSH 命令は引数の要素をスタックトップに積むものとする。  
POP 命令はスタックトップから要素を1つ取り除く命令であるとする。  

これらの命令を使って `2*3+4*5` を計算すると以下のようになる

```
// 2*3
PUSH 2
PUSH 3
MUL

// 4*5
PUSH 4
PUSH 5
MUL

// 2*3+4*5
ADD
```

## RSP（スタックポインタ）

RSP はスタックトップのメモリアドレス。  
PUSH 命令でデクリメント  
POP  命令でインクリメント  
される。  

## RBP（ベースポインタ）

RBP は関数内においてスタック領域を扱う処理の
基準となるメモリアドレス。  
