
次にやること

現状 coder.c / dongle.c / eye.c / utils.c は空です。依存関係の順に進めるのがおすすめ：

1. dongle.c：まず「dongle」という共有資源のデータ構造を定義する（pthread_mutex_t lock, pthread_cond_t cond, 使用中フラグ, cooldown用のタイムスタンプなど）。coderより先に、みんなが取り合う対象を固めておく。
2. coder.c：coderの構造体（番号、隣接する2つのdongleへのポインタ、状態、最後にcompileを始めた時刻など）と、pthread_create に渡すスレッド関数（void *coder_routine(void *arg)）。中で compile→debug→refactor のループを回す。
3. eye.c：monitorスレッド。全coderを定期的に見て回り、burnoutしていないかチェックし、burnoutを検出したらログを出してシミュレーションを止めるフラグを立てる。
4. utils.c：gettimeofday を使ったタイムスタンプ取得関数、ログ出力（mutexで保護されたprintf）などの共通処理。
5. main.c：perser() で得た値を使って dongle配列・coder配列を malloc、初期化、pthread_create で全スレッド起動、pthread_join で回収、free で解放。

依存の向きとしては utils → dongle → coder → eye → main が組みやすい順番です。

---

pthread、何が起きているのか

ここが一番大事なので詳しく説明します。

通常のプログラムは1本の道です。main が上から下に実行され、関数を呼んだら戻ってくるまで待ち、次の行へ進みます。

pthread_create を呼ぶと、これが崩れます。

pthread_create(&tid, NULL, coder_routine, &coder_data);
printf("next line\n");

この2行の実行順は保証されません。OSは新しいを実行する側）をいつ実際にCPUに乗せて動かすか自由に決めます。pthread_create を呼んだ瞬間に新しいスレッドがもう coder_routine の中身を実行し始めているかもしれないし、printf("next line\n")
が先に終わってからかもしれません。マルチコ くことすらあります。

重要なのは、新しいスレッドは元のスレッドと同じメモリ（ヒープ、malloc
した領域）を共有するという点です（スタック  ら、dongleの構造体は mallocでヒープに確保されていて、全coderスレッドが同じアドレスを指すポインタを持っています。つまり「coder 1のスレッド」と「coder 2のスレッド」が同時に同じdongle構造体を読み書きしうる、ということです。

なぜそれが問題か（具体例）

dongleが空いているかどうかを in_use という  もし何も保護せずにこう書くと：

if (dongle->in_use == 0)      // (A) チェック
{
    dongle->in_use = 1;       // (B) 確保
    ...
}

coder1のスレッドが (A) を実行して「空いてる」と判断した直後、CPUが coder2 のスレッドに切り替わり、coder2 も (A) を実行して「空いてる」と判断してしまう可能性があります。両方とも (B) を実行し、両方とも「自分がdongleを取った」と思い込む——これがレースコンディションです。実際には「両手にdongleを持たないとcompileできない」というルールを破ってしまいます。

mutex が解決すること

pthread_mutex_lock(&dongle->lock);
if (dongle->in_use == 0)
{
    dongle->in_use = 1;
    ...
}
pthread_mutex_unlock(&dongle->lock);

pthread_mutex_lock は「今このコードブロックに入れるのは自分だけ」という保証を作ります。もし他のスレッドが既に lock
を持っていたら、lock を呼んだスレッドはそこ 費せず、OSに「起こしてもらうまで眠る」形で待つ）。持っているスレッドが unlock した瞬間に、待っていたスレッドの誰か1つが lock を獲得して続きを実行します。これで (A)→(B) の間に誰も割り込めなくなります。

cond（条件変数）が解決すること

mutexだけだと「dongleが空くまで待つ」ができ 」とわかったら、unlock してまた lockして…と何度もループで確認する＝ビジーウェイトになり、CPUを無駄に食います）。そこで：

pthread_mutex_lock(&dongle->lock);
while (dongle->in_use)
    pthread_cond_wait(&dongle->cond, &dongle->lock);
dongle->in_use = 1;
pthread_mutex_unlock(&dongle->lock);

