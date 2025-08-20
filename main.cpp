// build: c++ -Wall -Wextra -Werror -std=c++98 -o irc_min irc_min.cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <map>
#include <set>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <iostream> //デバック用?
#include "Parser.hpp"

struct Client {
    int fd;
    std::string rbuf; // 受信バッファ
    std::string wbuf; // 送信キュー
    bool registered;  // 登録済みフラグ
    std::string nickname; // ニックネーム
    std::string username; // ユーザー名
    std::string realname; // 実名
    bool passOk;     // パスワード確認済みフラグ
    
    Client(int f = -1) : fd(f), registered(false), passOk(false) {}
};

// チャンネルメンバーの役割
enum ChannelRole {
    ROLE_NORMAL = 0,
    ROLE_OPERATOR = 1,
    ROLE_VOICE = 2
};

struct Channel {
    std::string name;                    // チャンネル名
    std::string topic;                   // トピック
    std::map<std::string, ChannelRole> members; // メンバー (nick -> role)
    std::set<std::string> modes;         // チャンネルモード (+i, +t, +k, +l)
    std::set<std::string> invited;       // 招待されたユーザー
    std::string key;                     // チャンネルキー
    int limit;                           // ユーザー制限
    time_t creationTime;                 // 作成時刻
    time_t topicTime;                    // 作成時刻
    
    Channel(const std::string& n = "") : name(n), limit(0) {}
    
    // メンバー追加
    void addMember(const std::string& nick, ChannelRole role = ROLE_NORMAL) {
        members[nick] = role;
    }
    
    // メンバー削除
    void removeMember(const std::string& nick) {
        members.erase(nick);
    }
    
    // メンバー数取得
    size_t getMemberCount() const {
        return members.size();
    }
    
    // 空かどうかチェック
    bool isEmpty() const {
        return members.empty();
    }
    
    // メンバーが存在するかチェック
    bool hasMember(const std::string& nick) const {
        return members.find(nick) != members.end();
    }
    
    // メンバーの役割を取得
    ChannelRole getRole(const std::string& nick) const {
        std::map<std::string, ChannelRole>::const_iterator it = members.find(nick);
        return (it != members.end()) ? it->second : ROLE_NORMAL;
    }
    
    // メンバーの役割を設定
    void setRole(const std::string& nick, ChannelRole role) {
        if (hasMember(nick)) {
            members[nick] = role;
        }
    }
};

// コマンドハンドラの型定義
typedef void (*CommandHandler)(Client&, const Message&);

// サーバーパスワード（グローバル変数）
std::string serverPassword;

// 登録が必要なコマンドのセット（初期化）
static std::set<std::string> requiresRegistrationInit;
static std::map<std::string, CommandHandler> commandHandlersInit;

// コマンドディスパッチテーブル
static const std::set<std::string>& requiresRegistration = requiresRegistrationInit;
static const std::map<std::string, CommandHandler>& commandHandlers = commandHandlersInit;

// チャンネル管理（Serverクラスの概念）
static std::map<std::string, Channel> channels;

// クライアント管理（グローバル変数）
static std::map<int, Client> clients;

// pollfd配列（グローバル変数）
static std::vector<pollfd> pfds;

// チャンネルを検索
static Channel* findChannel(const std::string& name) {
    std::map<std::string, Channel>::iterator it = channels.find(name);
    return (it != channels.end()) ? &(it->second) : NULL;
}

// チャンネルを取得または作成
static Channel* getOrCreateChannel(const std::string& name) {
    Channel* channel = findChannel(name);
    if (!channel) {
        // 新規チャンネルを作成
        channels[name] = Channel(name);
        channel = &channels[name];
        channel->creationTime= time(NULL);
        channel->modes.insert("n");
        channel->modes.insert("t");
    }
    return channel;
}

// 送信要求を設定する関数
static void setPollout(int fd) {
    for (size_t i = 0; i < pfds.size(); ++i) {
        if (pfds[i].fd == fd) {
            pfds[i].events |= POLLOUT;
            std::printf("SETPOLLOUT: fd=%d, events=0x%x\n", fd, pfds[i].events);
            break;
        }
    }
}

