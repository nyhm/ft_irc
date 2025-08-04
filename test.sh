#!/bin/bash

# IRCサーバーの接続先とポート
HOST=127.0.0.1
PORT=6667

# 各クライアントごとに FIFO を作成して双方向通信をシミュレート
mkfifo in1 out1
mkfifo in2 out2

# クライアント1（hiroki）
nc $HOST $PORT < in1 > out1 &
PID1=$!

# クライアント2（hanako）
nc $HOST $PORT < in2 > out2 &
PID2=$!

# 少し待って接続が安定するのを待つ
sleep 1

# NICK, USER, JOIN, PRIVMSG をクライアント1に送る
{
  echo "NICK hiroki"
  echo "USER hiroki 0 * :Hiroki"
  echo "JOIN #42tokyo"
  echo "PRIVMSG #42tokyo :こんにちは、みんな！"
  sleep 2
  echo "QUIT"
} > in1 &

# クライアント2に NICK, USER, JOIN を送る
{
  echo "NICK hanako"
  echo "USER hanako 0 * :Hanako"
  echo "JOIN #42tokyo"
  sleep 5
  echo "QUIT"
} > in2 &

# 出力ログを保存
cat out1 > client1.log &
cat out2 > client2.log &

# 待機
sleep 6

# 終了処理
kill $PID1 2>/dev/null
kill $PID2 2>/dev/null
rm -f in1 out1 in2 out2

# 結果表示
echo "=== クライアント1 (hiroki) のログ ==="
cat client1.log
echo ""
echo "=== クライアント2 (hanako) のログ ==="
cat client2.log
