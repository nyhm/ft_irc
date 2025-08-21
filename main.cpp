/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 10:47:23 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/21 10:48:23 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <netinet/in.h>
#include <fcntl.h>

#include "Parser.hpp"
#include "Client.hpp"
#include "Util.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Server.hpp"





// コマンドディスパッチ関数
static void dispatchCommand(Client& client, const Message& msg) {
    std::printf("DISPATCH: cmd=%s, registered=%d, nickname='%s', passOk=%d, username='%s'\n", 
                msg.cmd.c_str(), client.registered, client.nickname.c_str(), client.passOk, client.username.c_str());
    
    // 登録が必要なコマンドかチェック
    if (requiresRegistration.find(msg.cmd) != requiresRegistration.end() && !client.registered) {
        std::printf("SENDING 451: %s requires registration\n", msg.cmd.c_str());
        sendNotRegistered(client, msg.cmd);
        return;
    }
    
    // コマンドハンドラを検索
    std::map<std::string, Client::CommandHandler>::const_iterator it = commandHandlers.find(msg.cmd);
    if (it != commandHandlers.end()) {
        std::printf("HANDLING: %s\n", msg.cmd.c_str());
        it->second(client, msg);
        std::printf("AFTER HANDLER: wbuf.size=%zu\n", client.wbuf.size());
    } else {
        // 未実装コマンド
        std::printf("UNKNOWN COMMAND: %s\n", msg.cmd.c_str());
        if(client.registered)
            commandHandlers.at("UNKNOWN")(client, msg);
    }
}

static void set_nonblock(int fd) {
    // 評価要件: fcntl は F_SETFL, O_NONBLOCK のみ
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl O_NONBLOCK");
        std::exit(1);
    }
}

// CRLFで1行取り出す（\n単独も許容）
static bool pop_line(std::string &buf, std::string &line) {
    std::string::size_type p = buf.find("\r\n");
    if (p != std::string::npos) {
        line.assign(buf, 0, p);
        buf.erase(0, p + 2);
        return true;
    }
    p = buf.find('\n');
    if (p != std::string::npos) {
        line.assign(buf, 0, p);
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        buf.erase(0, p + 1);
        return true;
    }
    return false;
}

// 512バイト上限（CRLF込み）をだいたい守るためのガード
static bool line_too_long(const std::string &buf) { return buf.size() > 510; }

// ---------- 本体 ----------