// 空のチャンネルを削除
static void removeEmptyChannels() {
    std::map<std::string, Channel>::iterator it = channels.begin();
    while (it != channels.end()) {
        if (it->second.isEmpty()) {
            channels.erase(it++);
        } else {
            ++it;
        }
    }
}

// 基本レスポンスユーティリティ

// プレフィックス生成関数
static std::string prefix(const Client& client) {
    if (client.nickname.empty() || client.username.empty()) {
        return ":server";
    }
    return ":" + client.nickname + "!" + client.username + "@localhost";
}

// チャンネル全員へ送るブロードキャスト関数（後で実装）
/*
static void broadcast(std::map<int, Client>& clients, const std::string& msg, int exceptFd = -1) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->first != exceptFd) {
            it->second.wbuf += msg;
        }
    }
}
*/

// 数値応答のフォーマット統一関数
static void sendNumeric(Client& client, int code, const std::string& target, const std::string& message) {
    char codeStr[4];
    std::snprintf(codeStr, sizeof(codeStr), "%03d", code);
    std::string response = ":server " + std::string(codeStr) + " " + target + " " + message + "\r\n";
    client.wbuf += response;
}


// ERR_NEEDMOREPARAMS (461) 汎用関数
static void sendNeedMoreParams(Client& client, const std::string& command) {
    sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, command + " :Not enough parameters.");
}

// ERR_NOTREGISTERED (451) 汎用関数
static void sendNotRegistered(Client& client, const std::string&) {
    sendNumeric(client, 451, "*", "You have not registered");
}

// ウェルカムメッセージ送信関数（順序を修正）
static void sendWelcome(Client& client) {
    std::string nick = client.nickname.empty() ? "*" : client.nickname;
    
    // 001: Welcome message
    sendNumeric(client, 001, nick, "Welcome to the Internet Relay Network " + nick);
    
    // 002: Your host
    sendNumeric(client, 002, nick, "Your host is server, running version 1.0");
    
    // 003: Server creation date
    sendNumeric(client, 003, nick, "This server was created today");
    
    // 004: Server info
    sendNumeric(client, 004, nick, "server 1.0 o o");
    
    // MOTD（順序を修正）
    sendNumeric(client, 375, nick, ":- server Message of the day -");
    sendNumeric(client, 372, nick, ":- Welcome to ft_irc server!");
    sendNumeric(client, 376, nick, ":End of /MOTD command.");
}

// 文字列長・ニック/チャンネル名バリデーション関数
static bool isValidNickname(const std::string& nickname) {
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

static bool isValidChannelName(const std::string& channelName) {
    if (channelName.empty() || channelName.length() > 50) {
        return false;
    }
    
    // チャンネル名は # で始まる
    if (channelName[0] != '#') {
        return false;
    }
    
    // チャンネル名に空白、制御文字、カンマを含まない
    for (size_t i = 0; i < channelName.length(); ++i) {
        if (channelName[i] == ' ' || channelName[i] == ',' || 
            (channelName[i] >= 0 && channelName[i] <= 31)) {
            return false;
        }
    }
    
    return true;
}

// 登録完了チェック関数
static void checkRegistrationComplete(Client& client) {
    if (!client.registered && client.passOk && !client.nickname.empty() && !client.username.empty()) {
        client.registered = true;
        sendWelcome(client);
    }
}

// コマンドハンドラ関数
static void handlePass(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "PASS :Not enough parameters");
        return;
    }
    
    // パスワードチェック（サーバーパスワードと比較）
    if (msg.args[0] == serverPassword) {
        client.passOk = true;
        checkRegistrationComplete(client);
    } else {
        // パスワード不一致で464エラーを送信して切断
        sendNumeric(client, 464, client.nickname.empty() ? "*" : client.nickname, ":Password incorrect");
        // 切断フラグを設定（後で処理）
        client.wbuf += "ERROR :Password incorrect\r\n";
    }
}

