# ft_irc


## ft\_ircを作るための基本設計と開発手順

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
https://cdn.intra.42.fr/pdf/pdf/156649/en.subject.pdf

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

• Try to always code using C++ features when available (for example, choose <cstring> over \<string.h>). You are allowed to use C functions, but always prefer their C++ versions if possible.
• 可能な場合は常にC++の機能を使ってコードを書くようにしてください（例えば、<string.h>ではな<cstring>を使う）。C関数の使用は許可されていますが、可能な限りC++のバージョンを優先してください。

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
socket, close, setsockopt, getsockname, getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo, bind, connect, listen, accept, htons, htonl, ntohs, ntohl, inet\_addr, inet\_ntoa, send, recv, signal, sigaction, lseek, fstat, fcntl, poll (or equivalent)

Libft authorized: n/a
Libftの使用：不可

Description: An IRC server in C++ 98
説明：C++ 98で作成するIRCサーバー

You are required to develop an IRC server using the C++ 98 standard.
C++98標準でIRCサーバーを開発する必要があります。

You must not develop an IRC client.
IRCクライアントを開発してはいけません。

You must not implement server-to-server communication.
サーバー間通信の実装は禁止されています。

Your executable will be run as follows:
実行は以下のように行われます。
`./ircserv <port> <password>`

• port: The port number on which your IRC server will be listening for incoming IRC connections.
• port：IRCサーバーが接続を待ち受けるポート番号。

• password: The connection password. It will be needed by any IRC client that tries to connect to your server.
• password：接続パスワード。IRCクライアントが接続する際に必要。

Even though poll() is mentioned in the subject and the evaluation scale, you may use any equivalent such as select(), kqueue(), or epoll().
poll()の使用が課題説明や評価基準に記載されていますが、select(), kqueue(), epoll()などの同等の関数を使っても構いません。

III.1 Requirements
第III.1節 要件

• The server must be capable of handling multiple clients simultaneously without hanging.
• サーバーは複数のクライアントを同時に処理でき、処理が停止（ハング）しないこと。

• Forking is prohibited. All I/O operations must be non-blocking.
• fork()の使用は禁止。すべての入出力操作はノンブロッキングで行うこと。

• Only 1 poll() (or equivalent) can be used for handling all these operations (read, write, but also listen, and so forth).
• これらの操作（読み込み、書き込み、リッスンなど）を管理するために使える poll()（または同等の関数）は1つだけ。

Because you have to use non-blocking file descriptors, it is possible to use read/recv or write/send functions with no poll() (or equivalent), and your server wouldn’t be blocking.
ノンブロッキングファイルディスクリプタを使うため、poll()なしでもread/recvやwrite/sendを使いサーバーがブロックしない動作は可能。

However, it would consume more system resources.
しかし、それはより多くのシステムリソースを消費する。

Therefore, if you attempt to read/recv or write/send in any file descriptor without using poll() (or equivalent), your grade will be 0.
したがって、poll()（または同等の関数）を使わずにファイルディスクリプタでread/recvやwrite/sendを行った場合、評価は0となる。

• Several IRC clients exist. You have to choose one of them as a reference. Your reference client will be used during the evaluation process.
• 複数のIRCクライアントが存在するが、その中から1つを基準クライアントとして選択し、評価時に使用する。

• Your reference client must be able to connect to your server without encountering any error.
• 基準クライアントがエラーなくサーバーに接続できること。

• Communication between client and server has to be done via TCP/IP (v4 or v6).
• クライアントとサーバー間の通信はTCP/IP（IPv4またはIPv6）で行うこと。

• Using your reference client with your server must be similar to using it with any official IRC server. However, you only have to implement the following features:
• 基準クライアントと自分のサーバーを使った通信は、公式のIRCサーバーと使う場合と似ていること。ただし、以下の機能のみ実装すればよい。

◦ You must be able to authenticate, set a nickname, a username, join a channel, send and receive private messages using your reference client.
◦ 認証、ニックネーム設定、ユーザー名設定、チャンネル参加、プライベートメッセージの送受信が基準クライアントでできること。

◦ All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
◦ 1人のクライアントがチャンネルに送ったメッセージは、そのチャンネルに参加しているすべてのクライアントに転送されること。

◦ You must have operators and regular users.
◦ オペレーターと通常ユーザーの区別があること。

◦ Then, you have to implement the commands that are specific to channel operators:
◦ さらに、チャンネルオペレーターに特有のコマンドを実装すること。

∗ KICK - Eject a client from the channel
∗ KICK - チャンネルからクライアントを追放する。

∗ INVITE - Invite a client to a channel
∗ INVITE - クライアントをチャンネルに招待する。

∗ TOPIC - Change or view the channel topic
∗ TOPIC - チャンネルのトピックを変更または確認する。

∗ MODE - Change the channel’s mode:
∗ MODE - チャンネルのモードを変更する。

· i: Set/remove Invite-only channel
· i: 招待のみ参加可能なチャンネルに設定・解除する。

· t: Set/remove the restrictions of the TOPIC command to channel operators
· t: TOPICコマンドの制限をチャンネルオペレーターのみに設定・解除する。

· k: Set/remove the channel key (password)
· k: チャンネルキー（パスワード）を設定・解除する。

· o: Give/take channel operator privilege
· o: チャンネルオペレーター権限を付与または剥奪する。

· l: Set/remove the user limit to channel
· l: チャンネルのユーザー制限数を設定または解除する。

• Of course, you are expected to write a clean code.
• もちろん、きれいなコードを書くことが期待されている。

---

III.2 For MacOS only
III.2 MacOS専用

Since MacOS does not implement write() in the same way as other Unix OSes, you are permitted to use fcntl().
MacOSは他のUnix系OSと同じ方法でwrite()を実装していないため、fcntl()の使用が許可されている。

You must use file descriptors in non-blocking mode in order to get a behavior similar to the one of other Unix OSes.
他のUnix系OSと似た挙動にするために、ファイルディスクリプタはノンブロッキングモードで使用しなければならない。

However, you are allowed to use fcntl() only as follows:
ただし、fcntl()の使用は以下の方法に限られる：

fcntl(fd, F\_SETFL, O\_NONBLOCK);
ファイルディスクリプタfdをノンブロッキングモードに設定する。

Any other flag is forbidden.
その他のフラグの使用は禁止されている。

---

III.3 Test example
III.3 テスト例

Verify every possible error and issue, such as receiving partial data, low bandwidth, etc.
部分的なデータ受信や低帯域幅など、あらゆるエラーや問題を検証すること。

To ensure that your server correctly processes all data sent to it, the following simple test using nc can be performed:
サーバーが送信されたすべてのデータを正しく処理しているか確認するために、以下のようなncコマンドを使った簡単なテストが行える。

\$> nc -C 127.0.0.1 6667
com^Dman^Dd
\$>

Use ctrl+D to send the command in several parts: ’com’, then ’man’, then ’d\n’.
Ctrl+Dを使ってコマンドを複数の部分に分けて送信する：’com’、次に’man’、最後に’d\n’。

In order to process a command, you have to first aggregate the received packets in order to rebuild it.
コマンドを処理するには、まず受信したパケットを集約し、コマンドを再構築する必要がある。

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