int main(int argc, char **argv) {
    // 静的変数の初期化
    requiresRegistrationInit.insert("JOIN");
    requiresRegistrationInit.insert("PART");
    requiresRegistrationInit.insert("MODE");
    requiresRegistrationInit.insert("TOPIC");
    requiresRegistrationInit.insert("NAMES");
    requiresRegistrationInit.insert("LIST");
    requiresRegistrationInit.insert("INVITE");
    requiresRegistrationInit.insert("KICK");
    requiresRegistrationInit.insert("UNKNOWN");
    
    commandHandlersInit["PASS"] = handlePass;
    commandHandlersInit["JOIN"] = handleJoin;
    commandHandlersInit["PART"] = handlePart;
    commandHandlersInit["MODE"] = handleMode;
    commandHandlersInit["TOPIC"] = handleTopic;
    commandHandlersInit["INVITE"] = handleInvite;
    commandHandlersInit["KICK"] = handleKick;
    commandHandlersInit["PRIVMSG"] = handlePrivmsg;
    commandHandlersInit["NICK"] = handleNick;
    commandHandlersInit["USER"] = handleUser;
    commandHandlersInit["PING"] = handlePing;
    commandHandlersInit["QUIT"] = handleQuit;
    commandHandlersInit["UNKNOWN"] = handleUnknown;
    commandHandlersInit["CAP"] = handleCap;

    if (argc != 3) {
        std::fprintf(stderr, "Usage: %s <port> <pass>\n", argv[0]);
        return 1;
    }
    const int port = std::atoi(argv[1]);
    serverPassword = argv[2]; // サーバーパスワードを設定

    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int yes = 1;
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt SO_REUSEADDR"); return 1;
    }

    set_nonblock(listen_fd);

    sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (::listen(listen_fd, 128) < 0) { perror("listen"); return 1; }

    pfds.reserve(1024);
    pfds.push_back((pollfd){ listen_fd, POLLIN, 0 });

    std::puts("server: listening...");

    for (;;) {
        // ★ poll はここ一か所のみ
        int n = ::poll(&pfds[0], pfds.size(), -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        // pfds を走査
        for (size_t i = 0; i < pfds.size(); ++i) {
            pollfd &p = pfds[i];
            if (p.revents == 0) continue;

            // エラー/切断は即クローズ
            if (p.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                if (p.fd == listen_fd) {
                    std::puts("server: listen socket error");
                    return 1;
                }
                
                // クライアント切断時の後処理
                cleanupClient(p.fd);
                
                // pollfdから除去
                pfds.erase(pfds.begin() + i);
                --i;
                continue;
            }

            if (p.fd == listen_fd) {
                // 新規接続受理（ノンブロッキングなのでループで取れる分だけ取る）
                for (;;) {
                    int cfd = ::accept(listen_fd, 0, 0);
                    if (cfd < 0) {
                        // EAGAIN/EWOULDBLOCK でも再acceptしない（次の poll を待つ）
                        break;
                    }
                    set_nonblock(cfd);
                    pfds.push_back((pollfd){ cfd, POLLIN, 0 });
                     Client::clients[cfd] = Client(cfd);
                    std::printf("accept: fd=%d\n", cfd);
                }
                continue;
            }

            // 送信可能（先に処理）
            if (p.revents & POLLOUT) {
                Client &cl =  Client::clients[p.fd];
                std::printf("POLLOUT: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
                if (!cl.wbuf.empty()) {
                    std::printf("SEND: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
                    ssize_t s = ::send(p.fd, cl.wbuf.data(), cl.wbuf.size(), 0);
                    if (s <= 0) {
                        std::printf("close: fd=%d (send=%zd errno=%d)\n", p.fd, s, errno);
                        ::close(p.fd);
                         Client::clients.erase(p.fd);
                        pfds.erase(pfds.begin() + i);
                        --i;
                        continue;
                    }
                    cl.wbuf.erase(0, (size_t)s);
                    std::printf("SENT: fd=%d, bytes=%zd, remaining=%zu\n", p.fd, s, cl.wbuf.size());
                }
                if ( Client::clients.count(p.fd) &&  Client::clients[p.fd].wbuf.empty()) {
                    pfds[i].events &= ~POLLOUT; // 送り切ったら POLLOUT を外す
                    std::printf("REMOVE POLLOUT: fd=%d\n", p.fd);
                }
            }

            // 受信
            if (p.revents & POLLIN) {
                char buf[4096];
                ssize_t r = ::recv(p.fd, buf, sizeof(buf), 0);
                if (r <= 0) {
                    std::printf("close: fd=%d (recv=%zd errno=%d)\n", p.fd, r, errno);
                    ::close(p.fd);
                     Client::clients.erase(p.fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

                Client &cl =  Client::clients[p.fd];
                cl.rbuf.append(buf, (size_t)r);

                if (line_too_long(cl.rbuf)) {
                    std::printf("close: fd=%d (line too long)\n", p.fd);
                    ::close(p.fd);
                     Client::clients.erase(p.fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

                std::string line;
                while (pop_line(cl.rbuf, line)) {
                    std::printf("RAW LINE: [%s]\n", line.c_str());  // デバック用
                    Message msg;
                    if (!Parser::parse(line, msg)) continue;

                    std::printf("CMD=%s ARGC=%zu\n", msg.cmd.c_str(), msg.args.size());
                    
                    // コマンドディスパッチ
                    dispatchCommand(cl, msg);
                }

                if (!cl.wbuf.empty()) {
                    pfds[i].events |= POLLOUT; // 送信要求あり
                    std::printf("SET POLLOUT: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
                    
                    // 即座に送信を試行
                    std::printf("IMMEDIATE SEND: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
                    ssize_t s = ::send(p.fd, cl.wbuf.data(), cl.wbuf.size(), 0);
                    if (s <= 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            std::printf("SEND would block, keeping POLLOUT\n");
                        } else {
                            std::printf("SEND failed: %s\n", strerror(errno));
                        }
                    } else {
                        std::printf("IMMEDIATE SEND SUCCESS: fd=%d, bytes=%zd\n", p.fd, s);
                        cl.wbuf.erase(0, (size_t)s);
                        if (cl.wbuf.empty()) {
                            pfds[i].events &= ~POLLOUT; // 送信完了
                            std::printf("REMOVE POLLOUT (immediate): fd=%d\n", p.fd);
                            if (cl.logout) {
                                std::printf("Closing link after send: %s@localhost\n", cl.nickname.c_str());
                                ::close(p.fd);
                                Client::clients.erase(p.fd);
                                pfds.erase(pfds.begin() + i);
                                --i;
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }

    // クリーンアップ
    for (size_t i = 0; i < pfds.size(); ++i) ::close(pfds[i].fd);
    return 0;
}