static void handleJoin(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "JOIN :Not enough parameters");
        return;
    }
    
    std::string channelName = msg.args[0];
    
    // チャンネル名の妥当性チェック
    if (!isValidChannelName(channelName)) {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :Invalid channel name");
        return;
    }
    
    // チャンネルを取得または作成
    Channel* channel = getOrCreateChannel(channelName);
    
    // すでにチャンネルに在室しているかチェック
    if (channel->hasMember(client.nickname)) {
        sendNumeric(client, 443, client.nickname, channelName + " :You are already on that channel");
        return;
    }
    
    // +i モード（招待制）チェック
    if (channel->modes.find("i") != channel->modes.end()) {
        if (channel->invited.find(client.nickname) == channel->invited.end()) {
            sendNumeric(client, 473, client.nickname, channelName + " :Cannot join channel (+i)");
            return;
        }
    }
    
    // +k モード（キー認証）チェック
    if (channel->modes.find("k") != channel->modes.end()) {
        if (msg.args.size() < 2 || msg.args[1] != channel->key) {
            sendNumeric(client, 475, client.nickname, channelName + " :Cannot join channel (incorrect channel key)");
            return;
        }
    }
    
    // +l モード（人数制限）チェック
    if (channel->modes.find("l") != channel->modes.end()) {
        if (static_cast<size_t>(channel->limit) <= channel->getMemberCount()) {
            sendNumeric(client, 471, client.nickname, channelName + " :Cannot join channel (+l)");
            return;
        }
    }
    
    // チャンネルに参加
    if (channel->isEmpty()) {
        // 最初の参加者はオペレーター
        channel->addMember(client.nickname, ROLE_OPERATOR);
    } else {
        // 通常の参加者
        channel->addMember(client.nickname, ROLE_NORMAL);
    }
    
    // チャンネル参加メッセージをブロードキャスト
    std::string joinMsg = prefix(client) + " JOIN :" + channelName + "\r\n";
    
            // チャンネル内の全メンバーに送信
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (channel->hasMember(it->second.nickname)) {
                it->second.wbuf += joinMsg;
                setPollout(it->first);
            }
        }
    
    // 成功時の応答（順序を修正）
    
    // RPL_TOPIC (332) - チャンネルトピック
    if (!channel->topic.empty()) {
        sendNumeric(client, 332, client.nickname, channelName + " :" + channel->topic);
        sendNumeric(client, 333, client.nickname, channelName + " " + client.nickname + "!" + client.username + "@localhost" + " " + std::to_string(channel->topicTime));
    } 
    
    // RPL_NAMREPLY (353) - チャンネルメンバーリスト
    std::string memberList = "= " + channelName + " :";
    for (std::map<std::string, ChannelRole>::iterator it = channel->members.begin(); it != channel->members.end(); ++it) {
        if (it != channel->members.begin()) {
            memberList += " ";
        }
        if (it->second == ROLE_OPERATOR) {
            memberList += "@";
        } else if (it->second == ROLE_VOICE) {
            memberList += "+";
        }
        memberList += it->first;
    }
    sendNumeric(client, 353, client.nickname, memberList);
    
    // RPL_ENDOFNAMES (366) - メンバーリスト終了
    sendNumeric(client, 366, client.nickname, channelName + " :End of /NAMES list.");
    
    // デバッグ出力
    std::printf("JOIN: %s joined %s (role=%d, members=%zu)\n", 
                client.nickname.c_str(), channelName.c_str(), 
                channel->getRole(client.nickname), channel->getMemberCount());
}

// チャンネルモード設定用のテスト関数（後で使用）
/*
static void setChannelMode(const std::string& channelName, const std::string& mode, const std::string& param = "") {
    Channel* channel = findChannel(channelName);
    if (!channel) return;
    
    if (mode == "+i") {
        channel->modes.insert("+i");
    } else if (mode == "+k") {
        channel->modes.insert("+k");
        channel->key = param;
    } else if (mode == "+l") {
        channel->modes.insert("+l");
        channel->limit = std::atoi(param.c_str());
    } else if (mode == "+t") {
        channel->modes.insert("+t");
    }
}
*/

