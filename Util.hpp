#ifndef UTIL_HPP
#define UTIL_HPP
#include <string>
#include "Client.hpp"
#include "Parser.hpp"

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


#endif