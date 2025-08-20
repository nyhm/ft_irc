## 本家の挙動

了解。公式系 IRCd（例: InspIRCd / UnrealIRCd）で“本家の挙動”を手元で再現・確認するための最小セットを示す。各テストは **コマンド** → **期待挙動** の順。

---

# 0) 公式IRCdをローカルで起動（最速）

## A. InspIRCd（Docker）

```bash
docker run --name inspircd -d -p 6667:6667 inspircd/inspircd-docker
```

デフォルト設定で 6667/TCP のクライアント受付が入っている。必要なら設定は `inspircd.conf` の `<bind ... type="clients">` を編集。([hub.docker.com][1], [docs.inspircd.org][2])

> 参考：InspIRCd は接続クラスでサーバ接続パスワード（`PASS`）を要求できる（`<connect ... password="...">` 等）。([docs.inspircd.org][3])

## B. UnrealIRCd（設定が明快）

1. インストール/コンテナは任意。
2. 設定例（`unrealircd.conf` 抜粋）:

```conf
listen { ip *; port 6667; }     // 平文 6667 を開ける
allow {
    mask *;
    password "pass";           // サーバ接続パスワード（PASS）
    class clients;
}
```

`allow::password` を設定すると、クライアントは `PASS pass` を要求される。([unrealircd.org][4], [GitHub][5])

---

# 1) 受付・登録フロー（CRLF 前提）

> 以降は **CRLF を確実に送るため**に `printf` で明示して送る（環境依存の `nc -C` は使わない）。IRCは**行末CRLF**・**1行512バイト（CRLF含む）上限**が基本仕様。([rfc-editor.org][6], [tech-invite.com][7])

### 1-1 認証成功（UnrealIRCd で PASS を要求する例）

```bash
# 端末A
{ printf 'PASS pass\r\nNICK alice\r\nUSER alice 0 * :Alice\r\n'; sleep 1; } | nc 127.0.0.1 6667
```

**期待**: 数値 `001 RPL_WELCOME` などの登録完了応答が返る。([defs.ircdocs.horse][8], [GitHub][9])

### 1-2 パスワード不一致

```bash
{ printf 'PASS wrong\r\nNICK bad\r\nUSER bad 0 * :Bad\r\n'; sleep 1; } | nc 127.0.0.1 6667
```

**期待**: 接続拒否。UnrealIRCd では不一致メッセージをカスタムできる（`set::reject-message::password-mismatch`）。([unrealircd.org][10])

### 1-3 ニック競合

```bash
# 端末B（別セッション）
{ printf 'PASS pass\r\nNICK alice\r\nUSER bob 0 * :Bob\r\n'; sleep 1; } | nc 127.0.0.1 6667
```

**期待**: `433 ERR_NICKNAMEINUSE`。([defs.ircdocs.horse][8])

### 1-4 PING/PONG

```bash
printf 'PING :123\r\n' | nc 127.0.0.1 6667
```

**期待**: `PONG :123`。([docs.inspircd.org][11])

---

# 2) チャンネル基本挙動

### 2-1 JOIN → NAMES → NOTOPIC

```bash
# 端末A（登録済み）
printf 'JOIN #test\r\n' | nc 127.0.0.1 6667
```

**期待**: `JOIN` 通知、自分宛に `353 RPL_NAMREPLY` と `366 RPL_ENDOFNAMES`、トピック未設定なら `331 RPL_NOTOPIC`。([defs.ircdocs.horse][8])

### 2-2 ブロードキャスト

```bash
# 端末B
{ printf 'PASS pass\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #test\r\nPRIVMSG #test :hello\r\n'; sleep 1; } | nc 127.0.0.1 6667
```

**期待**: 端末Aで `:bob PRIVMSG #test :hello` を受信（チャンネル参加者全員へ配信）。([chi.cs.uchicago.edu][12])

### 2-3 PART

```bash
printf 'PART #test :bye\r\n' | nc 127.0.0.1 6667
```