static void handleMode(Client& client, const Message& msg) {
    
    std::string target = msg.args[0];
    
    // チャンネルモードの場合
    if (target[0] == '#') {
        Channel* channel = findChannel(target);
        if (!channel) {
            sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, target + " :No such channel");
            return;
        }
        
        // チャンネルのメンバーかチェック
        if (!channel->hasMember(client.nickname)) {
            sendNumeric(client, 442, client.nickname.empty() ? "*" : client.nickname, target + " :You're not on that channel");
            return;
        }
        
        // オペレーター権限チェック（一部のモード変更には必要）
        ChannelRole role = channel->getRole(client.nickname);
        bool isOperator = (role == ROLE_OPERATOR);
        
        std::string modes = (msg.args.size() > 1) ? msg.args[1] : "";
        std::string modeArgs = (msg.args.size() > 2) ? msg.args[2] : "";
        
        // モード文字列を解析
        bool adding = true;
        std::string response = "";
        if(modes!=""){//コマンドがあったら
        for (size_t i = 0; i < modes.length(); ++i) {
            char mode = modes[i];
            
            if (mode == '+') {
                adding = true;
                continue;
            } else if (mode == '-') {
                adding = false;
                continue;
            }
            
            switch (mode) {
                case 'i': // invite-only
                    if (adding) {
                        channel->modes.insert("i");
                        response += "+i ";
                    } else {
                        channel->modes.erase("i");
                        response += "-i ";
                    }
                    break;
                    
                case 't': // topic protection
                    if (adding) {
                        channel->modes.insert("t");
                        response += "+t ";
                    } else {
                        channel->modes.erase("t");
                        response += "-t ";
                    }
                    break;
                    
                case 'k': // key
                    if (adding) {
                        if (modeArgs.empty()) {
                            sendNumeric(client, 461, client.nickname, "MODE +k :Not enough parameters");
                            return;
                        }
                        channel->key = modeArgs;
                        channel->modes.insert("k");
                        response += "+k " + modeArgs + " ";
                    } else {
                        if (channel->key == modeArgs || modeArgs.empty()) {
                            channel->key = "";
                            channel->modes.erase("k");
                            response += "-k ";
                        } else {
                            sendNumeric(client, 467, client.nickname, target + " :Channel key already set");
                            return;
                        }
                    }
                    break;
                    
                case 'l': // limit
                    if (adding) {
                        if (modeArgs.empty()) {
                            sendNumeric(client, 461, client.nickname, "MODE +l :Not enough parameters");
                            return;
                        }
                        int limit = std::atoi(modeArgs.c_str());
                        if (limit <= 0) {
                            sendNumeric(client, 467, client.nickname, target + " :Invalid limit");
                            return;
                        }
                        channel->limit = limit;
                        channel->modes.insert("l");
                        response += "+l " + modeArgs + " ";
                    } else {
                        channel->limit = 0;
                        channel->modes.erase("l");
                        response += "-l ";
                    }
                    break;
                    
                case 'o': // operator
                    if (!isOperator) {
                        sendNumeric(client, 482, client.nickname, target + " :You must be a channel op or higher to change the topic.");
                        return;
                    }
                    
                    if (modeArgs.empty()) {
                        sendNumeric(client, 461, client.nickname, "MODE +o :Not enough parameters");
                        return;
                    }
                    
                    // 対象ユーザーがチャンネルにいるかチェック
                    if (!channel->hasMember(modeArgs)) {
                        sendNumeric(client, 441, client.nickname, modeArgs + " " + target + " :They aren't on that channel");
                        return;
                    }
                    
                    if (adding) {
                        channel->setRole(modeArgs, ROLE_OPERATOR);
                        response += "+o " + modeArgs + " ";
                    } else {
                        channel->setRole(modeArgs, ROLE_NORMAL);
                        response += "-o " + modeArgs + " ";
                    }
                    break;
                    
                default:
                    // 未対応のモードは無視
                    break;
            }
        }
        if (!response.empty()) {
            // チャンネル内の全メンバーにMODE変更通知を送信
            std::string modeMsg = prefix(client) + " MODE " + target + " " + response + "\r\n";
            
            for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
                if (channel->hasMember(it->second.nickname)) {
                    it->second.wbuf += modeMsg;
                }
            }
            
            std::printf("MODE: %s changed modes for %s: %s\n", 
                        client.nickname.c_str(), target.c_str(), response.c_str());
        }
        }else{
            //何もなかったらチャンネル内の情報表示
            std::string modeStr = "+";
            for (std::set<std::string>::const_iterator it = channel->modes.begin();
                it != channel->modes.end(); ++it) {
                modeStr += *it;  // 例: "t" "n" があれば "+tn"
            }

            // 数値応答を作成
            std::string reply = ":irc.example.com 324 " 
                            + client.nickname + " "
                            + channel->name + " "
                            + modeStr + "\r\n"
                            +":irc.example.com 329 " 
                            + client.nickname + " "
                            + channel->name + " "
                            + std::to_string(channel->creationTime) + "\r\n";

            client.wbuf += reply;
        }
        
    } else {
        // ユーザーモード（未実装）
        sendNumeric(client, 502, client.nickname, target + " :User modes not supported");
    }
}

