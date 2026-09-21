#!/bin/bash
# ============================================================================
#  codexion test script
#
#  usage:  ./test_codexion.sh            全部
#          ./test_codexion.sh args       引数チェックのみ
#          ./test_codexion.sh solo       1人テストのみ
#          ./test_codexion.sh alive      死なないケース
#          ./test_codexion.sh death      死ぬケース
#          ./test_codexion.sh finish     正常終了
#          ./test_codexion.sh cooldown   cooldown の効果
#          ./test_codexion.sh sched      fifo vs edf
#          ./test_codexion.sh order      タイムスタンプ単調性
#          ./test_codexion.sh san        sanitizer ビルド
# ============================================================================

BIN=./codexion
SRC="src/*.c"
INC="includes"

# ---- 出力フォーマットに合わせてここを書き換える ----------------------------
PAT_COMPILE="compil"     # compile 開始行にマッチする文字列
PAT_DEATH="burn"         # burnout 行にマッチする文字列
TS_FIELD=1               # タイムスタンプが何番目のフィールドか
ID_FIELD=2               # coder id が何番目のフィールドか
# ---------------------------------------------------------------------------

G="\033[32m"; R="\033[31m"; Y="\033[33m"; C="\033[36m"; N="\033[0m"
pass=0; fail=0

ok()   { echo -e "  ${G}PASS${N}  $1"; pass=$((pass+1)); }
ng()   { echo -e "  ${R}FAIL${N}  $1"; fail=$((fail+1)); }
warn() { echo -e "  ${Y}????${N}  $1"; }
head_() { echo -e "\n${C}=== $1 ===${N}"; }

need_bin() {
	if [ ! -x "$BIN" ]; then
		echo -e "${R}$BIN が見つかりません。make してください。${N}"
		exit 1
	fi
}

# ============================================================================
#  1. 引数チェック — 全部エラー終了すべき
# ============================================================================
test_args() {
	head_ "引数チェック（全てエラー終了すべき）"
	local cases=(
		""
		"5"
		"5 800 200 200 200 7 50"
		"5 800 200 200 200 7 50 fifo extra"
		"0 800 200 200 200 7 50 fifo"
		"-5 800 200 200 200 7 50 fifo"
		"5 -800 200 200 200 7 50 fifo"
		"5 800 abc 200 200 7 50 fifo"
		"5 800 200 200 200 7 50 lifo"
		"5 800 200 200 200 7 50 FIFO"
		"5 800 200 200 200 7 50 "
		"99999999999 800 200 200 200 7 50 fifo"
		"5 800 200 200 200 7 -50 fifo"
	)
	for c in "${cases[@]}"; do
		out=$(timeout 3 $BIN $c 2>&1)
		code=$?
		label="[$c]"
		if [ $code -eq 139 ]; then
			ng "$label segfault"
		elif [ $code -eq 124 ]; then
			ng "$label ハング（エラー終了すべき）"
		elif [ $code -eq 0 ]; then
			ng "$label 正常終了した（エラーにすべき）"
		elif [ -z "$out" ]; then
			warn "$label エラー終了したがメッセージなし"
		else
			ok "$label"
		fi
	done

	head_ "有効な入力（エラーにしてはいけない）"
	local valid=(
		"5 800 200 200 200 7 0 fifo"
		"5 800 200 200 200 1 50 edf"
		"1 800 200 200 200 7 50 fifo"
	)
	for c in "${valid[@]}"; do
		out=$(timeout 3 $BIN $c 2>&1)
		code=$?
		if [ $code -eq 139 ]; then
			ng "[$c] segfault"
		elif echo "$out" | grep -qi "error\|usage\|invalid"; then
			ng "[$c] エラー扱いされた"
		else
			ok "[$c] 受理された"
		fi
	done
}

# ============================================================================
#  2. 1人 — 二重ロック検出
# ============================================================================
test_solo() {
	head_ "1人テスト（burnout すべき / ハングしてはいけない）"
	out=$(timeout 5 $BIN 1 800 200 200 200 7 0 fifo 2>&1)
	code=$?
	if [ $code -eq 124 ]; then
		ng "ハングした → left_hand == right_hand の二重ロックの可能性"
	elif [ $code -eq 139 ]; then
		ng "segfault"
	elif echo "$out" | grep -qi "$PAT_DEATH"; then
		last=$(echo "$out" | grep -i "$PAT_DEATH" | tail -1 | awk "{print \$$TS_FIELD}")
		ok "burnout した (t=$last, 期待値 ~800)"
		echo "$out" | tail -3 | sed 's/^/        /'
	else
		ng "burnout しなかった (exit=$code)"
		echo "$out" | tail -3 | sed 's/^/        /'
	fi

	# cooldown 付きでも同じ
	out=$(timeout 5 $BIN 1 800 200 200 200 7 100 edf 2>&1)
	[ $? -eq 124 ] && ng "1人 + cooldown100 + edf でハング" || ok "1人 + cooldown100 + edf"
}

