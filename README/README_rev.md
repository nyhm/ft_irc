SCALE FOR PROJECT FT_IRC
You should evaluate 2 students in this team

Introduction
Please comply with the following rules:

- Remain polite, courteous, respectful and constructive throughout the
evaluation process. The well-being of the community depends on it.

- Identify with the student or group whose work is evaluated the possible
dysfunctions in their project. Take the time to discuss and debate the
problems that may have been identified.

- You must consider that there might be some differences in how your peers
might have understood the project's instructions and the scope of its
functionalities. Always keep an open mind and grade them as honestly as
possible. The pedagogy is useful only and only if the peer-evaluation is
done seriously.

Guidelines
- Only grade the work that was turned in the Git repository of the evaluated
student or group.

- Double-check that the Git repository belongs to the student(s). Ensure that
the project is the one expected. Also, check that 'git clone' is used in an
empty folder.

- Check carefully that no malicious aliases was used to fool you and make you
evaluate something that is not the content of the official repository.

- To avoid any surprises and if applicable, review together any scripts used
to facilitate the grading (scripts for testing or automation).

- If you have not completed the assignment you are going to evaluate, you have
to read the entire subject prior to starting the evaluation process.

- Use the available flags to report an empty repository, a non-functioning
program, a Norm error, cheating, and so forth.
In these cases, the evaluation process ends and the final grade is 0,
or -42 in case of cheating. However, except for cheating, student are
strongly encouraged to review together the work that was turned in, in order
to identify any mistakes that shouldn't be repeated in the future.

- Remember that for the duration of the defence, no segfault, no other
unexpected, premature, uncontrolled or unexpected termination of the
program, else the final grade is 0. Use the appropriate flag.
You should never have to edit any file except the configuration file if it
exists. If you want to edit a file, take the time to explicit the reasons
with the evaluated student and make sure both of you are okay with this.

- You must also verify the absence of memory leaks. Any memory allocated on
the heap must be properly freed before the end of execution.
You are allowed to use any of the different tools available on the computer,
such as leaks, valgrind, or e_fence. In case of memory leaks, tick the
appropriate flag.

Attachments
 subject.pdf   bircd.tar.gz
Mandatory Part
Basic checks

There is a Makefile, the project compiles correctly with the required options, is written in C++, and the executable is called as expected.
Ask and check how many poll() (or equivalent) are present in the code. There must be only one.
Verify that the poll() (or equivalent) is called every time before each accept, read/recv, write/send. After these calls, errno should not be used to trigger specific action (e.g. like reading again after errno == EAGAIN).
Verify that each call to fcntl() is done as follows: fcntl(fd, F_SETFL, O_NONBLOCK); Any other use of fcntl() is forbidden.
If any of these points is wrong, the evaluation ends now and the final mark is 0.
 Yes
 No
Networking

Check the following requirements:

The server starts, and listens on all network interfaces on the port given from the command line.
Using the 'nc' tool, you can connect to the server, send commands, and the server answers you back.
Ask the team what is their reference IRC client.
Using this IRC client, you can connect to the server.
The server can handle multiple connections at the same time. The server should not block. It should be able to answer all demands. Do some test with the IRC client and nc at the same time.
Join a channel thanks to the appropriate command. Ensure that all messages from one client on that channel are sent to all other clients that joined the channel.
 Yes
 No
Networking specials

Network communications can be disturbed by many strange situations.

Just like in the subject, using nc, try to send partial commands. Check that the server answers correctly. With a partial command sent, ensure that other connections still run fine.
Unexpectedly kill a client. Then check that the server is still operational for the other connections and for any new incoming client.
Unexpectedly kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked.
Stop a client (^-Z) connected on a channel. Then flood the channel using another client. The server should not hang. When the client is live again, all stored commands should be processed normally. Also, check for memory leaks during this operation.
 Yes
 No
Client Commands basic

With both nc and the reference IRC client, check that you can authenticate, set a nickname, a username, join a channel. This should be fine (you should have already done this previously).
Verify that private messages (PRIVMSG) are fully functional with different parameters.
 Yes
 No
Client Commands channel operator

With both nc and the reference IRC client, check that a regular user does not have privileges to do channel operator actions. Then test with an operator. All the channel operation commands should be tested (remove one point for each feature that is not working).
Rate it from 0 (failed) through 5 (excellent)






///////////////
以下は、`ft_irc` プロジェクトの評価基準に関する「日本語訳」と「解説」です。評価チェックリストの構成に従って順に訳し、各項目で何を求められているかを明確にします。