static void handleKick(Client& client, const Message& msg) {
    if (msg.args.size() < 2) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "KICK :Not enough parameters");
        return;
    }
    
    std::string channelName = msg.args[0];
    std::string targetNick = msg.args[1];
    std::string comment = (msg.args.size() > 2) ? msg.args[2] : "No reason";
    
    // チャンネル名の検証
    if (channelName[0] != '#') {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :Invalid channel name");
        return;
    }
    
    // チャンネルの存在確認
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :No such channel");
        return;
    }
    
    // チャンネルのメンバーかチェック
    if (!channel->hasMember(client.nickname)) {
        sendNumeric(client, 442, client.nickname.empty() ? "*" : client.nickname, channelName + " :You're not on that channel");
        return;
    }
    
    // オペレーター権限チェック
    ChannelRole role = channel->getRole(client.nickname);
    if (role != ROLE_OPERATOR) {
        sendNumeric(client, 482, client.nickname, channelName + " :You're not channel operator");
        return;
    }
    
    // 対象ユーザーがチャンネルにいるかチェック
    if (!channel->hasMember(targetNick)) {
        sendNumeric(client, 441, client.nickname, targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }
    
    // 自分自身をKICKしようとしている場合
    if (client.nickname == targetNick) {
        sendNumeric(client, 482, client.nickname, channelName + " :You cannot kick yourself");
        return;
    }
    
    // チャンネル内の全メンバーにKICK通知を送信
    std::string kickMsg = prefix(client) + " KICK " + channelName + " " + targetNick + " :" + comment + "\r\n";
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (channel->hasMember(it->second.nickname)) {
            it->second.wbuf += kickMsg;
        }
    }
    
    // 対象ユーザーをチャンネルから削除
    channel->removeMember(targetNick);
    
    // チャンネルが空になった場合の処理
    if (channel->isEmpty()) {
        channels.erase(channelName);
        std::printf("KICK: %s removed from %s, channel removed (empty)\n", 
                    targetNick.c_str(), channelName.c_str());
    } else {
        std::printf("KICK: %s removed from %s by %s, remaining members=%zu\n", 
                    targetNick.c_str(), channelName.c_str(), client.nickname.c_str(), channel->getMemberCount());
    }
}