# ============================================================================
#  3. 死なないケース
# ============================================================================
test_alive() {
	head_ "誰も burnout しないべきケース（10秒回す）"
	local cases=(
		"5 800 200 200 200 0 0 fifo"
		"5 800 200 200 200 0 0 edf"
		"4 410 200 200 200 0 0 fifo"
		"100 800 200 200 200 0 0 fifo"
	)
	for c in "${cases[@]}"; do
		out=$(timeout 10 $BIN $c 2>&1)
		code=$?
		if [ $code -eq 139 ]; then
			ng "[$c] segfault"
		elif echo "$out" | grep -qi "$PAT_DEATH"; then
			t=$(echo "$out" | grep -i "$PAT_DEATH" | head -1 | awk "{print \$$TS_FIELD}")
			ng "[$c] burnout した (t=$t)"
		elif [ $code -eq 124 ]; then
			n=$(echo "$out" | grep -ci "$PAT_COMPILE")
			ok "[$c] 10秒生存 (compile ${n}回)"
		else
			warn "[$c] 早期終了 exit=$code"
		fi
	done
}

# ============================================================================
#  4. 死ぬケース — 1人だけ死んで即停止
# ============================================================================
test_death() {
	head_ "burnout すべきケース（1人だけ死んで即停止）"
	local cases=(
		"4 310 200 100 100 100 0 fifo"
		"2 200 300 100 100 100 0 fifo"
		"5 600 650 150 150 100 0 edf"
	)
	for c in "${cases[@]}"; do
		out=$(timeout 10 $BIN $c 2>&1)
		code=$?
		if [ $code -eq 139 ]; then
			ng "[$c] segfault"; continue
		fi
		if [ $code -eq 124 ]; then
			ng "[$c] ハング（死後に終了できていない）"; continue
		fi
		ndeath=$(echo "$out" | grep -ci "$PAT_DEATH")
		if [ "$ndeath" -eq 0 ]; then
			ng "[$c] burnout しなかった"
		elif [ "$ndeath" -gt 1 ]; then
			ng "[$c] burnout が ${ndeath}回出力された（1回だけのはず）"
		else
			after=$(echo "$out" | grep -i -A100 "$PAT_DEATH" | tail -n +2 | grep -c .)
			if [ "$after" -gt 0 ]; then
				ng "[$c] 死亡行の後に ${after}行の出力がある"
				echo "$out" | grep -i -A5 "$PAT_DEATH" | sed 's/^/        /'
			else
				ok "[$c] 1人だけ死んで即停止"
			fi
		fi
	done
}

# ============================================================================
#  5. 正常終了 — 全員が required 回に到達
# ============================================================================
test_finish() {
	head_ "正常終了（required 回に達したら全員生存で終了）"
	local cases=(
		"5 800 200 200 200 3 0 fifo:3 *trickey"
		"5 800 200 200 200 1 0 edf:1"
		"2 800 200 200 200 5 0 fifo:5"
		"200 800 200 200 200 2 0 fifo:2"
	)
	for entry in "${cases[@]}"; do
		c="${entry%:*}"; req="${entry##*:}"
		out=$(timeout 20 $BIN $c 2>&1)
		code=$?
		if [ $code -eq 124 ]; then
			ng "[$c] 終了しない（join できていない）"; continue
		fi
		if [ $code -eq 139 ]; then
			ng "[$c] segfault"; continue
		fi
		if echo "$out" | grep -qi "$PAT_DEATH"; then
			ng "[$c] burnout した（全員生存で終わるべき）"; continue
		fi
		# 各コーダーの compile 回数が req 以上か
		short=$(echo "$out" | grep -i "$PAT_COMPILE" \
			| awk "{print \$$ID_FIELD}" | sort | uniq -c \
			| awk -v r="$req" '$1 < r {print $2}')
		if [ -n "$short" ]; then
			ng "[$c] 回数不足のコーダー: $(echo $short | tr '\n' ' ')"
		else
			ok "[$c] 全員が ${req}回以上 compile して終了"
		fi
	done
}

# ============================================================================
#  6. cooldown の効果
# ============================================================================
test_cooldown() {
	head_ "cooldown の効果（大きいほど compile 回数が減るべき）"
	prev=-1
	for cd in 0 50 100 200; do
		out=$(timeout 5 $BIN 5 100000 200 200 200 0 $cd fifo 2>&1)
		n=$(echo "$out" | grep -ci "$PAT_COMPILE")
		echo "        cooldown=${cd}ms  →  compile ${n}回"
		if [ $prev -ge 0 ] && [ "$n" -gt "$prev" ]; then
			warn "cooldown を上げたのに回数が増えた"
		fi
		prev=$n
	done
	if [ $prev -eq 0 ]; then
		ng "cooldown=200 で1度も compile していない（大きすぎ？）"
	else
		ok "cooldown の傾向を確認（上の数値が単調減少していれば正常）"
	fi

	head_ "cooldown が大きすぎると burnout するべき"
	out=$(timeout 8 $BIN 5 500 200 200 200 0 600 fifo 2>&1)
	if echo "$out" | grep -qi "$PAT_DEATH"; then
		ok "cooldown=600 / burnout=500 で死亡（cooldown が効いている）"
	else
		ng "cooldown=600 なのに誰も死なない → released_time が見られていない可能性"
	fi
}

