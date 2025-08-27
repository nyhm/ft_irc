/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 07:18:23 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/21 10:39:46 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Client.hpp"
#include "Util.hpp"
#include "Channel.hpp"

std::map<int, Client> Client::clients;
Client* Client::findClientByNick(const std::string& nick) {
    for (std::map<int, Client>::iterator it = Client::clients.begin();
         it != Client::clients.end(); ++it) {
        if (it->second.nickname == nick) {
            return &it->second;
        }
    }
    return NULL; // 見つからなければ nullptr
}

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
        sendWelcome(*this);
        std::printf("REGISTERED: %s\n", nickname.c_str());
    }
}


// コマンドハンドラ関数
void handlePass(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "PASS :Not enough parameters.");
        return;
    }
    
    // パスワードチェック（サーバーパスワードと比較）
    if (msg.args[0] == serverPassword) {
        client.passOk = true;
        client.checkRegistrationComplete();
    } else {
        // 切断フラグを設定（後で処理）
        std::string nick = client.nickname.empty() ? "*" : client.nickname;

    client.wbuf += "ERROR :Closing link: (" 
                 + nick + "@localhost) [Bad password]\r\n";
        client.logout=true;
    }
}

// 文字列長・ニック/チャンネル名バリデーション関数
bool isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }
    
    // 最初の文字は文字または特殊文字
    if (!isalpha(nickname[0]) && nickname[0] != '[' && nickname[0] != ']' && 
        nickname[0] != '\\' && nickname[0] != '`' && nickname[0] != '_' && 
        nickname[0] != '^' && nickname[0] != '{' && nickname[0] != '}' && 
        nickname[0] != '|') {
        return false;
    }
    
    // 残りの文字は文字、数字、特殊文字
    for (size_t i = 1; i < nickname.length(); ++i) {
        if (!isalnum(nickname[i]) && nickname[i] != '[' && nickname[i] != ']' && 
            nickname[i] != '\\' && nickname[i] != '`' && nickname[i] != '_' && 
            nickname[i] != '^' && nickname[i] != '{' && nickname[i] != '}' && 
            nickname[i] != '|' && nickname[i] != '-') {
            return false;
        }
    }
    
    return true;
}




void handleNick(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "nick :Not enough parameters.");
        return;
    }
    
    std::string newNick = msg.args[0];
    
    // ニックネームの妥当性チェック
    if (!isValidNickname(newNick)) {
        sendNumeric(client, 432, client.nickname.empty() ? "*" : client.nickname, newNick + " :Erroneous nickname");
        return;
    }
    
    // ニックネームの重複チェック
    for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
        if (it->second.nickname == newNick) {
            sendNumeric(client, 433, client.nickname.empty() ? "*" : client.nickname, newNick + " :Nickname is already in use");
            return;
        }
    }
    
    // ニックネームを設定
    client.nickname = newNick;
    if(!client.username.empty()&&client.passOk==false){
            client.wbuf += "ERROR :Closing link: ("+client.nickname+"@localhost"+") [You are not allowed to connect to this server]\r\n";
        client.logout=true;
    }
    // 登録完了チェック
    client.checkRegistrationComplete();
    
    // 登録済みの場合のみ応答を送信
    if (client.passOk&&client.registered) {
        // ニックネーム変更の確認
        std::string nickMsg = prefix(client) + " NICK :" + newNick + "\r\n";
        client.wbuf += nickMsg;
    }
}

void handleUser(Client& client, const Message& msg) {
    if (msg.args.size() < 4) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "USER :Not enough parameters.");
        return;
    }
    
    // 既に登録済みの場合
    if (client.registered) {
        sendNumeric(client, 462, client.nickname, ":You may not reregister");
        return;
    }
    
    // ユーザー名とリアルネームを設定
    client.username = msg.args[0];
    client.realname = msg.args[3];
    if(!client.nickname.empty()&&client.passOk==false){
            client.wbuf += "ERROR :Closing link: ("+client.nickname+"@localhost"+") [You are not allowed to connect to this server]\r\n";
         client.logout=true;
    }
    
    // 登録完了チェック
    client.checkRegistrationComplete();
    // 登録済みの場合のみ応答を送信
    if (client.passOk&&client.registered) {
        // ニックネーム変更の確認
        std::string userMsg = prefix(client) + " USER :" + std::string(client.username) + "\r\n";
        client.wbuf += userMsg;
        
    }
}


