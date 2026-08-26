
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