static void handleInvite(Client& client, const Message& msg) {
    if (msg.args.size() < 2) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "INVITE :Not enough parameters");
        return;
    }
    
    std::string targetNick = msg.args[0];
    std::string channelName = msg.args[1];
    
    // チャンネル名の検証
    if (channelName[0] != '#') {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :Invalid channel name");
        return;
    }
    
    // チャンネルの存在確認
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :No such channel");
        return;
    }
    
    // チャンネルのメンバーかチェック
    if (!channel->hasMember(client.nickname)) {
        sendNumeric(client, 442, client.nickname.empty() ? "*" : client.nickname, channelName + " :You're not on that channel");
        return;
    }
    
    // オペレーター権限チェック（+iモードが設定されている場合）
    if (channel->modes.find("i") != channel->modes.end()) {
        ChannelRole role = channel->getRole(client.nickname);
        if (role != ROLE_OPERATOR) {
            sendNumeric(client, 482, client.nickname, channelName + " :You're not channel operator");
            return;
        }
    }
    
    // 対象ユーザーの存在確認
    bool targetFound = false;
    int targetFd = -1;
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.nickname == targetNick) {
            targetFound = true;
            targetFd = it->first;
            break;
        }
    }
    
    if (!targetFound) {
        sendNumeric(client, 401, client.nickname.empty() ? "*" : client.nickname, targetNick + " :No such nick/channel");
        return;
    }
    
    // 対象ユーザーが既にチャンネルにいるかチェック
    if (channel->hasMember(targetNick)) {
        sendNumeric(client, 443, client.nickname, targetNick + " " + channelName + " :is already on channel");
        return;
    }
    
    // 招待を記録
    channel->invited.insert(targetNick);
    
    // 対象ユーザーにINVITE通知を送信
    std::string inviteMsg = prefix(client) + " INVITE " + targetNick + " :" + channelName + "\r\n";
    clients[targetFd].wbuf += inviteMsg;
    
    // 送り主に341確認を送信
    sendNumeric(client, 341, client.nickname, targetNick + " " + channelName);
    
    std::printf("INVITE: %s invited %s to %s\n", 
                client.nickname.c_str(), targetNick.c_str(), channelName.c_str());
}

static void handleTopic(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "TOPIC :Not enough parameters");
        return;
    }
    
    std::string channelName = msg.args[0];
    
    // チャンネル名の検証
    if (channelName[0] != '#') {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :Invalid channel name");
        return;
    }
    
    // チャンネルの存在確認
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, channelName + " :No such channel");
        return;
    }
    
    // チャンネルのメンバーかチェック
    if (!channel->hasMember(client.nickname)) {
        sendNumeric(client, 442, client.nickname.empty() ? "*" : client.nickname, channelName + " :You're not on that channel");
        return;
    }
    
    // トピックの取得（引数が1つの場合）
    if (msg.args.size() == 1) {
        if (channel->topic.empty()) {
            // トピック未設定
            sendNumeric(client, 331, client.nickname, channelName + " :No topic is set");
        } else {
            // トピック設定済み
            sendNumeric(client, 332, client.nickname, channelName + " :" + channel->topic);
            sendNumeric(client, 333, client.nickname, channelName + " " + client.nickname + "!" + client.username + " " + std::to_string(channel->topicTime));
        }
        return;
    }
    
    // トピックの設定（引数が2つ以上の場合）
    std::string newTopic = msg.args[1];
    
    // +tモードが設定されている場合、オペレーター権限が必要
    if (channel->modes.find("t") != channel->modes.end()) {
        ChannelRole role = channel->getRole(client.nickname);
        if (role != ROLE_OPERATOR) {
            sendNumeric(client, 482, client.nickname, channelName + " :You're not channel operator");
            return;
        }
    }
    
    // トピックを設定
    channel->topic = newTopic;
    channel->topicTime = time(NULL);
    
    // チャンネル内の全メンバーにTOPIC通知を送信
    std::string topicMsg = prefix(client) + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    
    // チャンネル内の全メンバーに送信（自分を含む）
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.nickname == client.nickname || channel->hasMember(it->second.nickname)) {
            it->second.wbuf += topicMsg;
        }
    }
    
    std::printf("TOPIC: %s set topic for %s: %s\n", 
                client.nickname.c_str(), channelName.c_str(), newTopic.c_str());
}