void handleQuit(Client& client, const Message& msg) {
    std::string reason = (msg.args.size() > 0) ? msg.args[0] : "Leaving";
    
    // 全在室チャンネルへQUITをブロードキャスト
    std::string quitMsg = prefix(client) + " QUIT :" + reason + "\r\n";
    
    
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->second.hasMember(client.nickname)) {
            channelsToLeave.push_back(it->first);
        }
    }
    
    // 各チャンネルから退出処理
    for (std::vector<std::string>::iterator it = channelsToLeave.begin(); it != channelsToLeave.end(); ++it) {
        std::string channelName = *it;
        Channel* channel = findChannel(channelName);
        
        if (channel) {
            // チャンネル内の全メンバーにQUIT通知を送信
            for (std::map<int, Client>::iterator clientIt =  Client::clients.begin(); clientIt !=  Client::clients.end(); ++clientIt) {
                if (channel->hasMember(clientIt->second.nickname)) {
                    clientIt->second.wbuf += quitMsg;
                    std::printf("QUIT: sending notification to fd=%d, nickname=%s\n", 
                               clientIt->first, clientIt->second.nickname.c_str());
                    setPollout(clientIt->first);
                }
            }
            
            // チャンネルからメンバーを削除
            channel->removeMember(client.nickname);
            
            // 招待セットからも削除
            channel->invited.erase(client.nickname);
            
            std::printf("QUIT: %s left %s, remaining members=%zu\n", 
                        client.nickname.c_str(), channelName.c_str(), channel->getMemberCount());
            
            // チャンネルが空になった場合の処理
            if (channel->isEmpty()) {
                channels.erase(channelName);
                std::printf("QUIT: %s removed (empty)\n", channelName.c_str());
            }
        }
    }
    close(client.fd);  
     Client::clients.erase(client.fd);
    std::printf("QUIT: %s disconnected from %zu channels\n", 
                client.nickname.c_str(), channelsToLeave.size());
}


// クライアント切断時の後処理関数
void cleanupClient(int clientFd) {
    std::map<int, Client>::iterator it =  Client::clients.find(clientFd);
    if (it ==  Client::clients.end()) {
        return;
    }
    
    Client& client = it->second;
    
    // 全在室チャンネルへQUITをブロードキャスト
    std::string quitMsg = prefix(client) + " QUIT :Connection lost\r\n";
    
    // クライアントが在室している全チャンネルを検索
    std::vector<std::string> channelsToLeave;
    for (std::map<std::string, Channel>::iterator channelIt = channels.begin(); channelIt != channels.end(); ++channelIt) {
        if (channelIt->second.hasMember(client.nickname)) {
            channelsToLeave.push_back(channelIt->first);
        }
    }
    
    // 各チャンネルから退出処理
    for (std::vector<std::string>::iterator channelIt = channelsToLeave.begin(); channelIt != channelsToLeave.end(); ++channelIt) {
        std::string channelName = *channelIt;
        Channel* channel = findChannel(channelName);
        
        if (channel) {
            // チャンネル内の全メンバーにQUIT通知を送信
            for (std::map<int, Client>::iterator clientIt =  Client::clients.begin(); clientIt !=  Client::clients.end(); ++clientIt) {
                if (clientIt->first != clientFd && channel->hasMember(clientIt->second.nickname)) {
                    clientIt->second.wbuf += quitMsg;
                    std::printf("DISCONNECT: sending notification to fd=%d, nickname=%s\n", 
                               clientIt->first, clientIt->second.nickname.c_str());
                    setPollout(clientIt->first);
                }
            }
            
            // チャンネルからメンバーを削除
            channel->removeMember(client.nickname);
            
            // 招待セットからも削除
            channel->invited.erase(client.nickname);
            
            std::printf("DISCONNECT: %s left %s, remaining members=%zu\n", 
                        client.nickname.c_str(), channelName.c_str(), channel->getMemberCount());
            
            // チャンネルが空になった場合の処理
            if (channel->isEmpty()) {
                channels.erase(channelName);
                std::printf("DISCONNECT: %s removed (empty)\n", channelName.c_str());
            }
        }
    }
    
    std::printf("DISCONNECT: %s cleaned up from %zu channels\n", 
                client.nickname.c_str(), channelsToLeave.size());
    
    // クライアントを削除
     Client::clients.erase(clientFd);
}