---

# 🧾 ft\_irc 評価スケールの日本語訳と解説

---

## 🟢 導入：評価者としての心構え（Introduction）

### ✅ 日本語訳

* 丁寧で礼儀正しく、敬意を持ち、建設的に評価を進めてください。
* 評価中に見つかった問題点は、相手と議論して理解し合う努力をしてください。
* 同じ課題でも理解の差があることを前提に、公平かつ正直に評価してください。
* 教育的な価値は、真剣なピアレビューを通じて最大化されます。

### ✅ 解説

→ 批判的にならず、協力的に「どこができていて、どこが改善できるか」を一緒に確認する姿勢が求められます。

---

## 📁 評価前のガイドライン（Guidelines）

### ✅ 日本語訳と解説

* **提出物は必ず Git レポジトリ内のものに限定**してください。
* `git clone` は**空ディレクトリで実行**し、正しい内容であることを確認してください。
* `.bashrc` などで **alias やスクリプトで偽装されていないか注意**してください。
* もし評価用スクリプトが使われるなら、内容を確認して両者合意のもと使用してください。
* 評価前に**自分が課題を終えていない場合**は、**必ず課題内容をすべて読んでから**評価してください。
* 評価中に以下が発覚したら**評価を終了**し、該当フラグをつけます：

  * Git レポジトリが空 → 0点
  * プログラムが動作しない → 0点
  * Norm違反 → 0点
  * チート（他人のコードのコピーなど）→ -42点

また、**実行中に Segfault や予期しない終了があった場合**も0点になります。

---

## 🛠 Mandatory Part（必須項目）

### ✅ Basic Checks（ビルドと構文チェック）

| チェック内容                                         | 説明                                          |
| ---------------------------------------------- | ------------------------------------------- |
| Makefile があるか                                      | `make` でコンパイルできる                            |
| C++ で書かれているか                                      | C言語ではNG                                     |
| 実行ファイル名が正しいか                                   | `ircserv`など指定された名前か                         |
| `poll()` は1箇所だけか                                   | イベントループで1回使うだけ（selectやepollでも同様）            |
| `poll()` は必ず `accept`/`read`/`write` 前に呼ばれているか | 読み書き・accept前に必須。`errno == EAGAIN` のような分岐はNG |
| `fcntl()` は `F_SETFL, O_NONBLOCK` だけか               | 他の使い方はNG                                    |

⚠ これらに1つでも違反すると「評価中止・0点」。

---

### ✅ Networking（ネットワーク関連）

| チェック内容                                           | 解説 |
| ------------------------------------------------ | -- |
| サーバーはコマンドライン引数のポートで全ネットワークインターフェースを listen しているか |    |
| `nc`（netcat）で接続・送信・応答できるか                        |    |
| チームが使っているIRCクライアント（例: irssi）を確認し、実際に接続できるか       |    |
| 複数接続が同時に行えて、ブロックされず処理されるか（nc + IRC同時使用）          |    |
| チャンネルにJOINして、1人が発言したら他のJOIN者全員に届くか               |    |

---

### ✅ Networking Specials（異常系の挙動）

| チェック内容                                                       | 解説 |
| ------------------------------------------------------------ | -- |
| nc でコマンドを部分的に送ったときに、サーバーが正しく動作するか                            |    |
| 途中でクライアントを強制終了しても、他の接続が正常に動くか                                |    |
| 中途半端なコマンドを送ったあとで nc を切ってもサーバーが異常にならないか                       |    |
| クライアントを一時停止（Ctrl+Z）し、他のクライアントから送信→再開したときにキューが処理され、メモリリークもないか |    |

---

### ✅ Client Commands basic（基本コマンド）

| チェック内容                                                     | 解説 |
| ---------------------------------------------------------- | -- |
| `nc` や IRC クライアントで `NICK`, `USER`, `JOIN` など基本的な認証と参加ができるか |    |
| `PRIVMSG`（個人またはチャンネルへのメッセージ）が正常に動作するか                      |    |

---

### ✅ Client Commands channel operator（チャンネル管理コマンド）

| チェック内容                                                       | 解説 |
| ------------------------------------------------------------ | -- |
| 通常ユーザーがチャンネルOP権限を持たないこと                                      |    |
| OPユーザーで以下のコマンドが動作するか：`MODE`, `TOPIC`, `INVITE`, `KICK`, `+o` |    |
| 各機能のテスト結果に応じて 0〜5 点で評価                                       |    |