static void handlePrivmsg(Client& client, const Message& msg) {
    if (msg.args.size() < 2) {
        sendNumeric(client, 411, client.nickname.empty() ? "*" : client.nickname, "No recipient given (PRIVMSG)");
        return;
    }
    
    std::string target = msg.args[0];
    std::string message = msg.args[1];
    
    // メッセージが空の場合
    if (message.empty()) {
        sendNumeric(client, 412, client.nickname.empty() ? "*" : client.nickname, "No text to send");
        return;
    }
    
    // チャンネル宛の場合
    if (target[0] == '#') {
        Channel* channel = findChannel(target);
        if (!channel) {
            sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, target + " :No such channel");
            return;
        }
        
        // チャンネルのメンバーかチェック
        if (!channel->hasMember(client.nickname)) {
            sendNumeric(client, 442, client.nickname.empty() ? "*" : client.nickname, target + " :You're not on that channel");
            return;
        }
        
        // チャンネル内の全メンバーにブロードキャスト（自分除外）
        std::string privmsgMsg = prefix(client) + " PRIVMSG " + target + " :" + message + "\r\n";
        
        // チャンネル内の全メンバーに送信
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->first != client.fd && channel->hasMember(it->second.nickname)) {
                it->second.wbuf += privmsgMsg;
                std::printf("PRIVMSG: sending to fd=%d, nickname=%s, wbuf.size=%zu\n", 
                           it->first, it->second.nickname.c_str(), it->second.wbuf.size());
                setPollout(it->first);
            }
        }
        
        std::printf("PRIVMSG: %s sent to channel %s: %s\n", 
                    client.nickname.c_str(), target.c_str(), message.c_str());
        
    } else {
        // ニックネーム宛の場合
        // 相手のクライアントを検索
        bool found = false;
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.nickname == target) {
                // 相手にメッセージを送信
                std::string privmsgMsg = prefix(client) + " PRIVMSG " + target + " :" + message + "\r\n";
                it->second.wbuf += privmsgMsg;
                std::printf("PRIVMSG: sending to fd=%d, nickname=%s, wbuf.size=%zu\n", 
                           it->first, it->second.nickname.c_str(), it->second.wbuf.size());
                setPollout(it->first);
                found = true;
                break;
            }
        }
        
        if (!found) {
            sendNumeric(client, 401, client.nickname.empty() ? "*" : client.nickname, target + " :No such nick/channel");
            return;
        }
        
        std::printf("PRIVMSG: %s sent to %s: %s\n", 
                    client.nickname.c_str(), target.c_str(), message.c_str());
    }
}

static void handlePart(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNeedMoreParams(client, "PART");
        return;
    }
    
    std::string channelName = msg.args[0];
    
    // チャンネル名の妥当性チェック
    if (channelName.empty() || channelName[0] != '#') {
        sendNumeric(client, 403, client.nickname, channelName + " :Invalid channel name");
        return;
    }
    
    // チャンネルを検索
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendNumeric(client, 403, client.nickname, channelName + " :No such channel");
        return;
    }
    
    // チャンネルのメンバーかチェック
    if (!channel->hasMember(client.nickname)) {
        sendNumeric(client, 442, client.nickname, channelName + " :You're not on that channel");
        return;
    }
    
    // 退出メッセージを準備
    std::string partMsg;
    if (msg.args.size() > 1) {
        // カスタムメッセージがある場合
        partMsg = prefix(client) + " PART " + channelName + " :" + msg.args[1] + "\r\n";
    } else {
        // デフォルトメッセージ
        partMsg = prefix(client) + " PART " + channelName + " :Leaving\r\n";
    }
    
            // チャンネル内の全メンバーに送信
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (channel->hasMember(it->second.nickname)) {
                it->second.wbuf += partMsg;
                setPollout(it->first);
            }
        }
    
    // チャンネルから退出
    channel->removeMember(client.nickname);
    
    // 空のチャンネルを削除
    if (channel->isEmpty()) {
        removeEmptyChannels();
        std::printf("PART: %s left %s, channel removed (empty)\n", 
                    client.nickname.c_str(), channelName.c_str());
    } else {
        std::printf("PART: %s left %s (remaining members=%zu)\n", 
                    client.nickname.c_str(), channelName.c_str(), 
                    channel->getMemberCount());
    }
}

