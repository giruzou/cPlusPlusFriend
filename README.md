# 君はC++クラスのフレンズなんだね

フレンズと言ったらC++でしょう。

```c++
class Train {
    // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
    friend std::ostream& operator <<(std::ostream& os, const Train& train);
    friend std::istream& operator >>(std::istream& is, Train& train);
    friend boost::serialization::access;

    // ユニットテストはprivateメンバを読めるフレンズなんだね。たーのしー!
    FRIEND_TEST(TestSerialization, Initialize);
    FRIEND_TEST(TestSerialization, Std);
    FRIEND_TEST(TestSerialization, Boost);
    FRIEND_TEST(TestSerialization, Invalid);
};
```

[cppFriends.cpp](cppFriends.cpp)では、friend関数以外にも「フレンズなんだね」を連発していますが、まあ流行語ですし。そういう意味で、「すごいHaskellたのしく学ぼう!」という題の本は、時代の先を行っていたのですね。

## やめるのだフェネックで学ぶC++

「やめるのだフェネック」あるいは「フェネックやめるのだ」がどこからきたのかよく分からないのですが、もしかして元ネタは[シェーン、おいやめろ！](http://labaq.com/archives/51829457.html)なのでしょうか。

1. やめるのだフェネック! enumを何でもintにキャストしてはいけないのだ! std::underlying_typeを使うのだ!
1. フェネックやめるのだ! XMMレジスタの値を、素のuint64_t[]に保存してはいけないのだ! 16 bytes アライ~~さん~~ンメントが必要なのだ! 単にreinterpret_castするだけじゃダメなのだ!
1. (そろそろしつこいので以下同文) XMMレジスタを収める配列を確保するのに、reinterpret_castを使ってアラインメントし直すのはめんどくさいのだ! std::aligned_storageを使えば済むのだ!
1. x64 ABIで、asmコードからCの関数を呼び出すときは、rspレジスタを16 bytes境界に合わせないといけないのだ! そうしないと、C++ライブラリの中で突然クラッシュすることがあるのだ!
1. x86で、intを一度に32ビット以上シフトするのはダメだ! REX.Wがないときのシフト回数は有効桁数が5ビットしかないのだ!
1. ```int64_t v = 1 << 32;``` はint64_tではなくintをシフトしているのだ! 0x100000000ではなく0が入るかもしれないのだ! int64_tの変数に代入してから<<=でシフトするのだ!
1. long longからIEEE 754 doubleへの変換は暗黙に行われるが、精度が落ちることがあるのだ! 仮数部は52ビットしかないから足りないのだ!
1. 配列をループで回すときのインデックスを何でもintにしてはいけないのだ! sizeof(size_t) > sizeof(int)だと4Gあたりで動作がおかしくなることがあるのだ!
1. 固定アドレスの格納先を、uint32_tとuint64_tとで、#ifdefで切り替えるのはやめるのだ! uintptr_tを使うのだ!
1. ポインタの差を、intに入れるのは嫌なのだ! ptrdiff_tを使うのだ! 符号ありだから、printfは"%td"を使うのだ!
1. size_tのビット数が分からないからといって、printfの書式指定に%luと書くのはダメだ! %zuと書くのだ! MinGW-w64 gccだとランライムが%zuを解釈しないからコンパイルになるって、それなら仕方ないのだ...
1. 組み込み32bit CPU向けコードのユニットテストを、x86 Google Testで書くのに、x86_64コンパイラは要らないのだ! size_tの違いを吸収するのは大変なのだ! target=i686だけインストールして、x86_64 multilibはインストールしないのだ!
1. クロスコンパイル環境の開発では、ターゲットでは使えるがホスト(ユニットテスト)では使えないキーワードを、マクロを介して使って欲しいのだ! 直接```__forceinline```とか```__packed```と書かないで欲しいのだ! そういうことはコーディング規約で決めるのだ!
1. sizeofに型名を入れてはいけないのだ! 変数の型が変わった時オーバランするのだ! sizeof(*pObject)とすれば、ポインタpObjectが指すもののサイズが得られるのだ!
1. sizeofにリテラルを渡すのはやめるのだ! sizeof('a')はCとC++で違うのだ! [(参考)](http://david.tribble.com/text/cdiffs.htm#C99-char-literal)
1. extern "C"はC「リンケージ」を指定しているのであって、C言語としてコンパイルしろとは言っていないのだ! CとC++の両方でコンパイルされるヘッダファイルなら、両方で通用するコードを書く必要があるのだ!
1. C++とCで共用するヘッダファイルで、構造体の内部構造体(入れ子になっている構造体)の名前はすべての構造体で一意でなければならないのだ! そうでないとCでは構造体を再定義しましたとエラーになるのだ! そもそも構造体を入れ子にしなければ済むのだ!
1. 配列や構造体はどんなに大きくしてもいいという訳ではないのだ! ARMコンパイラは4GBまでできても、MinGW-32bit GCCは0x7fffffffより大きくできないのだ! 大きすぎる構造体はユニットテストの妨げになるのだ!
1. 自分の.cから他の.cにある変数を使いたいからといって、自分の.cにexternを足さないで欲しいのだ! 宣言が```int* a```で、定義が```int[] a```だと、```a[0]```にアクセスしたときsegmentation faultで落ちるのだ! ODR違反とカプセル化の危機なのだ! 宣言はヘッダファイルに書いて.cで共有するのだ!
1. 上記の例は、ポインタと配列が違うことを示しているのだ!
1. ポインタが壊れていることを、デバッガでちゃんと見つけて欲しいのだ! gdbでinfo registersと打つのだ! 上位ビットがカーネル領域に入っていたり、下位ビットのアラインメントがとれていないのは疑わしいのだ!
1. 引数の"size"が、個数なのかbyte単位のサイズなのかword単位のサイズなのか分からないのは困るのだ! 区別できるような名前をつけて欲しいのだ!
そもそも1 wordって何byteか分からないのだ!
1. 時間には単位を書いてほしいのだ! usがμ秒だということはなんとなく分かるのだが、usecと書かないと我々とか米国と読めてしまうのだ!
1. 「どんな型の関数へのポインタでも入る物」として、void*を使うのはやめるのだ! データへのポインタとコードへのポインタは互換ではないのだ!  [(参考)](http://stackoverflow.com/questions/5579835/c-function-pointer-casting-to-void-pointer) (boost::anyなら [Value typeの要件](http://www.boost.org/doc/libs/1_63_0/doc/html/any/reference.html#any.ValueType) は満たしているはずですが)
1. 関数への参照は、&をつけるのとつけないのと使い分けるのだ! テンプレートマッチングに失敗することがあるのだ!
1. 参照とポインタは「どっちも間接アドレッシング」ではないのだ。ポインタはnullable(optional)とかiteratorとか、もっと多義的なのだ!
1. コンテナの中身をfor文でなめているときに、呼び出し先の関数でコンテナの中身を増減したらイテレータが無効になることがあるのだ! 無効なイテレータにアクセスするのはまずいのだ! プロセスの危機なのだ! [参考](https://stackoverflow.com/questions/6438086/iterator-invalidation-rules)
1. 演算子の優先度を覚えるのは無理なのだ! ```expr ? 2 : 3 + 4;``` ```4 + expr ? 2 : 3;``` ```4 + (expr ? 2 : 3);``` は違うのだ! 括弧をつけるのだ!
1. 同じx86 CPUだからって、64ビットアプリと32ビットアプリで、浮動小数が同じ計算結果を返すと仮定してはだめなのだ! SSEは内部64ビットだが、x87は内部80ビットで計算しているのだ! [(参考)](http://blog.practical-scheme.net/shiro/20110112-floating-point-pitfall) 数の比較結果が前者は==で後者が!=になることがあるのだ! [(例)](cFriends.c)
1. 浮動小数をprintf("%.0e")して7文字(-1e-308)で収まると、決め打ちするのはやめるのだ! -infinityと表示するときは9文字なのだ!
1. std::wstring_convertによるUTF-8/16変換は、例外を捕捉するのだ! わざと冗長にUTF-8エンコードした文字列を入力すると、std::range_errorが飛ぶことがあるのだ!
1. ビットフィールドを上から下に並べても、MSBから順に並ぶとは限らないのだ! エンディアンとコンパイラの仕様を確認するのだ!
1. ビットフィールドのアラインメントに隙間のビット数を指定したダミーのメンバを加えると、他のメンバのビット数が変わった時にダミーを変え忘れてメンバのレイアウトがずれてしまうのだ! 無名でサイズ0のフィールドを使うのだ!
1. unionを使ってstructをuint8_t[]に読み替えるのは、実行時にはできても、constexprでコンパイル時にできるとは限らないのだ!
1. コメントに「この変数は符号なしのはず」とか書いてはいけないのだ! static_assert(std::is_unsigned)を書くのだ!
1. ```do { ... } while(--i >= 0); ```は、iが符号なし整数だと永久に終わらないのだ! プロセスの危機なのだ!
1. マクロに複数の文を入れるときは、```do { ... } while(0) ```で囲むのだ! そうしないと、if文の直後でそのマクロを使ったときに、予想外の動作をすることがあるのだ!
1. 複数行マクロを入れる do {...} while(0); の;は余計なのだ! マクロを展開する場所で、直後にelseを置くとコンパイルエラーになるから;を取り除くのだ!
1. do {...} while(0)はループ実行のオーバヘッドが掛かるから無駄とか言っちゃいけないのだ! 今どきのコンパイラは最適化でこのループを取り除くのだ! アセンブリコードを出力して確認するのだ!
1. C++で「関数っぽいマクロ」を作ったらダメなのだ! function_traitsで返り値や引数の型を取得できないのだ!
1. putcというメンバ関数を作るのはダメなのだ! putcは関数ではなくマクロで実装されていることがあるから、そのときは意味不明なエラーメッセージが出るのだ!
1. コンパイラの最適化を甘く見てはいけないのだ! Tail recursionをiterationにできるなら、tail callもjmpにできるのだ! 自作のassemblyよりかしこいのだ!
1. switch-caseがいつも多段if-elsifになるとは限らないのだ! clangはcaseが多いときにルックアップテーブルを作ってジャンプするのだ!
1. switchに書けるのは整数だけで、switchのcaseに書けるのは整数定数だけなのだ! オブジェクトが==で比較できても書けないのだ! 多段if-elseではないし、Rubyのcase-whenとも違うのだ!
1. memsetが常にmemset関数呼び出しになるとは限らないのだ! clangは短い固定長領域については、mov命令で済ませることがあるのだ!
1. 条件演算子 (expr) ? t : f よりif-elseの方が分かりやすいとか言わないで欲しいのだ! それはconditional move命令に変換することで、分岐予測が外れるペナルティを避ける必要があったのだ! 歴史的経緯には相応の理由があることを分かって欲しいのだ!
1. (a) ? b/a : c; をasmで書くのに、b/aとcを求めてからcmovで選ぶのはまずいのだ! cを選ぶ前に0除算例外が発生することがあるのだ! コンパイラはこういうときcmovではなく分岐にしてくれるのだ!
1. 命令の依存関係は、レジスタだけでなくフラグにもあるのだ! INCよりADDを使う理由とか、SHLXをとか調べるのだ!
1. XOR r,rは単にレジスタrを0にするだけじゃないんだ! レジスタrとフラグの依存関係を絶って、実行効率を改善するためのイディオム(dependency breaking idiom)なのだ! フラグを変えたくなければMOV r,0にするのだ!
1. 非PODのオブジェクトをmemcpyしてはいけないのだ! memmoveもダメだ! vtableへのポインタもコピーされてしまうのだ! 派生クラスのメンバが切り捨てられて不定値に置き換わってしまうのだ! clang++ 4.0.0は警告してくれるが、g++ 6.3.0は警告しないのだ!
1. vtableを実行時に書き換えて「オープンクラス」みたいにはできないのだ! vtableはread-onlyセクションに置かれているのだ! 書き換えようとするとSegmentation faultで落ちてしまうのだ!
1. クラスにoffsetofを取ると警告が出るのだ! Non standard layout型に対してoffsetofを取る意味があるのか考えるのだ!
1. メンバ変数を一括クリアするのに、memset(this, 0, sizeof(*this))はまずいのだ! vtableへのポインタもクリアしてしまうのだ! せめてstatic_assert(std::is_standard_layout)で調べてからにして欲しいのだ!
1. 仮想関数呼び出しが常にvtableアクセスを伴うとは限らないのだ! 具象型がコンパイル時に分かるときはそのメンバ関数を直接呼び出すのだ(devirtualization)!
1. memsetを使ってbyte単位以外の値でメモリを埋めるのは無理なのだ! std::fillを使うのだ!
1. 二つの構造体インスタンスが同値かどうか比較するのに、memcmpは嫌なのだ! 不定値のパディングも比較するので、同値なのに異なると判断することがあるのだ! 比較関数を定義するか、0で埋めておくのだ!
1. snprintf(dst,N,"%s")で長さNのときにdstにN文字書き込もうとすると最後はNUL終端されるが、strncpyで長さNのときにN文字コピーしようとすると最後は終端されないのだ! バッファオーバランの危機なのだ!
1. 固定長配列同士のmemmove/memcmpは、サイズが合っているかどうかstatic_assertを書いて欲しいのだ! 配列のsizeofは配列全体のbyte単位サイズを返してくれるのだ!
1. 実行時に任意の正規表現を入力として受け取るのはやめるのだ! 複雑な正規表現を食わされて落ちる(ReDoS)ことがあるのだ! [(参考)](https://www.checkmarx.com/wp-content/uploads/2015/03/ReDoS-Attacks.pdf)
1. thisと引数が同じオブジェクトかどうか(二つの引数が同じオブジェクトかどうか)確かめずに、片方から他方にメンバをコピーするのはやめるのだ! memcpyで領域が重なっているときのように要素が消滅してしまうのだ! [(NaiveCopy)](cppFriends.cpp)
1. 確かにC99の機能はC++でも使えるが、restrictはコンパイルエラーになることがあるのだ! 本当にrestrictが必要か考えるのだ!
1. 立っているビット数をfor文で数えるのは遅いのだ! コンパイラのマニュアルから __builtin_popcount とかを探すのだ!
1. __builtin_popcountの引数はunsigned intなのだ! long long intを渡すと正しい答えを返さないことがあるのだ! テンプレートではないのだ! __builtin_popcountllとかもみるのだ!
1. 整数に対するlog2がライブラリにないからって、ループを回すのはイケてないのだ! BSR命令か__builtin_clz()を使うのだ
1. 負の整数を2で割るのに、算術右シフト一回だけでは済まないのだ! -1を2で割っても-1になってしまうのだ! コンパイラがどんなコードを出力するか確認するのだ!
1. ビットフィールドを設定/取得するのに、マスクしてシフトするのは唯一の方法ではないのだ! 最近のx86はPEXT/PDEP命令が使えるのだ!
1. ```__attribute__((always_inline))```は常にインライン展開できるとは限らないのだ! 再帰呼び出しはインライン展開できないのだ! そもそもinlineはヒントであって命令ではないのだ!
1. Strict aliasing rule警告の意味が分からないからって無視してはいけないのだ! [(参考)](http://dbp-consulting.com/tutorials/StrictAliasing.html) そもそもエンディアン変換なら、自作しないでntohlとかBoost.Endianとか探すのだ!
1. pragmaで警告を抑止してよいのは、コードレビューで承認されてからだ! -Wall -Werror は必須なのだ! (私見ですが、コーディング規約とは"comply or explain"だと思うのです。従うのが原則ですが、従わない方がよいコードになるのであればその理由を説明してプロジェクトの了承を得る。結局はコーディング規約を守らせるコストと、守れないときのリスクの、トレードオフの決断ですし、そのトレードオフは開発フェーズで変わるものでもあるので。)
1. コンパイラの警告を無視した箇所を、静的解析ツールに指摘されるのはやめるのだ! コンパイラが数秒で教えてくれることを、翌朝に教わるのは開発効率が低すぎるのだ!
1. 「符号付きと符号無しの整数式の間での比較です」という警告は、文字通りの意味とは限らないのだ! 同じデータ構造を、異なるファイルで、異なる基本型で扱っているのが原因かもしれないのだ!
1. 「変数が使われていない」という警告が出たからと言って、その変数定義を消すのはちょっと待つのだ! コンパイラは似たような変数と取り違えていないかを、指摘しているのかもしれないのだ!
1. ```/* Local Variables: c-file-style: "stroustrup" */``` を理解できないからって消さないで欲しいのだ! それはEmacs上でソースコードを整形するのに必要なのだ!
1. 2個のオブジェクトを交換するコードを自作してはいけないのだ! std::swapはno throw保証なのだ!
1. 出力ファイルストリームのcloseを、いつでもデストラクタ任せにすると、closeで書き出しに失敗したことを検出できないのだ! デストラクタはnoexceptだから呼び出し元に結果を通知できないのだ!
1. 関数内で異常が発生したから、それまで確保したメモリやファイルハンドルを解放する処理を、gotoで関数の最後に飛んで行うのはやめるのだ! Cではそれでよいが、C++ではRAIIを使わないと例外安全にならないのだ!
1. C++にfinally節やensure節はないのだ! リソースリークの防止にはRAIIを使うのだ!
1. ifとかforとか無いただの{}を、無駄だからといって削除してはならないのだ! それはブロックスコープで、変数の有効範囲とデストラクタ呼び出しを規定しているのだ!
1. 実行環境を確認せずに、いきなりnoexceptと書かないで欲しいのだ! 例外中立にして欲しいのだ! MinGW32 + pthreadGCE2.dll + clangだと、[pthread_exit](https://github.com/Tieske/pthreads-win32/blob/master/pthreads.2/pthread_exit.c)が例外を投げて、[スレッドエントリ関数](https://github.com/Tieske/pthreads-win32/blob/master/pthreads.2/ptw32_threadStart.c)が拾うまでに、noexcept違反でstd::terminateされてしまうのだ! (pthreadGCE-3.dllではこうならず、スレッドを正常に終了できます)
1. 何もしないデストラクタを{}と定義するのはダメだ! =defaultを使うのだ! 理由はEffective Modern C++ 項目17に書いてあるのだ!
1. ユニットテストが書きにくいからって、#defineでprivateをpublicに置き換えちゃいけないのだ! アクセス指定子を超えたメンバ変数の順序は入れ替わることがあるのだ!  [(参考)](http://en.cppreference.com/w/cpp/language/access) friendを使うのだ!
1. classはstructのsyntactic sugarにすぎないのだ! デフォルトの公開範囲が違うだけで、できることに変わりはないのだ!
1. ソフトウェアのテストは、ソフトウェアの完全性を保証するものではないのだ! ソフトウェアの品質が十分でないという帰無仮説をテストで支持しようとして、テストが全部通ったので、帰無仮説を棄却して品質が十分高いとみなすことなのだ。
1. メンバ変数名の目印のアンダースコアは、名前の先頭につけちゃいけないのだ! _で始まり次が英大文字の名前はC++処理系の予約語なのだ!
1. 短絡評価の||を「または」と読まないのだ! 「さもなくば」と読むのだ! &&は「だったら」「なので」と読むのだ!
1. メンバ変数を増やしたとき、複数あるコンストラクタすべてに、そのメンバ変数の初期化を加えるのを忘れちゃいけないのだ! 可能ならメンバ変数の定義と併せて初期化して、それ以外の初期化方法だけコンストラクタに書けばよいのだ!
1. コンストラクタでm_(m_)としているのは、メンバ変数をメンバ変数で初期化しているから、初期化していないのと同じなのだ! m_がstd::vectorへの参照だと、コンパイラは教えてくれないことがあるのだ!
1. コードにstd::coutを直書きしてはいけないのだ! ユニットテストが書けないでないか! std::ostreamへの参照を渡すのだ! もちろんstd::cinもだ! ユニットテストでキー入力するのは大変なのだ!
1. 組み込み系だからって、即値アドレスをreinterpret_castしてレジスタやメモリにアクセスすると、PC上でユニットテストできないのだ! アクセサ関数を経由するのだ!
1. デバッグprintfを#ifdef ... #endifすると、うっかりリリースビルドで消し忘れて、変なコンソール出力が出てしまうのだ! DEBUG_PRINTFマクロを使って、デバッグ時だけ出力するのだ!
1. 製品コードの中に直接```#ifdef UNITTEST ... #endif```と書くのは極力避けたいのだ! そこはdependency injectionを使うのだ!
1. コンテナの要素の型をソースコードにべた書きしたら、コンテナの型を変えた時に修正が大変なのだ! std::vector::value_type と auto と decltypeがあるじゃないか!
1. 配列の要素数を ```#define arraySizeof(a) (sizeof(a)/sizeof(a[0]))``` で数えるのはやめるのだ! aにポインタを渡すと、エラーにならずに変な値が返ってくるのだ! テンプレートとconstexprを使うのだ!
1. 長さNの配列を、for(int i=0; i<N; ++i)で回すのはやめるのだ! 配列の長さを増やしたとき、for文を変え忘れるのだ! インデックスiが必要なら、Nをsize()かconstexpr式とかで調べるのだ! iが不要ならrange-based forかstd::for_eachを使うのだ!
1. ```#if (sizeof(uintptr_t) > 4)```とは書けないのだ! ```if constexpr (sizeof(uintptr_t) > 4)```が使えるようになるのを待つのだ!
1. #defineで再帰呼び出しはできないのだ! テンプレートメタプログラミングを使うのだ! (Boost Library.Preprocessorもありますが...)
1. いくらマクロより関数テンプレートの方がいいからって、 ```#define WARN(str) printf("%s at %d", str, __LINE__)``` は関数にはできないのだ! WARNを呼び出した場所ではなく、WARNを定義した場所の行番号が表示されてしまうのだ!
1. 自作assertをマクロでなく関数で書くのに、MyAssert(int cond)にすると、ポインタを渡すとコンパイルエラーにになるのだ! 引数にboolとかtemplate T&&を渡すことを検討するのだ! boolにnullptrを渡せないのは仕方ないのだ!
1. 関数の動作を```#ifdef COLOR ... #endif```で切り替えると、COLOURと打ったときに```...```が除外されてしまうのだ! if (定数式)が使えるならそうするのだ! コンパイラが綴りの違いを見つけてくれるのだ!  C++17ではif constexprが使える(予定な)のだ!
1. if constexprは実行しないブロックを含めてコンパイル可能でなければならないのだ! PEXCEPTION_POINTERS->ContextRecord->{Rip|Eip} はどちらか一方がコンパイルエラーになってしまうのだ! ここは```#ifdef __x86_64__```が必要なのだ!
1. 複数行のコードをとりあえずコメントアウトするのに```/* */```を使うと、```/* */```が入れ子になっておかしくなることがあるのだ! #if 0 - #endifにして欲しいのだ!
1. 実行ファイルのバイナリサイズを削減するのに、使ってなさそうな関数を探して #if 0 - #endif を試すのはまわりくどいのだ! LTO(Link Time Optimization)を使うのだ!
1. -flto オプションはすべてのソースコードのコンパイルとリンカにつけるのだ! -fltoをつけたりつけなかったりすると、リンクエラーになることがあるのだ!
1. uint32_t = 1を一度に35回シフトして0になったのは、LTOが有効だからだ! LTOがないと8なのだ! Nビット整数を一度にN回以上シフトするのは未定義動作だから直すのだ!
1. テンプレートマッチングをstd::is_pointerだけで済ましてはいけないのだ! 配列T(&)[SIZE]とstd::is_null_pointerに対するマッチングも必要なのだ!
1. free(p);をif(p){}で囲む必要はないのだ! freeにNULLを渡しても無害なのだ! こういうインタフェースはnull object patternにして欲しいのだ! アライさんはその辺ばっちりなのだ!
1. 関数へのポインタにnullptrは確かに使えるが、できれば何もしない関数を設定して欲しいのだ! NULL検査はわずらわしいし忘れやすいのだ!
1. f(uint8_t)とf(BYTETYPE)とf(unsigned char)を同時には定義できないのだ! 関数を再定義してますと言われてしまうのだ! プログラマには違う型に見えても、コンパイラには区別がつかないのだ!
1. 関数の動作を何でもかんでもboolの引数で切り替えたら、呼び出す側のコードを読んでtrueとかfalseとか書いてあっても、何をしたいか分からなくなるのだ! enum classでパラメータに名前を付けるのだ!
1. そのエラーメッセージ"has incomplete type T"は、T型が不完全型だと言ってる訳じゃないのだ! インクルードしているヘッダファイルが足りないから、T型の定義が見つからなくて困ると言っているのだ!
1. ヘッダファイルをインクルードする順番が入れ替わるとエラーになるのは困るのだ! 宣言と定義を適切にヘッダファイル群に分割して、依存関係を一方向にそろえるのだ!
1. 「みんながインクルードしているヘッダファイルの定義を書き足したらフルビルド」を避けるのだ! 宣言と定義を分離するのだ! enum classの前方宣言を活用するのだ!
1. Google Testなどサードパーティーのヘッダファイルは、-Iではなく-isystemでインクルードするのだ! そうしないと対処不能な警告が出て、-Werrorだとビルドが止まってしまうことがあるのだ!
1. パラメータをちょっといじりたいからって、.cppの定数を手で書き換えるのはイケてないのだ。一貫性の無いパラメータが設定されて事故の元なのだ! 設定ファイルからRubyで.hを自動生成してインクルードするのだ! スクリプトなら一貫性のあるパラメータを出力できるし、異常値ならエラーにすることもできるのだ!
1. 定数伝搬するつもりがない定数は、宣言だけ公開して定義は.cppに入れておけば、定数を変えてもその.cppだけリコンパイルすれば済むのだ!
1. ヘッダファイルを追加したのに、add-commitし忘れて帰っちゃ嫌なのだ! レポジトリをチェックアウトした他の人に、コンパイルエラーの危機が訪れるのだ!
1. #include文のファイル名は、大文字と小文字を間違えちゃ嫌なのだ! Windowsでは問題なくても、Linuxではファイルがインクルードできないと言われてしまうのだ!
1. #include "../detail/foo.h"と、.cppからディレクトリを横断するパスを書かないで欲しいのだ! それは実装部のヘッダだから、他モジュールに公開するつもりはないのだ! Makefileのインクルードパスは、ヘッダファイルの公開範囲を規定しているから従って欲しいのだ!
1. 関数宣言と関数定義の引数名が違うのはまずいのだ! 片方がradiusで他方がdiameterだと、どちらが正しい仕様だか分からないのだ!
1. 公開するヘッダファイルに、実装にだけ必要な宣言や定義も書いて「全部入り」にしちゃいけないのだ! 他のプログラマがドキュメントに無い「隠し機能」だと思って使ってしまうのだ! そういうものはdetailディレクトリ以下に分離するのだ! 隠し機能を保守するのはコストが高過ぎるのだ!
1. タイピングが大変だからって、using namespace std;って書いちゃいけないのだ! boostと衝突したらどうするのだ! もうすぐC++17でstd::anyとstd::optionalがくるんだぞッ!
1. そのboost::regexをstd::regexに置き換えるのはやめるのだ! その再帰正規表現は入れ子になった括弧を、一番外側の括弧ごとに分けるのだが、std::regexは再帰正規表現をまだサポートしていないのだ!
1. boost::anyオブジェクトにchar*型の値を入れたとき、 boost::any_cast<char *> ではなく boost::any_cast<const char *> で取り出すのはやめるのだ! boost::bad_any_castが飛んでくるぞ! typeidとは違って、constを無視しないのだ!
1. boost::fusion::vectorの番号付き(vectorN)は、C++11で廃止になるのだ! C++03では型引数が同じでも、番号付きと番号なしでは違う型だから注意が必要なのだ! Boost.Fusionの[仕様](http://www.boost.org/doc/libs/1_64_0/libs/fusion/doc/html/fusion/container/vector.html)をよく読むのだ! boost::anyでキャストするとき気を付けるのだ!
1. C++98の自作STATIC_ASSERTマクロは、C++11では公式static_assertにして欲しいのだ! どこでエラーが起きたかは分かっても、なぜ起きたか分かりにくいのだ! ```#if __cplusplus >= 201103L```を使うのだ
1. 基底クラスと派生クラスのtypeidは異なるのだ! 素直にmarker interfaceを使うのだ!
1. 空の構造体がたくさんあるからって、typedefで全部まとめて一つにするのはやめるのだ! それらはBoost.MultiIndexのタグなのだ!
1. CとC++で共用するヘッダファイルで、空の構造体のサイズが異なることがあるのだ! gccは0 byteでg++は1 byteなのだ! そもそもCで空の構造体は作れないのだ!
1. ラムダ式の型をtypeid().name()で取得しようとするのは無駄なのだ! それはコンパイラが一意なものを決めるのだ!
1. 関数の返り値型をautoにしたら、return vec[i]でベクタの要素への参照は返せないのだ! 要素のコピーが値渡しされるのだ! そこはdecltype(auto)が必要なのだ!
1. そのreturn文の括弧は余計なのだ! 返り値がdecltype(auto)のときにreturn (v);すると、ローカル変数vへの参照を返すのだ! 未定義動作の危機なのだ!
1. ```__PRETTY_FUNCTION__``` は「かわいい」関数名じゃないのだ! 「見た目がきれい」な関数名なのだ!
1. ```__PRETTY_FUNCTION__```の長さを決め打ちするのはやめるのだ! 引数にboost::multiprecisionとか取るテンプレートは、数百文字になることがあるのだ!
1. BBC World Serviceの"Popular Programmes and Clips"は人気のソフトウエアプロジェクトじゃないのだ! 番組のことなのだ! ちなみにNAND flash memoryに書き込むのは、動詞で"program"だ!
1. 関数名を付けるときは辞書をひいて欲しいのだ! 間違った英単語も困るが、ローマ字はもっと困るのだ!
1. テストの成功をOKと書くのはよいが、失敗をNGと報告しないで欲しいのだ! Failedと書かないと通じないのだ!
1. シュミレーションはちょっと恥ずかしい誤字なのだ!
1. 変数名をtimeと書いて時間と読ませるのは、点(時刻)なのか幅(狭義の時間)なのか分からないのだ! timestampかdurationと書いてほしいのだ!
1. 二つのオブジェクトが「同じ」と言われても、同値(same)なのか同一(identical)なのか曖昧なのだ! はっきり書くのだ!
1. 任意精度実数の「任意」に相当する形容詞は、anyではないのだ! arbitraryなのだ! 単語を間違えると、検索で引っかからないのだ!
1. 実行権を取れるまでセマフォを待つのはacquireなのだ! Getと書くとセマフォのインスタンスを取得すると読めてしまうのだ! 獲ると取るは違うのだ!
1. VRAMの転送は、copyではなくblit(bitblt)の方が適切ならそう書いて欲しいのだ!
1. 変数がintだからといって、変数名の先頭にiをつけてiVarにするのはやめるのだ! 後で64ビット化したときにlong longに変えても、保守プログラマはllVarにはしてくれないのだ!
1. Gitの公式[ガイドライン](https://git.kernel.org/pub/scm/git/git.git/tree/Documentation/SubmittingPatches?id=HEAD)に従うなら、コミットメッセージは命令形(imperative mood)で書いて欲しいのだ! 他のプロジェクトもこうしておけば、英語で迷うことは減るのだ!
1. 問題を"problem"と報告したら、相手に「お前のせいで困っている」と言っているようで怒らせてしまうのだ! 取り組むべき課題は"issue"と訳すのだ!
1. Embedded systemを「埋め込み」システムと訳すのは嫌なのだ! 組み込みシステムと訳して欲しいのだ!
1. その代入は最適化で削除されるかもしれないのだ! メモリとレジスタに書きにいかないのだ! デバッグビルドでは動作してもリリースビルドでは正しく動かないのだ! volatileをつけるのだ!
1. ```volatile int value = *p;```はpの指すものがvolatileではなく、valueがvolatileなのだ! pの指すものが更新されても、valueは連動して更新されないのだ!
1. printfを入れたら動いた、とか言っちゃダメなのだ! それはprintfをまたぐ最適化が抑止されたから、volatileではない変数を読みに行くようになっただけなのだ! Memory mapped I/Oを読むときは、忘れずにvolatileをつけるのだ!
1. 関数定義をヘッダに移したら動かなくなった、とか言っちゃダメなのだ! 定義がみえる関数呼び出しをまたぐ最適化が適用されたから、volatileでない変数を読みに行かなくなったのだ! Memory mapped I/Oを読むときは、忘れずにvolatileをつけるのだ!
1. volatileでスレッド間共有変数の同期は取れないのだ! std::atomicが必要なのだ! 競合動作の危機なのだ! [(Counter)](cppFriends.cpp)
1. 32bitレジスタマシンで、64bitリアルタイムクロックを読むのに、単に下位32bit-上位32bitと読むと、時刻が逆転することがあるのだ! 上位が一致するまで読み直すのだ!
1. std::atomic<int>は明示的に初期化する必要があるのだ! 初期化を忘れてもコンパイラは教えてくれないのだ!
1. プロセッサのstore命令は、そのプロセッサの論理スレッドからみた順序は正しくても、バスや他のプロセッサからも同じ書き込み順序に見えるとは限らないのだ! Memory fenceが必要な場合があるのだ!
1. autoは便利だが万能ではないのだ! std::atomic<int> aから値を取り出すのに、auto v=a;とは書けないのだ! atomicなオブジェクトをコピーしようとして失敗するのだ! 明示的な型に代入するか、a.load()が必要なのだ!
1. volatile T*へのキャストをどう書く分からないからって、Cキャストを使っちゃいけないのだ! const_castを使うのだ!
1. C++キャストは確かに字面が長いが、だからって横着してCキャストにすると、ポインタのconstが剥がれても気がつかないのだ!
1. constメンバ関数からメンバ変数を書き換えたくなったからといって、いきなりmutableとかconst_castとかしちゃいけないのだ! 呼び出し側はスレッドセーフを期待しているのだ!
1. スレッドセーフと再入可能は違うのだ! 複数のスレッドからmallocは呼べるかもしれないが、割り込みハンドラからmallocは呼べないのだ! ヒープ構造の危機なのだ!
1. VLAがスタックに置かれるとは限らないのだ! ヒープに確保する[コンパイラ](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0472k/chr1359124223721.html)もあるのだ! 割り込みハンドラからmallocは呼べないことを思い出して欲しいのだ!
1. 割り込みハンドラで自動変数のオブジェクトを生成するとき、コンストラクタがnewするのは困るのだ! メンバ変数と他のメンバ関数についても確認するのだ!
1. コメントに「このクラスのインスタンスはヒープに置かないでください」と書いて済ますのは嫌なのだ! operator newを=deleteして欲しいのだ!
1. pthread_cond_waitを直接使うと、spurious wakeup対策が必要なのだ! std::condition_variable::waitを述語付きで使うのだ!
1. 割り込みやシグナルやcondition variableは、それらの待ちに入る前に通知される場合に対処する必要があるのだ! そうでないと無限待ちになってしまうことがあるのだ!
1. グローバル変数のコンストラクタから、別のグローバル変数のインスタンス関数を呼び出してはいけないのだ! グローバル変数の初期化順序はコンパイラが決めるから、呼び出し先は未初期化かもしれないのだ! Construct on first useイディオムが必要なのだ!
1. static T& GetInstance(void) { static T instance; return instance; } はマルチスレッド環境ではシングルトンにならないことがあるのだ! シングルトンの実装方法をよく確認するのだ!
1. boost/thread/future.hppなどをインクルードする.cppファイルで、Intel Syntaxのインラインアセンブリを使うと、アセンブラがエラーを出すことがあるぞ! Intel Syntaxでインラインアセンブリを記述するなら、その.cppファイルは他と分けた方がいいぞ!
1. CreateInstance()がいつでも生ポインタを返したら、誰がdeleteするかわからなくなって、メモリリークしたり二重解放したりするかもしれないのだ! deleteして欲しければ、std::unique_ptrを返すことを検討するのだ! 生ポインタは所有権を渡さないという意志なのだ!
1. std::shared_ptrのダウンキャストは、getしてdynamic_castじゃないのだ! それじゃ何のために共有しているか分からないのだ! std::dynamic_pointer_castなのだ!
1. abi::__cxa_demangleが返したものはfreeしないと[メモリリークするのだ!](https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html) 誰がメモリを解放するか、仕様を確認するのだ!
1. リークするのはメモリだけじゃないのだ! TCP/UDP受信ポートがリークしたら、プロセス終了まで同じ番号のポートが開けなくなるのだ!
1. std::vector<T>.data()で得られるポインタは、ベクタの要素を増減すると無効になることがあるのだ! 関数の返り値にするなんてもってのほかなのだ! Segmentation faultの危機なのだ!
1. 引数としてconst T* pObjectを渡すと、ポインタpObjectが指すオブジェクトはimmutableとして扱われるが、deleteはできるのだ! deleteされたくなければ、デストラクタを非publicにするのだ!
1. thread_localを使う前によく考えるのだ! 特定のスレッドしか参照しない値は、スレッド起動時の引数で参照できるのだ! アライさんはその辺ばっちりなのだ!
1. pthread_t型の変数を、pthread_createを呼び出す前に初期化はできないのだ! pthread_tの型はopaqueで、CygwinはポインタでMinGWはstructだから、ユーザが設定できる初期値などないのだ!
1. 文字列の集合を連結するのに、```boost::algorithm::join(lines, " ");```で連結しちゃ嫌なのだ! 英語はこれが正しいけど、日本語だと「弁慶がな ぎなたを振り回し」になってしまうのだ! 文字列の末尾がUS-ASCIIかどうかで、空白を入れるか決めて欲しいのだ!
1. 海外に渡すコードに、うっかり全角空白みたいな、US-ASCII以外の文字が入るのはまずいのだ! fileコマンドを使うか、ソースコードをRubyで読んでascii_only?で調べるのだ!
1. ファイルをShift_JISで保存するのはやめるのだ! //コメントが「でゲソ」とか「可能」とかで終わると、次の行もコメント扱いされてコンパイルされなくなってしまうのだ!
1. ユニットテストを書くときは、いきなりテストを成功させてはいけないのだ! でないと、テストに成功したのか、そのテストを実行していないのか、区別がつかなくなるぞ! まずテストを実行して失敗することを確かめるのだ!
1. 時刻表記のテストケースには、うるう秒も加えるのだ! std::get_timeは"2017-01-01 08:59:60"をちゃんと解析してくれるのだ!
1. 2/3/2017は二月三日だか三月二日だか分からないのだ! Feb. 3, 2017と表示して欲しいのだ! それと、JuneとJulyは略さなくてよいのだ!
1. プロジェクトの、暦年第一週をてきとーに決めないで欲しいのだ! 例えばISO 8601(一週間は月曜始まり、暦年の第一週=第一木曜日を含む週)にして欲しいのだ!
1. ユニットテストに共通の初期化は、Ruby test/unitはsetupで、CppUnitはsetUpで、Google TestはSetUpなのだ! 大文字小文字に注意するのだ! overrideをつければコンパイラが間違いに気づいてくれるのだ!
1. ユニットテストのassert_equalに、期待値(expected)と実際の値(actual)のどちらを先に書くかは、テストを書く前に確認するのだ! Google TestとCppUnitは期待値が先だが、CUnitは後なのだ!
1. Google Testの結果を取り出せるのは、テストが完走したときだけなのだ! 途中でsegmentation faultで落ちたときは、出力が途中で切れているのだ! Exit statusを確認するのだ!
1. インラインアセンブリのシフト命令でCLレジスタを使ったら、破壊レジスタに書かないとダメなのだ! RCXレジスタは第一引数が入っているのだ! Segmentation faultの危機なのだ!
1. x64のRCXレジスタの上位32ビットをクリアするのに、ビットマスクを掛ける必要はないのだ! ECXレジスタへの書き込みで自動的にクリアされるのだ! ECXレジスタ自身にmovしてもよいのだ!
1. .sファイルのアセンブリマクロをテストできないと諦めないのだ! 単にマクロを展開して、C++のインラインアセンブリからcallすればテストできるのだ!
1. x86でNOP命令を「何クロックか待つ」ためには使えないのだ! Z80とは違うのだ! NOPは命令をアラインメントするために使うのだ!
1. アセンブリの細かい処理を高速化するときは、何マイクロ秒経ったかだけでなく、何クロック掛かったも測るのだ! 今どきのCPUは可変クロック周波数なのだ!
1. 短時間で終わる処理の時間測定に、外れ値を入れちゃ嫌なのだ! WindowsやLinuxはマルチタスクOSだから、測定対象以外のプロセスに時間を取られることがあるのだ!
1. メンバ関数にYieldという名前は使えないのだ! winbase.hでYieldを「何もしない」マクロと定義しているのだ! 理解不能なエラーメッセージの危機なのだ!
1. .cppから.dファイルを作るときに、自動生成する予定のファイルが見つからない、とエラーが出るのは困るのだ! -MGオプションが必要なのだ!
1. .hppからモックなどを作るときに、自動生成する予定のファイルがインクルードできないのは困るのだ! ```__has_include```はまだ早いのだ!
1. 単に速い処理をリアルタイムと呼んじゃ嫌なのだ! リアルタイムシステムとは、結果の正しさが内容だけでなく、締め切りに間に合ったかどうかにも依存するシステムなのだ! Stop the world GCはいつ終わるか分からないから困るのだ!
1. オブジェクト指向を「もの指向」と呼ぶのは嫌なのだ! 「目的語指向」と呼んで欲しいのだ!
1. ドキュメントがなくてもソースコード見れば分かる、ってそんな訳ないのだ! スレッドセーフとか再入不可とか計算量とか性能についてソースコードからは読み取れないのだ!
1. クイックソートの素朴な実装は、要素数Nに対して再帰呼び出しがO(N)回になってしまうのだ! スタックオーバフローの危機なのだ!
1. 質が低いドキュメントを書いて「分からないことがあったらなんでも訊いてね」にすると、プログラミングとユニットテストの最中にしょっちゅう割り込まれて作業が進まないのだ! プロジェクトの危機なのだ!
1. 再帰による探索だけでは計算量が多すぎて解が求まらないことがあるのだ! そこは動的計画法を使うのだ!
1. 非同期処理をシーケンス図で表現するのは大変なのだ! アクティビティ図を使って並行性を表現して欲しいのだ! ご飯を炊きながらカレーを煮込んで果物を刻むことを考えるのだ!
1. シーケンス図で複合フラグメントを駆使して、いろんな状態を一つの図にまとめるのはやめるのだ! 状態遷移図とアクションを使えばすっきり描けるかもしれないのだ! 設計中にいきなりif文とかswitch-caseとか、コードをイメージするのはよくないのだ!
1. クラス図のメンバにintとかstringとか書かないで欲しいのだ! ～というデータという抽象を保ってほしいのだ! intやstringで実装するのは後で決めればよいのだ! 実装時にtypedefを使えばよいのだ!
1. UML図の各要素の名前は、統一性のあるきちんとした英語でつけて欲しいのだ! 統一性のない用語や、間違った英単語を使うと、それがそのままコードになってしまうのだ! コーディングを始めてから語彙を統一するのは無理なのだ!
1. コレクションの先頭要素が0か1か区別して欲しいのだ! 0時はあっても、0日や0月や平成0年はないのだ!
1. 配列の要素を0から数えるのは、C言語が初めてじゃないのだ! イギリスやフランスでは、日本やアメリカの1階が0階(ground floor)なのだ! エレベーターに0とかGとか表示するのだ!
1. キーボードについている#はシャープじゃないのだ! Poundとかhashとかnumberとか読んで欲しいのだ! シャープはU+266Fという別の文字なのだ!
1. 関数名のtoを2と書いたり、forを4と書いたりするのはやめるのだ! コーディング規約は守ってほしいのだ! ソースコードをgrepする人のことも考えて欲しいのだ!
1. 引いて負なら0にするという飽和演算は、素直にsaturated sub(traction)と書いてほしいのだ! First grade subtraction(小学一年生の引き算)は正しいけど通じないのだ!
1. 変数をどこで使っているかgrepして目視するのは見落としが多いのだ! 変数名を変えてコンパイルして、エラーをあぶり出すのだ! でも変数名をお試しで変えたコードをcommitしちゃ嫌なのだ!
1. シンボル名やコメントを一括置換するときは、置換元を狭く限定して、過剰に置換していないかちゃんと確認するのだ! バをヴァに置き換えるだけだと、カヴァンになってしまうのだ!
1. シンボル名を短くしたいからって、変な略語を発明しないで欲しいのだ! Cではないのだから、シンボルを31文字以内に収める必要はないのだ!
1. 関数や型がどこで定義されているかを、いちいちgrepで調べるのは嫌なのだ! etagsでTAGSを作って、emacsからタグジャンプすればよいのだ!
1. 自由関数のシグネチャをgrepで集めると、複数行にわたる宣言と定義が途中で切れてしまうのだ! indentとctagsを使うのだ!
1. 複数のファイルを結合したとき、改行コードが行ごとにLFだったりCRLFだったり、まちまちなのは気持ち悪いのだ! Emacsで表示すると^Mが見えてしまうのだ!
1. インデントとか括弧の字下げとか一行は100文字以内とかいう規則を、プログラマに手作業で守らせるのは無理なのだ! どうしても守らせたければ、IDEやindentにさせるのだ!
1. 手作業で、段落を整形して一行の長さをそろえるのは大変なのだ! EmacsならM-q一発なのだ! (setq-default fill-column 70)で長さを変えられるのだ!
1. ソースコードのファイルの最後は改行で終わって欲しいのだ!
1. /usr/binのgccが古いと、新しいLLVMをビルドできないのだ! LLVM 4.0.0にはGCC4.8以降が必要なのだ!
1. /usr/binのツールが古かったり、/usr/libのライブラリが足りなかったりすると、新しいGCCをビルドできないのだ! 服を買いに行く服がないのは困るのだ!
1. LLVMをビルドするのに、もうconfigureは使えないのだ! cmakeの使い方を覚えるのだ!
1. 成果物のビルドはmake一発でできて欲しいのだ! それより複雑な手順は、間違った成果物をリリースする元凶なのだ! スモークテストも併せて実行して、結果を報告して欲しいのだ!
1. make cleanするときに、.dを作ってすぐ消すのは時間の無駄なのだ! ifneq ($(MAKECMDGOALS),clean) で囲って、.dを作らないようにするのだ! [参考](https://www.gnu.org/software/make/manual/html_node/Goals.html)
1. Makefileはデバッグ不能と諦めないのだ! ```make --dry-run```を実行するとか、Makefileに```$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))```を書いてmake showで表示するとかするのだ!
1. ビルド中のエラーメッセージを英語で出したいからといって、シェルで```export LC_ALL=C```とすると、svnで日本語ファイル名が扱えなくてエラーになるのだ! LC_ALLはMakefileの中で設定するのだ!
1. コレクションの最大要素と最小要素を別々に求めるのは二度手間なのだ! std::minmax_elementを使うのだ!
1. コレクションの最小要素を求める関数を下手に自作すると、要素が空の時INT_MAXが返ってしまうのだ! std::min_elementはイテレータを返すので、空なら第二引数(vector.end()など)が返るのだ!
1. abs(INT_MIN)は間違った値 INT_MIN を返すが、INT_MIN / -1 はintに収まらないので、CPUがDivide Error例外を出すのだ! プロセスの危機なのだ!
1. よく知られたアルゴリズムをそのまま使うときは出典を明記して欲しいのだ! そのコードがXorshiftだということは、保守する人には分からないのだ!
1. 素の乱数を作るのにXorshiftを引用するのはいいとしても、std::uniform_int_distributionを自作するのはやめるのだ! うまく作らないと結果に偏りが出てしまうが、偏っていることに簡単には気づかないのだ!
1. 区間演算を自作すると、両端を区間に含めるかどうかで不具合が起きやすいのだ! 素直にBoost.ICLを使うのだ! 計算量のオーダーも低くて済むのだ!
1. CRC計算を自作する必要はないのだ! Boost CRC Libraryがあるのだ!
1. RDTSC命令で取得したクロックをそのまま乱数に使うのはやめるのだ! 偶数しか出ないとか、4で割った余りがいつも同じとかいう偏りがあるのだ!
1. デバッガ上で実行していることはバレているのだ! わざとint3を踏んで、SEHでハンドリングしたかどうか確認しているのだ!
1. T時間当たり1回起きる不具合を、T時間起きなかったといって、直ったと断言するのはやめるのだ! T時間経っても起きない確率は{1/自然定数の底=37%}あるのだ!
1. トイレットペーパーが2ロールセットされているのを、両方少しずつ使っちゃ嫌なのだ! 両方同時に無くなったら危機なのだ! ダブルバッファリングを思い出すのだ!
1. Googleの看板"{first 10-digit prime found in consecutive digits of _e_}.com"を解くのに、エラトステネスの篩を自作するのはイケてないのだ! boost::multiprecision::miller_rabin_testを使うのだ!
1. "The dinosaur book"は恐竜のフレンズ図鑑じゃないのだ! "Operating System Concepts"の通称なのだ!
1. "The dragon book"はドラゴンのフレンズじゃないのだ! "Compilers: Principles, Techniques, and Tools"の通称なのだ!
1. テキストエディタのフォントはデフォルトではなく、{I,l,1,|}, {0,O}の区別がつきやすいフォントを使うのだ! NTEmacsなら、Consolas + Migu 1Mとか使うのだ!
1. diffコマンドは-uprNオプションをつけて起動して欲しいのだ! Working copyがbranchから一行でもずれたらパッチが当たらないのは困るのだ! git diffに形式をあわせるのだ!
1. 手元の開発作業とGitHubへのリリース作業の両方の画面を開くと、どれがどれだか分からなくなるのだ! VirtuaWinとか使って作業画面を分けるのだ!
1. デバッグで行き詰ったからといって、いきなり人を捕まえて、何が起きているか分からないと言わないで欲しいのだ! コノハ博士とミミちゃん助手のポストカードに話しかけて、状況を整理してから持ってきて欲しいのだ!
1. [ジョエルテスト](https://www.joelonsoftware.com/2000/08/09/the-joel-test-12-steps-to-better-code/)で11点取れないからって、テストしなかったことにするのはよくないのだ!

この語り口はあくまでネタなので、普段の私はもっと柔らかい口調で話しています、念のため。

## 動作確認

### フェネックとアライさんのやり取りを一通り実行する

```bash
make
```

を実行すると、一通りテストをビルドして実行します。最後はコンパイルエラーで終わりますが、これはコンパイルエラーを意図的に再現しているものです。

RDTSC命令の下の桁に偏りがある、という判定は実行環境によっては失敗するようです。何回か試して失敗するようでしたら、閾値を期待値に寄せるか、諦めてコメントアウトしてください。

当方の実行環境は以下の通りです。Google Test / Mockは$HOME直下にあると仮定していますので、それ以外の場合はMakefileを変更してください。AVX命令を使用することが前提ですので、サポートしていないプロセッサの場合は、Makefileの _CPPFLAGS_ARCH_ をコメントアウトしてください。

* Windows 10 Creators Update 64bit Edition
* Cygwin 64bit version (2.8.0)
* Google Test / Mock (1.7.0)
* Boost C++ Libraries (1.63.0)
* gcc (6.3.0)
* clang (4.0.1)
* Ruby (2.3.3p222)

### LTO(Link Time Optimization)

上記の通りmakeを実行すると、LTOを有効にした実行ファイルと、そうでないものを生成します。実行ファイルのシンボルテーブルを確認すると、UnusedFunctionの定義が以下の通りになります。

```bash
$ objdump -x cppFriends_gcc_lto | grep UnusedFunction
[780](sec -1)(fl 0x00)(ty   0)(scl   2) (nx 0) 0x0000000000000000 _Z14UnusedFunctionv

$ objdump -x cppFriends | grep UnusedFunction
[15586](sec  1)(fl 0x00)(ty  20)(scl   2) (nx 0) 0x000000000001d230 _Z14UnusedFunctionv

$ objdump -d cppFriends | less
000000010041e230 <_Z14UnusedFunctionv>:
   10041e230:   31 c0  xor    %eax,%eax
   10041e232:   c3     retq
```

### MinGWで何種類の空白文字を認識するか確認する

くいなちゃんさんによると、Unicodeの空白文字は17種類あるそうです。[ここに](https://twitter.com/kuina_ch/status/816977065480069121)あるものは、サーバで変換されて14種類になっているので、U+00A0を加えた15種類を空白文字として扱うかどうかを、[cppFriendsSpace.cpp](cppFriendsSpace.cpp)で調べます。

コマンドプロンプトから、

```bash
cppFriendsSpace.bat
```

を実行すると、ビルドして実行します。MinGWのインストール先はC:\MinGWに固定していますので、適宜cppFriendsSpace.batを変更してください。Boost C++ Librariesのファイル名が異なる場合(-mtなどがついている)場合も適宜変更してください。

当方の実行環境は以下の通りです。

* Windows 10 Creators Update 64bit Edition
* MinGW-w64 (Distro 14.1)
* gcc (6.3.0)
* Boost C++ Libraries (1.63.0)

MinGWでは、Boost.Regexの空白文字(\sと[:space:])は、15文字すべてを空白とみなしました。しかしCygwinでは12文字しか空白とみなしませんでした。

### ファイルにUS-ASCII以外の文字が含まれないことを確認する

fileコマンドを使うのが簡単です。ファイルの何行目にUS-ASCII以外の文字があるかを表示したければ、下記のようなRubyのワンライナーを書けばよいです。

```bash
$ ruby -ne '$_.ascii_only? ? 0 : (puts "#{$.} : #{$_}" ; abort)' LICENSE.txt ; echo $?
0
$ ruby -ne '$_.ascii_only? ? 0 : (puts "#{$.} : #{$_}" ; abort)' cppFriends.cpp ; echo $?
31 :     // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
1
```

### switch-caseは整数しか振り分けられない

[こちら](switchCase.md)に説明を書きました。Rubyのcase-whenは便利ですね。

### 複数行のマクロを安全に展開する

Compound statementがあることを教えて頂きました。[こちら](expandMacro.md)に説明を書きました。

### コンパイラのバージョンによって動作が異なる

[こちら](upgradeCompiler.md)にまとめました。

## ツイッターbotの投稿順序を並び替える

[こちら](shuffleLines.md)に説明があります。

## ライセンス

本レポジトリのライセンスは、[MITライセンス](LICENSE.txt)です。

ちなみに、けものフレンズ公式には[二次創作に関するガイドライン](http://kemono-friends.jp/)があります。私にはあいにく絵心がないです。

## C++の一般的な情報源

これらに記載されていることをすべて本ページに書くわけにもいきませんので、自分でC++のコードを書いていて、特に気になることだけを随時上記にまとめています。

* [私が読んだ書籍](https://github.com/zettsu-t/zettsu-t.github.io/wiki/Books)
* [Boost C++ Libraries](http://www.boost.org/)
* [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
* [More C++ Idioms](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms)
* [Intel 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm) and [Intel 64 and IA-32 Architectures Optimization Reference Manual](http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html)
* [Use reentrant functions for safer signal handling](https://www.ibm.com/developerworks/library/l-reent/index.html)
* http://stackoverflow.com/ などの各記事

「やめるのだフェネック! たとえ英語が嫌いでも、プログラマに英語は必要なのだ! 英語が読み書きできて、語彙が十分でないと、stackoverflow.com で解決策を調べられないのだ! 」

## おまけ

アニメ「けものフレンズ」の主題歌「ようこそジャパリパークへ」を歌っているのは"どうぶつビスケッツ×PPP"ですが、PPPという単語から思い出すのは、Point-to-Point Protocolでも購買力平価説でもなく、アニメ「アイドルマスターシンデレラガールズ」に出てくる架空のファッションブランドPikaPikaPopだったりします。PikaPikaPopをPPPと略しているシーンはアニメにないはずですが、それ言ったら「フレンズなんだね」も本編になかったはずですし...