**期待**: `PART` がチャンネルに通知。([defs.ircdocs.horse][8])

---

# 3) 権限・モード（+i/+t/+k/+o/+l）

> 以降、**A がオペ**になるように最初に #test を作るか、MODEで付与。

### 3-1 非オペの拒否

```bash
# 端末B
printf 'MODE #test +t\r\n' | nc 127.0.0.1 6667
```

**期待**: `482 ERR_CHANOPRIVSNEEDED`。([defs.ircdocs.horse][8])

### 3-2 オペ付与/剥奪

```bash
# 端末A
printf 'MODE #test +o bob\r\n' | nc 127.0.0.1 6667
printf 'MODE #test -o bob\r\n' | nc 127.0.0.1 6667
```

**期待**: 成功時 MODE 変更が通知。([defs.ircdocs.horse][8])

### 3-3 招待制 +i と INVITE

```bash
# 端末A
printf 'MODE #test +i\r\n' | nc 127.0.0.1 6667

# 端末C（新規）
{ printf 'PASS pass\r\nNICK carol\r\nUSER carol 0 * :Carol\r\nJOIN #test\r\n'; sleep 1; } | nc 127.0.0.1 6667
```

**期待**: C は `473 ERR_INVITEONLYCHAN`。その後:

```bash
# 端末A
printf 'INVITE carol #test\r\n' | nc 127.0.0.1 6667
# 端末C
printf 'JOIN #test\r\n' | nc 127.0.0.1 6667
```

**期待**: 参加成功。([defs.ircdocs.horse][8])

### 3-4 TOPIC と +t

```bash
# 端末A
printf 'MODE #test +t\r\nTOPIC #test :Welcome\r\n' | nc 127.0.0.1 6667
# 端末B
printf 'TOPIC #test :nope\r\n' | nc 127.0.0.1 6667
```

**期待**: Aの変更は成功、Bは `482` で拒否。([defs.ircdocs.horse][8])

### 3-5 キー +k と入室検証

```bash
# 端末A
printf 'MODE #test +k secret\r\n' | nc 127.0.0.1 6667
# 端末B（いったん抜けて）
printf 'PART #test\r\nJOIN #test\r\n' | nc 127.0.0.1 6667
```

**期待**: `475 ERR_BADCHANNELKEY`。`JOIN #test secret` なら成功。([defs.ircdocs.horse][8])

### 3-6 上限 +l

```bash
# 端末A
printf 'MODE #test +l 2\r\n' | nc 127.0.0.1 6667
# 新規Dが JOIN
printf 'JOIN #test\r\n' | nc 127.0.0.1 6667
```

**期待**: `471 ERR_CHANNELISFULL`。([defs.ircdocs.horse][8])

### 3-7 KICK

```bash
# 端末A
printf 'KICK #test bob :rule\r\n' | nc 127.0.0.1 6667
```

**期待**: B が即時退出、KICK 通知。([defs.ircdocs.horse][8])

---

# 4) 断片化・CRLF・行長上限

### 4-1 断片化（部分送信でも1コマンドにまとまるか）

```bash
# 端末X（逐次送信）
( printf 'PRIV'; sleep 1; printf 'MSG #test :hi'; sleep 1; printf '\r\n' ) | nc 127.0.0.1 6667
```

**期待**: サーバ側は受信バッファで **CRLF 到達時にのみ** 1コマンドとして処理（途中では未処理）。([tech-invite.com][7])

### 4-2 行末CRLF厳密性

```bash
# わざと LF のみを送る
{ printf 'NICK n\nUSER u 0 * :U\n'; sleep 1; } | nc 127.0.0.1 6667
```

**期待**: 多くの実装は後方互換で受理することがあるが、**正規の送信は CRLF**。挙動は実装依存（Modern specの互換推奨）。([modern.ircdocs.horse][13])

### 4-3 512バイト上限の確認

