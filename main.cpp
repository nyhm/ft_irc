#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }
    int port = atoi(argv[1]);

    // ① ソケット作成
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // ② ソケットオプション設定
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        return 1;
    }

    // ③ ソケットアドレス設定
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // 全てのインターフェースから接続を受け入れ
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // ④ リッスン開始
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        return 1;
    }

    // ⑤ poll用構造体準備
    struct pollfd fds[1024];  // 最大1024クライアントまで対応例
    nfds_t nfds = 1;
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    // ⑥ イベントループ
    while (true) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        // 新規接続受付
        if (fds[0].revents & POLLIN) {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }
            // クライアントソケットをnon-blockingに
            fcntl(client_fd, F_SETFL, O_NONBLOCK);

            // poll監視リストに追加
            fds[nfds].fd = client_fd;
            fds[nfds].events = POLLIN;
            nfds++;

            std::cout << "New client connected: fd=" << client_fd << std::endl;
        }

        // ここにクライアントからの読み書き処理を追加していく
    }

    close(server_fd);
    return 0;
}
