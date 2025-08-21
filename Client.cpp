/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 07:18:23 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/21 08:07:06 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Client.hpp"
#include "Util.hpp"

std::map<int, Client> Client::clients;
// 登録完了チェック関数
void Client::checkRegistrationComplete() {
    if (!registered && passOk && !nickname.empty() && !username.empty()&&capDone!=true) {
        registered = true;
        sendWelcome(*this);
    }
    else if (!registered &&
        passOk &&
        !nickname.empty() &&
        !username.empty() &&
        capDone) // CAP END受信後にtrueにする
    {
        std::printf("ログイン\r\n");
        registered = true;

        // 001〜004 を返す
        wbuf += ":irc.example.com 001 " + nickname + " :Welcome to the IRC network " + nickname + "\r\n";
        wbuf += ":irc.example.com 002 " + nickname + " :Your host is irc.example.com, running version 1.0\r\n";
        wbuf += ":irc.example.com 003 " + nickname + " :This server was created just now\r\n";
        wbuf += ":irc.example.com 004 " + nickname + " irc.example.com 1.0 o o\r\n";

        // MOTD (おまけ)
        wbuf += ":irc.example.com 375 " + nickname + " :- irc.example.com Message of the Day -\r\n";
        wbuf += ":irc.example.com 372 " + nickname + " :- Hello world!\r\n";
        wbuf += ":irc.example.com 376 " + nickname + " :End of /MOTD command.\r\n";

        std::printf("REGISTERED: %s\n", nickname.c_str());
    }
}
