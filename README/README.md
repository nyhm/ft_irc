# ft_irc
要件
https://cdn.intra.42.fr/pdf/pdf/173717/en.subject.pdf
AIの使用が認められて要件が変わった

レビュー
https://web.archive.org/web/20250429115023/https://42-evaluation-sheets-hub.vercel.app/Cursus/Ft_irc/


レビュー内容:
Introduction
イントロダクション
Please comply with the following rules:
次の規則に従ってください。

Remain polite, courteous, respectful and constructive throughout the evaluation process. The well-being of the community depends on it.
評価の全過程で、丁寧・礼儀正しく・敬意を払い・建設的であること。コミュニティの健全性はそれにかかっています。

Identify with the student or group whose work is evaluated the possible dysfunctions in their project. Take the time to discuss and debate the problems that may have been identified.
評価対象の学生やグループと一緒に、プロジェクトの不具合の可能性を特定すること。見つかった問題について時間をかけて議論してください。

You must consider that there might be some differences in how your peers might have understood the project's instructions and the scope of its functionalities. Always keep an open mind and grade them as honestly as possible. The pedagogy is useful only and only if the peer-evaluation is done seriously.
同輩が課題の指示や機能範囲を理解した仕方に差がある可能性を考慮してください。常に柔軟な姿勢を保ち、可能な限り誠実に採点してください。ピア評価が真剣に行われてこそ教育は有益です。


Guidelines
ガイドライン

Only grade the work that was turned in the Git repository of the evaluated student or group.
採点は評価対象の学生／グループのGitリポジトリに提出された成果物のみに限ること。

Double-check that the Git repository belongs to the student(s). Ensure that the project is the one expected. Also, check that 'git clone' is used in an empty folder.
リポジトリが学生本人のものであることを再確認し、プロジェクトが想定のものであることを確認する。さらに、空のフォルダに対して git clone が実行されているか確認する。

Check carefully that no malicious aliases was used to fool you and make you evaluate something that is not the content of the official repository.
公式リポジトリの内容ではないものを評価させるための悪意あるエイリアスが使われていないか注意深く確認する。

To avoid any surprises and if applicable, review together any scripts used to facilitate the grading (scripts for testing or automation).
想定外を避けるため、該当する場合は採点補助（テストや自動化）に使うスクリプトを事前に一緒に確認する。

If you have not completed the assignment you are going to evaluate, you have to read the entire subject prior to starting the evaluation process.
あなた自身が評価対象の課題を未完了の場合は、評価開始前に課題文を全て読むこと。

Use the available flags to report an empty repository, a non-functioning program, a Norm error, cheating, and so forth.
空のリポジトリ、動作しないプログラム、Normエラー、不正行為などは用意されたフラグで報告すること。

In these cases, the evaluation process ends and the final grade is 0, or -42 in case of cheating. However, except for cheating, student are strongly encouraged to review together the work that was turned in, in order to identify any mistakes that shouldn't be repeated in the future.
これらの場合、評価はそこで終了し最終評価は0点（不正行為の場合は-42）。ただし不正を除き、将来繰り返すべきでない誤りを特定するため提出物を一緒に振り返ることを強く推奨します。

Remember that for the duration of the defence, no segfault, no other unexpected, premature, uncontrolled or unexpected termination of the program, else the final grade is 0. Use the appropriate flag.
口頭試問（ディフェンス）中にセグフォやその他の予期せぬ・早期の・制御不能な終了が起きた場合、最終評価は0点であることを忘れないこと。該当フラグを使用する。

You should never have to edit any file except the configuration file if it exists. If you want to edit a file, take the time to explicit the reasons with the evaluated student and make sure both of you are okay with this.
設定ファイルがある場合を除き、どのファイルも編集する必要はありません。編集が必要な場合は、理由を評価対象者と明確に話し合い、双方の合意を得てください。

You must also verify the absence of memory leaks. Any memory allocated on the heap must be properly freed before the end of execution.
メモリリークがないことも検証しなければなりません。ヒープに確保したメモリは終了前に適切に解放する必要があります。

You are allowed to use any of the different tools available on the computer, such as leaks, valgrind, or e_fence. In case of memory leaks, tick the appropriate flag.
leaks、valgrind、e_fence など利用可能なツールは使用可。リークがあれば該当フラグにチェックすること。

Attachments
添付
subject.pdf bircd.tar.gz
subject.pdf bircd.tar.gz


Mandatory Part
必須パート

Basic checks
基本チェック

There is a Makefile, the project compiles correctly with the required options, is written in C++, and the executable is called as expected.
Makefileがあり、指定オプションで正しくコンパイルでき、C++で書かれており、実行ファイル名が要件どおりであること。

