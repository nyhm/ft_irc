#include "Util.hpp"


// 数値応答のフォーマット統一関数
void sendNumeric(Client& client, int code, const std::string& target, const std::string& message) {
    char codeStr[4];
    std::snprintf(codeStr, sizeof(codeStr), "%03d", code);
    std::string response = ":server " + std::string(codeStr) + " " + target + " " + message + "\r\n";
    client.wbuf += response;
}

// ウェルカムメッセージ送信関数（順序を修正）
void sendWelcome(Client& client) {
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
    sendNumeric(client, 372, nick, ":███████╗ ████████╗        ██████╗ ██████╗  ██████╗ ");
    sendNumeric(client, 372, nick, ":██╔════╝ ╚══██╔══╝          ██╔═╝ ██╔══██╗██╔════╝ ");
    sendNumeric(client, 372, nick, ":███████╗    ██║             ██║   ██████╔╝██║      ");
    sendNumeric(client, 372, nick, ":██╔════╝    ██║             ██║   ██╔══██╗██║      ");
    sendNumeric(client, 372, nick, ":██║         ██║ ████████╗ ██████╗ ██║  ██║╚██████╗ ");
    sendNumeric(client, 372, nick, ":╚═╝         ╚═╝ ╚═══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ");
    sendNumeric(client, 372, nick, ":- Welcome to ft_irc server!");
    sendNumeric(client, 376, nick, ":End of /MOTD command.");
}





// ERR_NEEDMOREPARAMS (461) 汎用関数
void sendNeedMoreParams(Client& client, const std::string& command) {
    sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, command + " :Not enough parameters.");
}

// ERR_NOTREGISTERED (451) 汎用関数
void sendNotRegistered(Client& client, const std::string&) {
    sendNumeric(client, 451, "*", "You have not registered");
}
// ERR_NOTONCHANNEL (442) 汎用関数
void sendNotonchannel(Client& client, const std::string& channelName) {
    sendNumeric(client, 442, client.nickname, channelName + " :You're not on that channel");
}