pthread_cond_wait は「lock を手放して眠る。誰かが signal/broadcast してくれたら、目を覚まして、もう一度自動的に lock を取り直してから戻ってくる」という動きをします。手放すのと眠るのがアトミック（不可分）に行われるのがポイントで、これがないと「手放した瞬間、寝る前」に他のスレッドが signal してしまい、通知を取りこぼす、という別のレースが起きます。

dongleを返す側はこう書きます：

pthread_mutex_lock(&dongle->lock);
dongle->in_use = 0;
pthread_cond_broadcast(&dongle->cond);
pthread_mutex_unlock(&dongle->lock);

broadcast で「待っている全員」を起こし、起きた各スレッドは while ループでもう一度 dongle->in_use
を確認します（起こされても他の誰かに先に取  ず再チェックが必要）。

まとめると

- pthread_create = 「並行に動く別の実行の流行順は不定。
- 共有メモリへのアクセスは、素朴に読み書きすると壊れる（レースコンディション）。
- mutex = 「一度に1人だけ」を保証する鍵。
- cond = 「条件が整うまで、CPUを使わずに眠  ず再チェック。
- pthread_join = 「そのスレッドが終わるまで待って、後片付けする」。呼ばないとリソースが残り続ける。

まずは dongle.c から、この mutex + cond の骨組みを一緒に書いてみますか？






構造体　＋＠
❯ ほかにおすすめの構造体はある？

はい、あと2つ、むしろ必須に近いものがあります。

1. 共有状態をまとめる t_env（環境／シミュレーション全体の状態）

今の t_coder  ればいけない情
強いて確認するとしたら、norminette（42 Norm チェッカー）を使える環境ならインデントやタブの形式だけ流しておくと安心です。機能的には今の定義で大丈夫です。       
✻ Baked for 24s                                            
❯ ていうかこれらは構造体になる必要があるの？               
いい質問です。結論から言うと、t_coder と t_dongle        は構造体でなけはそこまで厳密ではないですが、この課題の制約下ではやはり構造体にするのが正解です。理由をそれぞれ説明します。       
t_coder — 構造体が必須な理由：pthread_create の制約
                                                         int pthread_cr pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg);

pthread_create はスレッド関数に渡せる引数が void *arg      1個だけです。c 左右のdongleへ
                                                           int pthread_cr pthread_attr_t *attr,                                                                   void *), void*arg);                                                    
pthread_create はスレッド関数に渡せる引数が void *arg 1個だけです。coderのスレッド関数は自分のID、左右のdongleへ のポインタなど 引数を渡す仕組みは存在しません。なので「複数の値を1つのポインタにまとめる」＝構造体にする、以外の方法が事実上ありません（配列で渡すという手もなくはないですが、型が揃っていない（IDはint、d ongleはポイン 造体に行き着きます）。                                                  
さらに「グローバル変数は禁止」というルールがあるので、g_co ders[]のようなグローバル配列で共有することもできません。スレッド関数の中から自分のデータにアクセスするには、pthread_create の arg 経由で かありません。

t_dongle — 構造体が必須な理由：mutexはdongleごとに1つ必要

各dongleは独立して取り合われる資源なので、dongleごとに別々
の pthread_mutを持つ必要があります（1つの mutex
で全dongleを保 操作までブロックし合ってしまい、並行性が落ちます）。

これを構造体なしでやろうとすると、pthread_mutex_t locks[n]; pthread_cond_t conds[n]; int in_use[n];
のようなパラレ
- 同じインデックス i を常に対応づけて使う必要があり、ズレるとバグる
- coder側から 配列全体へのポインタ＋インデックスをセットで持ち回る必要があり、単一のポインタで完結しない

構造体にして t_dongle *left_dongle の形にすれば、1個のポインタで「そのdongleのmutex・cond・状
態」全部にアク も書けなくはないですが、バグを埋め込みやすく、この規模のプロジェクトでそれを選ぶ理由がありません。

t_params — こちらは「必須」ではなく「強く推奨」

これは前者2つ createの1引数制限）はありません。理論上は8個のローカル変数を main に置いて、必要な関数に8個ずつ引数で渡していくことも可能です。ただし：
- グローバル変数禁止なので、dongle や coder を初期化する関数、monitorスレッドなど、複数の場所でこれらの値が必要になります
- 8個バラバラの引数を関数呼び出しのたびに順番通り渡すのは、引数の順序を間違えるバグの温床です（time_to_debug と time_to_refactor を入れ替えて渡しても、コンパイラは気づいてくれません）
- 構造体1個・ポインタ1個渡すだけなら、順序ミスが起きようがありません

