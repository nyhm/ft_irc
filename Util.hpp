#ifndef UTIL_HPP
#define UTIL_HPP
#include <string>
#include <map>
#include <set>
#include <poll.h>
#include "Client.hpp"
#include "Parser.hpp"

// pollfd配列（グローバル変数）
extern std::vector<pollfd> pfds;
// 数値応答のフォーマット統一関数
void sendNumeric(Client& client, int code, const std::string& target, const std::string& message) ;
// ウェルカムメッセージ送信関数（順序を修正）
void sendWelcome(Client& client);




// ERR_NEEDMOREPARAMS (461) 汎用関数
void sendNeedMoreParams(Client& client, const std::string& command);

// ERR_NOTREGISTERED (451) 汎用関数
void sendNotRegistered(Client& client, const std::string&) ;
// ERR_NOTONCHANNEL (442) 汎用関数
void sendNotonchannel(Client& client, const std::string& channelName) ;

// サーバーパスワード（グローバル変数）
extern std::string serverPassword;



// 登録が必要なコマンドのセット（初期化）
extern std::set<std::string> requiresRegistrationInit;
extern std::map<std::string, Client::CommandHandler> commandHandlersInit;

// コマンドディスパッチテーブル
extern std::set<std::string>& requiresRegistration;
extern std::map<std::string, Client::CommandHandler>& commandHandlers;

// プレフィックス生成関数
std::string prefix(const Client& client);

// 送信要求を設定する関数
void setPollout(int fd) ;
void handleUnknown(Client& client, const Message& msg);

#endif