```bash
# 510バイトペイロード（+ "\r\n" で 512）
python3 - <<'PY' | nc 127.0.0.1 6667
print("PRIVMSG #test :" + "a"*510 + "\r\n", end="")
PY

# 513バイト超（明確に超過）
python3 - <<'PY' | nc 127.0.0.1 6667
print("PRIVMSG #test :" + "a"*511 + "\r\n", end="")
PY
```

**期待**: 仕様は**512バイト（CRLF含む）まで**。超過時の扱いは実装差（エラー `417 ERR_INPUTTOOLONG` を返す、切り捨て、無視/切断など）だが、**ハングやクラッシュはしない**。([rfc-editor.org][6], [modern.ircdocs.horse][13])

---

# 5) 直接メッセージとエラー

### 5-1 PRIVMSG ニック宛

```bash
printf 'PRIVMSG bob :hi\r\n' | nc 127.0.0.1 6667
```

**期待**: bob に `:alice PRIVMSG bob :hi`。([chi.cs.uchicago.edu][12])

### 5-2 宛先不在

```bash
printf 'PRIVMSG nothere :hi\r\n' | nc 127.0.0.1 6667
```

**期待**: `401 ERR_NOSUCHNICK`。([defs.ircdocs.horse][8])

### 5-3 引数不足

```bash
printf 'JOIN\r\n' | nc 127.0.0.1 6667
```

**期待**: `461 ERR_NEEDMOREPARAMS JOIN`。([defs.ircdocs.horse][8])

---

## 備考

* 数値リプライの**コード**は共通だが、**メッセージ文言**はIRCd実装で異なる。照合は**数値**で行う。([defs.ircdocs.horse][8])
* `PASS` の意味は「サーバ接続パスワード」。InspIRCd/UnrealIRCdとも機能があり、設定で必須化できる。([docs.inspircd.org][11], [unrealircd.org][10])

このセットを実行すれば、**本家IRCdの CRLF 処理／断片化／512バイト上限／数値リプライ／オペ権限**などの挙動を実機で確認できる。

[1]: https://hub.docker.com/r/inspircd/inspircd-docker?utm_source=chatgpt.com "Docker Image - inspircd"
[2]: https://docs.inspircd.org/3/configuration/?utm_source=chatgpt.com "v3 Configuration"
[3]: https://docs.inspircd.org/4/configuration/?utm_source=chatgpt.com "v4 Configuration"
[4]: https://www.unrealircd.org/docs/Configuration?utm_source=chatgpt.com "Configuration - UnrealIRCd documentation wiki"
[5]: https://github.com/unrealircd/unrealircd/blob/unreal60_dev/doc/conf/examples/example.conf?utm_source=chatgpt.com "unrealircd/doc/conf/examples/example.conf at unreal60_dev"
[6]: https://www.rfc-editor.org/rfc/rfc1459.html?utm_source=chatgpt.com "RFC 1459: Internet Relay Chat Protocol"
[7]: https://www.tech-invite.com/y25/tinv-ietf-rfc-2812.html?utm_source=chatgpt.com "RFC 2812 - Client Protocol"
[8]: https://defs.ircdocs.horse/defs/numerics?utm_source=chatgpt.com "Numerics - IRC Definition Files"
[9]: https://github.com/ircdocs/irc-defs/blob/gh-pages/_data/numerics.yaml?utm_source=chatgpt.com "irc-defs/_data/numerics.yaml at gh-pages"
[10]: https://www.unrealircd.org/docs/Set_block?utm_source=chatgpt.com "Set block - UnrealIRCd documentation wiki"
[11]: https://docs.inspircd.org/4/commands/?utm_source=chatgpt.com "v4 Commands"
[12]: https://chi.cs.uchicago.edu/chirc/irc_examples.html?utm_source=chatgpt.com "Example IRC Communications - The UChicago χ-Projects"
[13]: https://modern.ircdocs.horse/?utm_source=chatgpt.com "IRC Client Protocol Specification"