# ============================================================================
#  7. fifo vs edf
# ============================================================================
test_sched() {
	head_ "scheduler の比較"
	a=$(timeout 5 $BIN 5 100000 200 200 200 0 50 fifo 2>&1 | head -60)
	b=$(timeout 5 $BIN 5 100000 200 200 200 0 50 edf  2>&1 | head -60)
	if [ "$a" = "$b" ]; then
		warn "fifo と edf の出力が完全一致（偶然の可能性もあるが要確認）"
	else
		ok "fifo と edf で出力が異なる"
	fi

	echo -e "\n        ${C}--- 飢餓耐性テスト ---${N}"
	echo "        edf は最も deadline が近い人を優先するので、"
	echo "        ギリギリの設定では fifo より生存しやすいはず"
	for s in fifo edf; do
		died=0
		for i in 1 2 3; do
			out=$(timeout 6 $BIN 5 410 200 100 100 0 0 $s 2>&1)
			echo "$out" | grep -qi "$PAT_DEATH" && died=$((died+1))
		done
		echo "        $s : 3回中 ${died}回 burnout"
	done
	echo "        （edf の方が少なければ edf が効いている証拠）"
}

# ============================================================================
#  8. タイムスタンプ単調性
# ============================================================================
test_order() {
	head_ "タイムスタンプの単調性"
	for c in "5 800 200 200 200 5 50 fifo" "5 800 200 200 200 5 50 edf" \
	         "10 800 200 200 200 3 0 edf"; do
		out=$(timeout 15 $BIN $c 2>&1)
		bad=$(echo "$out" | awk -v f=$TS_FIELD '
			{ v = $f + 0
			  if (NR > 1 && v < prev) print NR": "$0
			  prev = v }')
		if [ -n "$bad" ]; then
			ng "[$c] 順序が逆転している行あり"
			echo "$bad" | head -3 | sed 's/^/        /'
		else
			ok "[$c] タイムスタンプは単調増加"
		fi
	done
}

# ============================================================================
#  9. sanitizer
# ============================================================================
test_san() {
	head_ "ThreadSanitizer（データ競合検出）"
	cc -Wall -Wextra -pthread -fsanitize=thread -g -I $INC $SRC -o /tmp/cx_tsan 2>/dev/null
	if [ $? -ne 0 ]; then
		warn "tsan ビルド失敗（環境が対応していない可能性）"
	else
		out=$(timeout 20 /tmp/cx_tsan 5 800 200 200 200 3 50 edf 2>&1)
		if echo "$out" | grep -q "WARNING: ThreadSanitizer"; then
			ng "データ競合を検出"
			echo "$out" | grep -A12 "WARNING: ThreadSanitizer" | head -20 | sed 's/^/        /'
		else
			ok "データ競合なし"
		fi
	fi

	head_ "AddressSanitizer（範囲外・リーク検出）"
	cc -Wall -Wextra -pthread -fsanitize=address -g -I $INC $SRC -o /tmp/cx_asan 2>/dev/null
	if [ $? -ne 0 ]; then
		warn "asan ビルド失敗"
	else
		for c in "5 800 200 200 200 3 50 fifo" "1 800 200 200 200 3 0 fifo" \
		         "200 800 200 200 200 1 0 edf"; do
			out=$(timeout 25 /tmp/cx_asan $c 2>&1)
			if echo "$out" | grep -q "ERROR: AddressSanitizer\|LeakSanitizer"; then
				ng "[$c] asan エラー"
				echo "$out" | grep -A8 "ERROR:\|SUMMARY:" | head -14 | sed 's/^/        /'
			else
				ok "[$c] asan クリーン"
			fi
		done
	fi
}

# ============================================================================
#  main
# ============================================================================
need_bin
case "${1:-all}" in
	args)     test_args ;;
	solo)     test_solo ;;
	alive)    test_alive ;;
	death)    test_death ;;
	finish)   test_finish ;;
	cooldown) test_cooldown ;;
	sched)    test_sched ;;
	order)    test_order ;;
	san)      test_san ;;
	all)
		test_args; test_solo; test_alive; test_death
		test_finish; test_cooldown; test_sched; test_order; test_san ;;
	*)
		echo "usage: $0 [args|solo|alive|death|finish|cooldown|sched|order|san|all]"
		exit 1 ;;
esac

echo -e "\n${C}========================================${N}"
echo -e "  ${G}PASS: $pass${N}   ${R}FAIL: $fail${N}"
echo -e "${C}========================================${N}"
[ $fail -eq 0 ]