static void handleNick(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 431, client.nickname.empty() ? "*" : client.nickname, ":No nickname given");
        return;
    }
    
    std::string newNick = msg.args[0];
    
    // ニックネームの妥当性チェック
    if (!isValidNickname(newNick)) {
        sendNumeric(client, 432, client.nickname.empty() ? "*" : client.nickname, newNick + " :Erroneous nickname");
        return;
    }
    
    // ニックネームの重複チェック
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.nickname == newNick) {
            sendNumeric(client, 433, client.nickname.empty() ? "*" : client.nickname, newNick + " :Nickname is already in use");
            return;
        }
    }
    
    // ニックネームを設定
    client.nickname = newNick;
    
    // 登録完了チェック
    checkRegistrationComplete(client);
    
    // 登録済みの場合のみ応答を送信
    if (client.registered) {
        // ニックネーム変更の確認
        std::string nickMsg = prefix(client) + " NICK :" + newNick + "\r\n";
        client.wbuf += nickMsg;
    }
}

static void handleUser(Client& client, const Message& msg) {
    if (msg.args.size() < 4) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "USER :Not enough parameters");
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
    
    // 登録完了チェック
    checkRegistrationComplete(client);
    
    // 登録済みの場合のみ応答を送信
    if (client.registered) {
        // ユーザー登録の確認
        std::string userMsg = prefix(client) + " USER " + client.username + " 0 * :" + client.realname + "\r\n";
        client.wbuf += userMsg;
    }
}

static void handlePing(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 409, client.nickname.empty() ? "*" : client.nickname, ":No origin specified");
        return;
    }
    
    std::string token = msg.args[0];
    
    // PONGレスポンスを送信
    std::string pongMsg = ":server PONG server :" + token + "\r\n";
    client.wbuf += pongMsg;
    
    std::printf("PING: %s pinged with token '%s', sent PONG response\n", 
                client.nickname.empty() ? "unknown" : client.nickname.c_str(), token.c_str());
}

static void handleQuit(Client& client, const Message& msg) {
    std::string reason = (msg.args.size() > 0) ? msg.args[0] : "Leaving";
    
    // 全在室チャンネルへQUITをブロードキャスト
    std::string quitMsg = prefix(client) + " QUIT :" + reason + "\r\n";
    
    // クライアントが在室している全チャンネルを検索
    std::vector<std::string> channelsToLeave;
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
            for (std::map<int, Client>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt) {
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
    
    std::printf("QUIT: %s disconnected from %zu channels\n", 
                client.nickname.c_str(), channelsToLeave.size());
}

static void handleUnknown(Client& client, const Message& msg) {
    sendNumeric(client, 421, client.nickname, msg.cmd  + " :Unknown command");
        return;
}

// クライアント切断時の後処理関数
static void cleanupClient(int clientFd) {
    std::map<int, Client>::iterator it = clients.find(clientFd);
    if (it == clients.end()) {
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
            for (std::map<int, Client>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt) {
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
    clients.erase(clientFd);
}

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
    std::map<std::string, CommandHandler>::const_iterator it = commandHandlers.find(msg.cmd);
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
                    clients[cfd] = Client(cfd);
                    std::printf("accept: fd=%d\n", cfd);
                }
                continue;
            }

            // 送信可能（先に処理）
            if (p.revents & POLLOUT) {
                Client &cl = clients[p.fd];
                std::printf("POLLOUT: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
                if (!cl.wbuf.empty()) {
                    std::printf("SEND: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
                    ssize_t s = ::send(p.fd, cl.wbuf.data(), cl.wbuf.size(), 0);
                    if (s <= 0) {
                        std::printf("close: fd=%d (send=%zd errno=%d)\n", p.fd, s, errno);
                        ::close(p.fd);
                        clients.erase(p.fd);
                        pfds.erase(pfds.begin() + i);
                        --i;
                        continue;
                    }
                    cl.wbuf.erase(0, (size_t)s);
                    std::printf("SENT: fd=%d, bytes=%zd, remaining=%zu\n", p.fd, s, cl.wbuf.size());
                }
                if (clients.count(p.fd) && clients[p.fd].wbuf.empty()) {
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
                    clients.erase(p.fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

                Client &cl = clients[p.fd];
                cl.rbuf.append(buf, (size_t)r);

                if (line_too_long(cl.rbuf)) {
                    std::printf("close: fd=%d (line too long)\n", p.fd);
                    ::close(p.fd);
                    clients.erase(p.fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

                std::string line;
                while (pop_line(cl.rbuf, line)) {
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