なので t_params は「なくても書けるが、書かない理由がない」パターンです。

まとめ

┌───────┬───────┬────────────────────────────────────┐
│ 構造  │ 必須  │                理由                │
│  体   │  度   │                                    │
├───────┼───────┼────────────────────────────────────┤
│ t_cod │ 必須oid *     │
│ er    │       │ 1個だけ                            │
├───────┼───────┼────────────────────────────────────┤
│ t_don │ ほぼ/condが要 │
│ gle   │ 必須  │ る                                 │
├───────┼───────┼────────────────────────────────────┤
│ t_par │ 推奨 安全に配 │
│ ams   │       │ りやすくするため                   │
└───────┴───────┴────────────────────────────────────┘


❯ ほかにおすすめ

はい、あと2つ、むしろ必須に近いものがあります。

1. 共有状態をまとめる t_env（環境／シミュレーション全体の状態）

今の t_coder には、他のスレッドと共有しなければいけない情報へのアクセス手段がありません。例えば：

- シミュレーション開始時刻（timestamp_in_ms を計算するのに全スレッドが必要）
- 「burnoutが起きた／規定回数コンパイルし終えた」を示す停止フラグ（monitorがこれを立て、全coderスレッドがこれを見てループを抜ける）
- ログ出力用の mutex（前に話した「1行が混ざらないように」を実現する）
- 全coder・全dongleの配列（monitorが全coderを巡回チェックするために必要）
- t_params（グローバル変数禁止なので、time_to_burnout や dongle_cooldown にどのスレッドからもアクセスできるようにする経路が必要）

これをまとめて t_env のような構造体にし、t_coder に t_env *env; というポインタを1つ持たせれば、coderスレッドはそこか
ら何でも参照で

typedef struct s_env
{
      t_params                params;
      long                    start_time_ms;
      int                 //達成で1になる
      pthread_mutex_t stop_lock;
      pthread_
      t_coder                 *coders;
      t_dongle                *dongles;
}     t_env;

これがないと、「monitorスレッドがどうやって全coderの状態を見るのか」「coderがどうやって time_to_burnout の値を知るのか」が解決できません。前回の「t_params をどう配るか」の話（A案）とも直結していて、t_coder に t_env *env を足せば coder->env->params.time_to_burnout のようにアクセスできます。

2. FIFO/EDFスケジューリング用の優先度キュー（ヒープ）構造体

これは仕様書（Mandatory part, p.12）に明記されている必須要件で、まだ手をつけていない部分です：

▎ You must implement a priority queue (heap) for FIFO/EDF scheduling (no standard library priority queue may be used).

複数のcoderが同じdongleを待っているとき、fifo（到着順）か edf（last_compile_start + time_to_burnout が一番早い者勝ち）で次に誰に渡すかを決める必要があります。これを標準ライブラリなしで自前実装しなければいけません。

typedef struct s_request
{
      t_coder *coder;
      long    arrival_ms;    // FIFO用：待ち行列に入った時
      long    _compile_starttime_to_burnout
}     t_request;

typedef struct s_heap
{
      t_request       **items;
      int                     size;
      int                     capacity;
}     t_heap;

dongle はこのままだと「今誰が使っているか」しかわかりませんが、この t_heap を各 t_dongle に持たせれば「今このdongleを待っている人たち」を優先順位付きで管理できます（このプロジェクトの構造上、1つのdongleを取り合うのは隣接する2人のcoderだけなので、ヒープのサイズは最大2程度と小さいですが、「ヒープを実装する」こと自体が要
件なので、サイできる汎用的な実装にしておくのが安全です）。

3. t_coder に

number_of_compiles_required の判定（「全員が規定回数コンパ
イルしたら終了何回コンパイルしたかを覚えておく必要があります。今の t_coder にはこれがありません：

unsigned int  compiles_done;

---

まとめると、優先度としては


1. t_env（他の
2. t_coder に compiles_done と t_env *env を追加
3. t_heap / t_request（FIFO/EDFの実装に着手するとき）

の順で進めるのがいいと思います。t_env から一緒に書いてみますか？それとも自分でまず書いてみますか？



