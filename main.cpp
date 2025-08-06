#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>

#define MAX_CLIENTS 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);

    // ソケット作成
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    std::cout << "socket created" << std::endl;

    // ソケットオプション設定
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        return 1;
    }
    std::cout << "socket option set" << std::endl;

    // ソケットアドレス設定
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    std::cout << "socket bind successful" << std::endl;

    // リッスン開始
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        return 1;
    }
    std::cout << "listening on port " << port << std::endl;

    // poll 構造体の初期化
    struct pollfd fds[MAX_CLIENTS];
    nfds_t nfds = 1;
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    std::cout << "poll setup complete" << std::endl;

    // イベントループ
    while (true) {
        int ret = poll(fds, nfds, -1); // -1: 無制限に待つ
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

            // 非ブロッキングに設定
            if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
                perror("fcntl");
                close(client_fd);
                continue;
            }

            if (nfds >= MAX_CLIENTS) {
                std::cerr << "Too many clients" << std::endl;
                close(client_fd);
                continue;
            }

            fds[nfds].fd = client_fd;
            fds[nfds].events = POLLIN;
            nfds++;

            std::cout << "New client connected: fd=" << client_fd << std::endl;
        }

        // クライアントからの読み取りと応答処理
        char buffer[1024];
        for (nfds_t i = 1; i < nfds; ++i) {
            if (fds[i].revents & POLLIN) {
                ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes <= 0) {
                    std::cout << "Client disconnected: fd=" << fds[i].fd << std::endl;
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1]; // 最後の要素を上書き
                    nfds--;
                    i--;
                    continue;
                }

                buffer[bytes] = '\0';
                std::cout << "Received from fd=" << fds[i].fd << ": " << buffer;

                if (send(fds[i].fd, buffer, bytes, 0) < 0) {
                    perror("send");
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
