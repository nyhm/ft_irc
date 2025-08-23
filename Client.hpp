/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kanahash <kanahash@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 19:20:40 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/22 20:10:08 by kanahash         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <map>
#include <unistd.h>
//追加
#include <cstdio>
//
#include "Parser.hpp"


// チャンネルメンバーの役割
enum ChannelRole {
    ROLE_NORMAL = 0,
    ROLE_OPERATOR = 1,
    ROLE_VOICE = 2
};

class Client{
    public:
    int fd;
    std::string rbuf; // 受信バッファ
    std::string wbuf; // 送信キュー
    bool registered;  // 登録済みフラグ
    std::string nickname; // ニックネーム
    std::string username; // ユーザー名
    std::string realname; // 実名
    bool passOk;     // パスワード確認済みフラグ
    bool capDone;
    bool logout;
    
    Client(int f = -1) : fd(f), registered(false), passOk(false) ,capDone(false),logout(false) {}
    // クライアント管理（グローバル変数）
    static std::map<int, Client> clients;
    typedef void (*CommandHandler)(Client&, const Message&);
    // 登録完了チェック関数
    void checkRegistrationComplete();
    

};
bool isValidNickname(const std::string& nickname);
void handleNick(Client& client, const Message& msg);
void handleUser(Client& client, const Message& msg);
void handlePass(Client& client, const Message& msg);
void handleQuit(Client& client, const Message& msg);
void cleanupClient(int clientFd);
#endif