Ask and check how many poll() (or equivalent) are present in the code. There must be only one.
コード内に poll()（または同等機能）がいくつあるか確認する。1つのみでなければならない。

Verify that the poll() (or equivalent) is called every time before each accept, read/recv, write/send. After these calls, errno should not be used to trigger specific action (e.g. like reading again after errno == EAGAIN).
accept、read/recv、write/send の各操作の前に毎回 poll()（または同等）が呼ばれていることを検証する。これらの呼び出し後に、errno を根拠に特別な処理（例：errno == EAGAIN で再読込など）をしてはならない。

Verify that each call to fcntl() is done as follows: fcntl(fd, F_SETFL, O_NONBLOCK); Any other use of fcntl() is forbidden.
fcntl() の呼び出しは fcntl(fd, F_SETFL, O_NONBLOCK); の形に限る。その他の fcntl() の使い方は禁止。

If any of these points is wrong, the evaluation ends now and the final mark is 0.
これらのいずれかに誤りがあれば評価はそこで終了し、最終評価は0点。



Networking
ネットワーキング

Check the following requirements:
以下の要件を確認すること：

The server starts, and listens on all network interfaces on the port given from the command line.
サーバが起動し、コマンドラインで与えられたポートで全ネットワークインターフェースに対して待受している。

Using the 'nc' tool, you can connect to the server, send commands, and the server answers you back.
nc ツールでサーバに接続し、コマンドを送信でき、サーバが応答する。

Ask the team what is their reference IRC client.
チームが参照（基準）としているIRCクライアントを確認する。

Using this IRC client, you can connect to the server.
そのIRCクライアントでサーバに接続できる。

The server can handle multiple connections at the same time. The server should not block. It should be able to answer all demands. Do some test with the IRC client and nc at the same time.
サーバは同時に複数接続を処理でき、ブロックせず、全ての要求に応答できる。同時にIRCクライアントと nc でテストを行う。

Join a channel thanks to the appropriate command. Ensure that all messages from one client on that channel are sent to all other clients that joined the channel.
適切なコマンドでチャンネルに参加し、そのチャンネルで1クライアントが送った全メッセージが参加中の他クライアント全員に配信されることを確認する。


Networking specials
ネットワーク特記事項

Network communications can be disturbed by many strange situations.
ネットワーク通信は様々な異常状況に乱され得る。

Just like in the subject, using nc, try to send partial commands. Check that the server answers correctly. With a partial command sent, ensure that other connections still run fine.
課題文どおりに、nc を使って部分的なコマンドを送ってみる。サーバが正しく応答すること、部分コマンド送信中でも他の接続が正常に動作することを確認する。

Unexpectedly kill a client. Then check that the server is still operational for the other connections and for any new incoming client.
クライアントを予期せず終了させる。その後、他の接続や新規接続に対してサーバが引き続き正常動作することを確認する。

Unexpectedly kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked.
コマンド半分を送った状態で nc を突然終了する。サーバが異常状態やブロックに陥っていないことを再確認する。

Stop a client (^-Z) connected on a channel. Then flood the channel using another client. The server should not hang. When the client is live again, all stored commands should be processed normally. Also, check for memory leaks during this operation.
チャンネルに接続中のクライアントを停止（^-Z、Ctrl-Z）する。別のクライアントでチャンネルをフラッドする。サーバはハングしてはならない。停止したクライアントが復帰したら、蓄積された全コマンドが正常に処理されること。併せてこの操作中のメモリリークも確認する。


Client Commands basic
クライアントコマンド（基礎）

With both nc and the reference IRC client, check that you can authenticate, set a nickname, a username, join a channel. This should be fine (you should have already done this previously).
nc と参照IRCクライアントの両方で、認証・ニックネーム設定・ユーザー名設定・チャンネル参加ができることを確認する。（これは既に前段で確認済みのはず。）

Verify that private messages (PRIVMSG) are fully functional with different parameters.
プライベートメッセージ（PRIVMSG）が様々なパラメータで完全に機能することを検証する。


Client Commands channel operator
クライアントコマンド（チャンネルオペレータ）

With both nc and the reference IRC client, check that a regular user does not have privileges to do channel operator actions. Then test with an operator. All the channel operation commands should be tested (remove one point for each feature that is not working).
nc と参照IRCクライアントの両方で、一般ユーザにチャンネルオペレータ権限がないことを確認し、次にオペレータでテストする。チャンネル操作コマンドは全てテストし、動作しない機能1つにつき1点減点すること。

rgb(255,0,0)
それぞれに対する解説
Mandatory Part — Basic checks（基本）
Makefileあり／指定オプションでC++ビルド／実行名規定どおり
意味：makeで一発ビルド、-Wall -Wextra -Werror等、出力名が要求通り（例：ircserv）。
確認：make clean && make → 失敗しない、./ircserv が生成。

poll()は1か所のみ
意味：イベント多重化は単一の中央ループで管理。ネストや複数スレッド/複数poll禁止。
確認：コード検索でpoll(の呼び出し回数=1（ラッパ関数も含めて実体1）。

accept/read/writeの前に毎回pollを呼ぶ。errnoで再試行制御しない
意味：準備完了に基づくI/Oが前提。EAGAINをトリガに回す実装はNG。
確認：POLLIN/POLLOUTフラグを見てからrecv/send。while (errno==EAGAIN) の類が無い。

fcntlは fcntl(fd, F_SETFL, O_NONBLOCK); のみ
意味：非ブロッキング化以外のfcntl利用禁止（他フラグ操作やF_SETOWN等は不可）。
確認：該当呼び出しの形と回数、他用途が無いこと。


Networking（基本動作）

全IFで指定ポートlisten
意味：bind(0.0.0.0:PORT)（または::）。127.0.0.1限定は不可。
確認：ss -ltnp/netstatで0.0.0.0:PORTを確認。

ncで接続・コマンド送信・応答
意味：手動テスト可能な最低限の対話が成立。
確認：nc 127.0.0.1 PORT→NICK/USER等に数値リプライが返る。

参照IRCクライアントの確認
意味：互換対象（irssi/weechat等）を明確化。
確認：チームに口頭確認→そのクライアントで接続検証。

そのクライアントで接続成功
意味：一般的IRCクライアントと実用互換。
確認：サーバ追加→接続→登録完了メッセージ受領。

同時複数接続・非ブロック・全要求に応答
意味：N>1クライアントで遅延/固まり無し。
確認：ncとGUIクライアントを同時接続→相互やり取りが滞らない。

JOIN後のブロードキャスト
意味：あるクライアントのチャンネル投稿が他の参加者全員へ配信。
確認：A,B,Cが#x参加→Aの発言がB,Cへ届く（A自身への二重送信は不要）。

Networking specials（異常系）
部分コマンド（フラグメント）対応
意味："PRIV" →（後から）"MSG #c :hi\r\n" のような分割受信でも正しくバッファリングし、CRLF確定で処理。
確認：ncで小出し送信→サーバが固まらず、完全行でのみ処理。

クライアントを予期せずkillしても継続
意味：切断の掃除（FD/データ構造）を正しく実施。
確認：kill -9/ウィンドウ閉じ→他接続/新規接続が正常。

半端な行を送信中にncをkillしても健全
意味：中途メッセージの捨て方やバッファ破棄が安全。
確認：半行送信→プロセスkill→サーバが詰まらない。

Ctrl-Zで一時停止中に別クライアントでフラッド→サーバはハングしない。復帰後に滞留送信が正常処理。リーク無し
意味：送信キューを持ち、POLLOUTで少しずつ吐く設計。停止中の相手にsend()でブロックしない。
確認：Aを停止、Bで大量発言→サーバ健在。A再開→キューが順次送られる。valgrindでもOK。

Client Commands basic（基礎コマンド）

認証／NICK／USER／JOINが両方（ncとGUI）で動く
意味：登録フローがRFC互換の数値リプライで成立。
確認：PASS/NICK/USER→RPL_WELCOMEなど→JOIN #xが成功。

PRIVMSGが各種パラメータで完全動作
意味：宛先がユーザ・チャンネル、トレーリング（:以降にスペース含む）対応、エラー時に適切なERR_*。
確認：PRIVMSG nick :hi there / PRIVMSG #c :… / 不在ユーザ→ERR_NOSUCHNICK。

Client Commands channel operator（オペ権限）

一般ユーザはオペ権限なし
意味：非オペのKICK/MODE +o/+k/+l/+t/+i/INVITE/TOPIC等は拒否。
確認：一般で実行→権限エラー（該当ERR_*）。

オペでは各操作が動く。未実装1機能ごとに減点
意味：権限昇格後に全操作者能が機能。
確認：MODE #c +o user、+k key、+l 10、+t、+i、INVITE、KICK、TOPIC等が期待通り。

典型NG例の具体像（抜粋）

poll()を複数箇所で呼ぶ／ソケットごとに個別ループを回す。
recv()してEAGAINならすぐ再試行するスピン。
非ブロッキング未設定でsend()が詰まりハング。
行末\r\nではなく\nのみ処理／512バイト超の取扱い不備。
切断時にクライアントをチャンネルから外さずダングリング参照。
送信キュー未実装で一時停止クライアント宛send()がブロック。











## ft_ircを作るための基本設計と開発手順

### 1. **設計フェーズ**

* **プロジェクト要件の理解**

  * C++98標準で書く
  * 複数クライアントの同時接続をノンブロッキングI/O（poll()など）で処理
  * フォークは禁止。マルチプロセス・マルチスレッドは使わない
  * TCP/IP通信のみ
  * IRCの基本的なコマンド（認証、ニックネーム設定、チャネル参加、プライベートメッセージ送受信など）を実装
  * チャネルオペレーター用コマンド（KICK, INVITE, TOPIC, MODEなど）を実装
  * MacOSはfcntl()でノンブロッキング設定可能（必要に応じて）

* **通信モデルの設計**

  * メインイベントループ：poll()（またはselect(), epoll()）でソケットの状態を監視
  * クライアント管理：接続中のクライアント情報（ニックネーム、ユーザー名、認証状態など）を保持
  * チャネル管理：チャネル名、参加クライアントリスト、チャネルモード、トピック、ユーザーリミット等の管理

* **コマンドパーサー設計**

  * 受信したメッセージを解析し、IRCコマンド（NICK, USER, JOIN, PRIVMSG, KICKなど）を識別して処理
  * パケット断片を組み合わせて完全なコマンドとして扱うバッファリング機能の実装

---

### 2. **開発フェーズ**

* **初期セットアップ**

  * Makefile作成（-Wall -Wextra -Werrorを付ける、名前は規定通り）
  * ソケットの作成、バインド、listen、acceptを実装（ノンブロッキング設定）

* **ノンブロッキングI/Oの実装**

  * poll()で全クライアントソケットを監視し、読み書き可能な状態を判定
  * read/recvとwrite/sendはノンブロッキングで処理

* **クライアント管理**

  * 新規接続のaccept処理
  * クライアント情報を保存する構造体やクラスを用意
  * クライアントの切断処理

* **コマンドの解析と処理**

  * 受信データをバッファに貯めて改行単位でコマンドを分割
  * コマンドごとに処理関数を用意（NICK, USER, JOIN, PRIVMSGなど）
  * 認証フローの実装（接続パスワードの検証）

* **チャネル管理機能**

  * チャネル参加・退室処理
  * チャネルごとのユーザーリスト管理
  * メッセージのブロードキャスト（チャネル内の他クライアントに転送）

* **チャネルオペレーターコマンド実装**

  * KICK（チャネルからの強制退出）
  * INVITE（チャネルへの招待）
  * TOPIC（トピックの閲覧・変更）
  * MODE（チャネルモードの設定・解除）

* **エラーハンドリングと安定性**

  * 不正なコマンドや状態異常への対応
  * クライアントが切断した場合のリソース解放
  * メモリリークチェック

---

### 3. **テスト・デバッグ**

* **リファレンスクライアントで接続テスト**
* **複数クライアント同時接続・通信確認**
* **IRCコマンドの動作検証**
* **異常系テスト（途中切断、バッファの分割受信など）**

---

### 4. **ドキュメント整備と提出準備**

* コードにコメントを入れて可読性向上
* Makefileの動作確認
* ファイル・フォルダ名の確認
* Gitで正しく管理・提出



身につく技術

### 1. **ネットワークプログラミング**

* TCPソケットの基本操作（socket作成、bind、listen、accept、connect）
* ノンブロッキングI/Oの理解と実装（poll/select/epollの使い方）
* クライアント複数同時接続の処理（マルチクライアント対応）

### 2. **システムプログラミング**

* ファイルディスクリプタの管理
* ノンブロッキングモードでのI/O制御
* シグナル処理やエラー処理の基本

### 3. **プロトコル理解と実装**

* IRCプロトコルの仕様理解
* メッセージのパースと処理（コマンド解析）
* プロトコルに基づく正しいレスポンスの生成

### 4. **C++基礎とスタンダード準拠**

* C++98標準に準拠したコード記述
* C++の標準ライブラリ利用（string, vectorなど）
* 効率的で安全なメモリ管理

### 5. **ソフトウェア設計**

* イベント駆動型プログラムの設計（イベントループ）
* 状態管理（クライアントの認証状態やチャネル状態の管理）
* モジュール分割や責務分離

### 6. **デバッグ・テスト技術**

* ネットワーク通信のトラブルシューティング
* 不完全データの処理や分割受信への対応
* 複数クライアント間の同期問題の解決

### 7. **バージョン管理**

* Gitリポジトリを使ったソース管理・提出


要件
https://cdn.intra.42.fr/pdf/pdf/173717/en.subject.pdf

rgb(255,0,0)
rgb(255,0,0)
Chapter I
Introduction
インターネットリレーチャット（IRC）は、インターネット上でのテキストベースの通信プロトコルです。

Internet Relay Chat or IRC is a text-based communication protocol on the Internet.
これは、公開または非公開のリアルタイムメッセージングを提供します。

It offers real-time messaging that can be either public or private.
ユーザーは直接メッセージを交換したり、グループチャネルに参加したりできます。

Users can exchange direct messages and join group channels.
IRCクライアントはチャネルに参加するためにIRCサーバに接続します。

IRC clients connect to IRC servers in order to join channels.
IRCサーバはネットワークを形成するために相互に接続されています。

IRC servers are connected together to form a network.


Chapter II
General rules
第2章
一般規則

• Your program should not crash in any circumstances (even when it runs out of memory), and should not quit unexpectedly.
• プログラムはどんな状況でもクラッシュしてはならず（メモリ不足時も含む）、予期せず終了してはいけません。

If it happens, your project will be considered non-functional and your grade will be 0.
もしそうなった場合、プロジェクトは機能しないものと見なされ、評価は0点になります。

• You have to turn in a Makefile which will compile your source files. It must not perform unnecessary relinking.
• ソースファイルをコンパイルするMakefileを提出しなければなりません。不要な再リンクは行ってはいけません。

• Your Makefile must at least contain the rules: \$(NAME), all, clean, fclean and re.
• Makefileには最低限、\$(NAME)、all、clean、fclean、re のルールを含める必要があります。

• Compile your code with c++ using the flags -Wall -Wextra -Werror.
• コードは c++ でコンパイルし、フラグ -Wall -Wextra -Werror を使用してください。

• Your code must comply with the C++ 98 standard. Then, it should still compile if you add the flag -std=c++98.
• コードはC++98標準に準拠していなければならず、-std=c++98フラグを付けてもコンパイルできる必要があります。

• Try to always code using C++ features when available (for example, choose \<cstring> over \<string.h>). You are allowed to use C functions, but always prefer their C++ versions if possible.
• 可能な場合は常にC++の機能を使ってコードを書くようにしてください（例えば、\<string.h>ではなく\<cstring>を使う）。C関数の使用は許可されていますが、可能な限りC++のバージョンを優先してください。

• Any external library and Boost libraries are forbidden.
• 外部ライブラリおよびBoostライブラリの使用は禁止されています。

Chapter III
Mandatory Part
第3章
必須パート

Program name ircserv
プログラム名：ircserv

Turn in files Makefile, \*.{h, hpp}, \*.cpp, *.tpp, *.ipp, an optional configuration file
提出ファイル：Makefile、ヘッダーファイル（*.h, *.hpp）、ソースファイル（*.cpp）、テンプレート実装ファイル（*.tpp, \*.ipp）、オプションの設定ファイル

Makefile rules: NAME, all, clean, fclean, re
Makefileのルール：NAME、all、clean、fclean、re

Arguments:
引数:

* port: The listening port
  ポート番号：IRCサーバーが接続待ち受けを行うポート
* password: The connection password
  パスワード：サーバーに接続しようとするIRCクライアントが使用する接続パスワード

External functions allowed: Everything in C++ 98 plus the following system calls:
使用可能な外部関数: C++98標準に加えて、以下のシステムコールを使用可能
socket, close, setsockopt, getsockname, getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo, bind, connect, listen, accept, htons, htonl, ntohs, ntohl, inet_addr, inet_ntoa, send, recv, signal, sigaction, lseek, fstat, fcntl, poll (or equivalent)

Libft authorized: n/a
Libftの使用：不可

Description: An IRC server in C++ 98
説明：C++ 98で作成するIRCサーバー

原文: You are required to develop an IRC server using the C++ 98 standard.
和訳: C++98規格を用いてIRCサーバを開発することが求められます。
意味: `-std=c++98`でコンパイルできること。C++11以降の機能（`auto`/`nullptr`/ラムダ等）は使用不可。

---

原文: You must not develop an IRC client.
和訳: IRCクライアントを開発してはなりません。
意味: 提出物はサーバのみ。動作確認に使う`nc`や既存クライアントは可だが、自作クライアントは課題範囲外。

---

原文: You must not implement server-to-server communication.
和訳: サーバ間通信を実装してはなりません。
意味: 他IRCサーバへのリンクやネットワーク合流（links/TS/ネットスプリット処理）は不要。対象はクライアント対サーバのみ。

---

原文: Your executable will be run as follows:
./ircserv <port> <password>
和訳: 実行ファイルは次のように起動されます：`./ircserv <port> <password>`
意味: 実行バイナリ名は`ircserv`。引数は**ちょうど2つ**。不足・過多や不正値なら使用法表示等の適切なエラー処理が必要。

---

原文: • port: The port number on which your IRC server will be listening for incoming IRC connections.
和訳: ・port：IRC接続を受け付けるためにサーバが待ち受けるポート番号。
意味: 数値のポートを受け取り、そのポートで`bind`/`listen`する。無効値はエラー扱い（一般に1–65535の範囲）。

---

原文: • password: The connection password. It will be needed by any IRC client that tries to connect to your server.
和訳: ・password：接続用パスワード。サーバへ接続を試みるすべてのIRCクライアントに必要。
意味: クライアントは`PASS <password>`で提示。サーバは一致確認し、不一致/未送信なら登録（`NICK`/`USER`完了）を拒否し接続を拒否・切断。

---

原文: Even though poll() is mentioned in the subject and the evaluation scale, you may use any equivalent such as select(), kqueue(), or epoll().
和訳: 課題文や評価基準で`poll()`に言及がありますが、`select()`、`kqueue()`、`epoll()`など同等の手段を使用しても構いません。
意味: OSに応じたI/O多重化APIを任意選択可（macOSなら`kqueue`、Linuxなら`epoll`等）。挙動が同等になるようイベント駆動で実装すればよい。


III.1 Requirements
第III.1節 要件

[READMEBehavior.md](READMEBehavior.md)
細かい挙動をまとめた


原文: • The server must be capable of handling multiple clients simultaneously without hanging.
和訳: ・サーバは複数クライアントを同時に扱え、ハングしないこと。
意味: 1プロセスで多数接続を並行処理。I/O待ちや1クライアントの遅延で全体が止まらない設計（イベント駆動・送受信キュー必須）。

---

原文: • Forking is prohibited. All I/O operations must be non-blocking.
和訳: ・`fork`は禁止。すべてのI/Oはノンブロッキングで行うこと。
意味: `fork()`や子プロセス生成なし。`fcntl(fd, F_SETFL, O_NONBLOCK)`でlisten/クライアントFDを非ブロッキング化。

---

原文: • Only 1 poll() (or equivalent) can be used for handling all these operations (read, write, but also listen, and so forth).
和訳: ・読み書きだけでなくlisten等も含む全処理を扱う`poll()`（または同等）は**1つだけ**に限る。
意味: 中央のイベントループ1か所で多重化。ソケットごと・モジュールごとの複数`poll/select/kqueue/epoll_wait`は不可。

---

原文: Because you have to use non-blocking file descriptors, it is possible to use read/recv or write/send functions with no poll() (or equivalent), and your server wouldn’t be blocking. However, it would consume more system resources. Therefore, if you attempt to read/recv or write/send in any file descriptor without using poll() (or equivalent), your grade will be 0.
和訳: 非ブロッキングFDなら`poll()`なしで`read/recv`や`write/send`を呼んでもブロックはしませんが、より多くの資源を消費します。したがって、どのFDに対しても`poll()`（または同等）を使わずに`read/recv`や`write/send`を試みた場合、評価は0点です。
意味: \*\* readiness確認なしのI/O呼び出しは全面禁止 \*\*。必ず`POLLIN/POLLOUT`等で準備完了を見てから`recv/send`を行うこと（スピンやエラー再試行で回す実装はNG）。

---

原文: • Several IRC clients exist. You have to choose one of them as a reference. Your reference client will be used during the evaluation process.
和訳: ・複数のIRCクライアントが存在する。いずれか1つを参照クライアントとして選び、評価時にそれが使用される。
意味: 例：irssi / Weechat / HexChat等から1つを正式指定。以後はそのクライアント基準で互換性を確認。

---

原文: • Your reference client must be able to connect to your server without encountering any error.
和訳: ・参照クライアントはエラーなくサーバへ接続できなければならない。
意味: `PASS/NICK/USER`で登録完了し、数値リプライやMOTD等が期待通り。接続・認証・JOIN・送受信で例外/エラーなし。

---

原文: • Communication between client and server has to be done via TCP/IP (v4 or v6).
和訳: ・クライアントとサーバ間の通信はTCP/IP（IPv4またはIPv6）で行うこと。
意味: `SOCK_STREAM`。`bind`は`0.0.0.0`または`::`等。UDP不可。v4/v6どちらか対応でよい（両対応なら尚可）。

---

原文: • Using your reference client with your server must be similar to using it with any official IRC server. However, you only have to implement the following features:
和訳: ・参照クライアントの使用感は公式IRCサーバと同等であるべき。ただし、実装必須機能は以下に限定される：
意味: 完全互換は不要だが基本操作は同様に動くこと。エラー時は適切な数値リプライを返す。

---

原文: ◦ You must be able to authenticate, set a nickname, a username, join a channel, send and receive private messages using your reference client.
和訳: ・参照クライアントで認証、ニックネーム・ユーザー名設定、チャンネル参加、プライベートメッセージの送受信ができること。
意味: `PASS`→`NICK`→`USER`→`RPL_WELCOME`、`JOIN #ch`、`PRIVMSG nick/#ch :msg`が機能。

---

原文: ◦ All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
和訳: ・あるクライアントがチャンネルへ送ったメッセージは、そのチャンネル参加中の他の全クライアントに転送されること。
意味: チャンネル内ブロードキャスト実装。送信者自身への二重送信を避けるかどうかは仕様に合わせる。

---

原文: ◦ You must have operators and regular users.
和訳: ・オペレータ（管理者）と一般ユーザを備えること。
意味: チャンネルごとの役割管理（`isOperator`など）を保持。権限チェックが全コマンドで働くこと。

---

原文: ◦ Then, you have to implement the commands that are specific to channel operators:
和訳: ・次に、チャンネルオペレータ特有のコマンドを実装すること：
意味: 以降の`KICK/INVITE/TOPIC/MODE`を権限検証込みで実装。

---

原文: ∗ KICK - Eject a client from the channel
和訳: ・KICK — クライアントをチャンネルから追放。
意味: 対象が在室中であることを検証し、KICK通知を配信して退室処理。

---

原文: ∗ INVITE - Invite a client to a channel
和訳: ・INVITE — クライアントをチャンネルに招待。
意味: 招待テーブルを管理。`+i`（invite-only）時は招待済みのみJOIN許可。

---

原文: ∗ TOPIC - Change or view the channel topic
和訳: ・TOPIC — チャンネルトピックの変更/閲覧。
意味: `+t`時はオペのみ変更可。取得は全員可。変更時に通知送出。

---

原文: ∗ MODE - Change the channel’s mode:
和訳: ・MODE — チャンネルモードの変更：
意味: 下記サブモードを実装。引数の有無や順序を厳格にチェック。

---

原文: · i: Set/remove Invite-only channel
和訳: ・i：招待制（Invite-only）の設定/解除。
意味: `+i`で招待必須。`-i`で解除。JOIN時に招待チェック。

---

原文: · t: Set/remove the restrictions of the TOPIC command to channel operators
和訳: ・t：TOPICの変更権限をオペレータに制限/解除。
意味: `+t`でオペのみTOPIC変更可、`-t`で誰でも変更可。

---

原文: · k: Set/remove the channel key (password)
和訳: ・k：チャンネルキー（パスワード）の設定/解除。
意味: `+k <key>`で設定、`-k`で解除。`JOIN #ch key`の検証が必須。

---

原文: · o: Give/take channel operator privilege
和訳: ・o：チャンネルオペレータ権限の付与/剥奪。
意味: `+o <nick>`で昇格、`-o <nick>`で降格。対象ユーザの在室確認。

---

原文: · l: Set/remove the user limit to channel
和訳: ・l：チャンネルのユーザ数上限を設定/解除。
意味: `+l <n>`で上限設定、`-l`で解除。JOIN時に人数チェック。

---

原文: • Of course, you are expected to write a clean code.
和訳: ・当然ながら、クリーンなコードを書くことが求められます。
意味: 一貫したスタイル、分割設計、適切なエラーハンドリングとリソース解放、未使用コードや未定義動作の排除、警告ゼロ。

---

III.2 For MacOS only
III.2 MacOS専用

原文: Since MacOS does not implement write() in the same way as other Unix OSes, you are permitted to use fcntl().
和訳: macOS は他の Unix 系 OS と write() の実装が同一ではないため、fcntl() の使用を許可します。
意味: macOS の送受信挙動差（ブロッキング条件など）を吸収する目的で、**非ブロッキング化のための fcntl 使用のみ特例で可**。

---

原文: You must use file descriptors in non-blocking mode in order to get a behavior similar to the one of other Unix OSes.
和訳: 他の Unix 系 OS に近い動作にするため、ファイルディスクリプタは**非ブロッキングモード**で使用しなければなりません。
意味: `socket()` の戻りFD、`accept()` 後の各クライアントFD、`listen`用FDを**すべて非ブロッキング**に設定。以後は `poll()/select/kqueue/epoll` の**準備完了通知に従って I/O** を行う。

---

原文: However, you are allowed to use fcntl() only as follows: fcntl(fd, F_SETFL, O_NONBLOCK);
和訳: ただし、fcntl() は次の呼び出し**のみ**使用を認めます：`fcntl(fd, F_SETFL, O_NONBLOCK);`
意味: **許可される fcntl はこの一形態だけ**。`F_GETFL`で既存フラグ取得→OR 加算や、他の `cmd`/フラグの使用は禁止。

---

原文: Any other flag is forbidden.
和訳: 他のフラグはすべて禁止です。
意味: `FD_CLOEXEC` 等の別目的設定や `O_ASYNC` など**一切不可**。非ブロッキング以外の fcntl 利用は違反。

---

実装上の要点（参考・短縮）

* **設定箇所**: `listen_fd` と毎回の `accept()` 直後の `client_fd` に対して即 `fcntl(fd, F_SETFL, O_NONBLOCK);`
* **送受信**: `POLLIN/POLLOUT` を見てから `recv/send`。`EWOULDBLOCK/EAGAIN` は**送受信キューに滞留**させ、次回書き込み可能時に再送（errno をトリガにループ再試行しない）。
* **禁止例**: `fcntl(fd, F_GETFL)` や `fcntl(fd, F_SETFD, FD_CLOEXEC)`、`fcntl` を用いた他設定。


---

III.3 Test example
III.3 テスト例

原文: Verify every possible error and issue, such as receiving partial data, low bandwidth, etc.
和訳: 受信データの分割、低帯域など、起こり得るあらゆるエラーや問題を検証しなさい。
意味: ネットワーク異常（細切れ到着、遅延、輻輳、突然の切断）でもサーバが止まらず正しく動くかをテストすること。

---

原文: To ensure that your server correctly processes all data sent to it, the following simple test using nc can be performed:
和訳: サーバが送られたデータを正しく処理することを確認するため、以下の簡単な `nc` によるテストを行える。
意味: `nc`（netcat）で手軽に部分送信を再現するテスト方法を指定している。

---

原文:

```
$> nc -C 127.0.0.1 6667
com^Dman^Dd
$>
```

意味: `-C` は改行時に CRLF を送るオプション。ターミナルで `com` 入力→Ctrl+D→`man` 入力→Ctrl+D→`d` 入力→改行、という順で\*\*「command\r\n」\*\*を分割送信する例。

---

原文: Use ctrl+D to send the command in several parts: ’com’, then ’man’, then ’d\n’.
和訳: Ctrl+D を使ってコマンドを複数の部分に分けて送る：まず `com`、次に `man`、最後に `d\n`。
意味: 一つのコマンドが**複数回の受信（パケット）に分割**される状況を人工的に作り、サーバが一つに再構成できるかを確認する。

---

原文: In order to process a command, you have to first aggregate the received packets in order to rebuild it.
和訳: コマンドを処理するには、まず受信したパケットを集約して再構成しなければならない。
意味: **各接続ごとの受信バッファ**を持ち、到着データを追記→\*\*区切り（CRLF）\*\*まで溜める→**完全な1行**になったときだけパース・実行。未完は保留、残りは次回に繰り越す。

---


Chapter IV
Bonus part
第IV章
ボーナスパート

Here are additional features you may add to your IRC server to make it resemble an actual IRC server more closely:
IRCサーバーを実際のIRCサーバーにより近づけるために追加できる機能は以下の通り：

• Handle file transfer.
• ファイル転送を扱う機能。

• A bot.
• ボットの実装。

The bonus part will only be assessed if the mandatory part is PERFECT.
ボーナスパートは必須パートが完璧に仕上がっている場合のみ評価される。

Perfect means the mandatory part has been integrally done and works without malfunctioning.
完璧とは、必須パートが完全に実装され、正常に動作している状態を意味する。

If you have not passed ALL the mandatory requirements, your bonus part will not be evaluated at all.
必須要件のすべてを満たしていなければ、ボーナスパートは一切評価されない。

Chapter V
Submission and peer-evaluation
第V章
提出とピア評価

Submit your assignment to your Git repository as usual. Only the work inside your repository will be evaluated during the defense.
課題はいつも通りGitリポジトリに提出してください。防衛（発表）時にはリポジトリ内の作業のみが評価対象となります。

Do not hesitate to double-check the names of your files to ensure they are correct.
ファイル名が正しいかどうかを必ず再確認してください。

You are encouraged to create test programs for your project even though they will not be submitted or graded.
プロジェクトのためにテストプログラムを作成することが推奨されますが、それらは提出も評価もされません。

Those tests could be especially useful to test your server during defense, but also your peer’s if you have to evaluate another ft\_irc one day.
これらのテストは防衛時にサーバーの動作を検証するために特に役立ちますし、将来誰かのft\_ircを評価する場合にも役立つでしょう。

Indeed, you are free to use whatever tests you need during the evaluation process.
評価過程で必要なテストは自由に使って構いません。

Your reference client will be used during the evaluation process.
評価の際にはあなたが選んだリファレンスクライアントが使用されます。

