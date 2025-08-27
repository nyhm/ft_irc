#include "Command.hpp"

void handleWho(Client &client, const Message &msg) {
    std::string target = msg.args.empty() ? "" : msg.args[0];
    for (std::map<int, Client>::iterator it = Client::clients.begin(); it != Client::clients.end(); ++it) {
        Client &cl = it->second;
        // とりあえず全員返す（チャンネルで絞るのは後回しでも可）
        sendNumeric(client, 352, client.nickname,
            target + " " + cl.username + " " + "localhost" + " " +
            "server" + " " + cl.nickname + " H :0 " + cl.realname);
    }
    sendNumeric(client, 315, client.nickname, target + " :End of WHO list");
}

void handleWhois(Client &client, const Message &msg) {
    if (msg.args.empty()) {
        sendNumeric(client, 431, client.nickname, ":No nickname given"); // ERR_NONICKNAMEGIVEN
        return;
    }
    std::string nick = msg.args[0];
    Client *target = Client::findClientByNick(nick);
    if (!target) {
        sendNumeric(client, 401, client.nickname, nick + " :No such nick");
        return;
    }
    sendNumeric(client, 311, client.nickname,
        target->nickname + " " + target->username + " localhost * :" + target->realname);
    sendNumeric(client, 318, client.nickname, target->nickname + " :End of WHOIS list");
}



 void handleJoin(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "JOIN :Not enough parameters.");
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
        for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
            if (channel->hasMember(it->second.nickname)) {
                it->second.wbuf += joinMsg;
                setPollout(it->first);
            }
        }
    
    // 成功時の応答（順序を修正）
    
	//変更
	if (!channel->topic.empty()) {
    sendNumeric(client, 332, client.nickname, channelName + " :" + channel->topic);

    std::stringstream ss;
    ss << channel->topicTime;
    std::string topicTimeString = ss.str();

    sendNumeric(client, 333, client.nickname, channelName + " " + client.nickname + "!" + client.username + "@localhost" + " " + topicTimeString);
}

// 

    
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


 void handleMode(Client& client, const Message& msg) {
    
    std::string target = msg.args[0];
    
    // チャンネルモードの場合
    if (target[0] == '#') {
        Channel* channel = findChannel(target);
        if (!channel) {
            sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, target + " :No such channel");
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
                            sendNumeric(client, 461, client.nickname, "MODE +k :Not enough parameters.");
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
                            sendNumeric(client, 461, client.nickname, "MODE +l :Not enough parameters.");
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
                        sendNumeric(client, 461, client.nickname, "MODE +o :Not enough parameters.");
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
            
            for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
                if (channel->hasMember(it->second.nickname)) {
                    it->second.wbuf += modeMsg;
                    setPollout(it->first);
                }
            }
            
            std::printf("MODE: %s changed modes for %s: %s\n", 
                        client.nickname.c_str(), target.c_str(), response.c_str());
        }
        }else { //変更
    // nothing to display if no modes were set
    std::string modeStr = "+";
    for (std::set<std::string>::const_iterator it = channel->modes.begin();
         it != channel->modes.end(); ++it) {
        modeStr += *it;  // example: "t" "n" becomes "+tn"
    }

    // create the numeric replies
    std::stringstream ss;
    ss << channel->creationTime;
    std::string creationTimeString = ss.str();

    std::string reply = ":irc.example.com 324 "
                    + client.nickname + " "
                    + channel->name + " "
                    + modeStr + "\r\n"
                    +":irc.example.com 329 "
                    + client.nickname + " "
                    + channel->name + " "
                    + creationTimeString + "\r\n";

    client.wbuf += reply;
}

//

}else {
        // ユーザーモード（未実装）
        sendNumeric(client, 502, client.nickname, target + " :User modes not supported");
    }
}

 void handleKick(Client& client, const Message& msg) {
    if (msg.args.size() < 2) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "KICK :Not enough parameters.");
        return;
    }
    
    std::string channelName = msg.args[0];
    std::string targetNick = msg.args[1];
    std::string comment = (msg.args.size() > 2) ? msg.args[2] : client.nickname;
    
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
        sendNotonchannel(client,channelName);
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
    
    // チャンネル内の全メンバーにKICK通知を送信
    std::string kickMsg = prefix(client) + " KICK " + channelName + " " + targetNick + " :" + comment + "\r\n";
    
    for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
        if (channel->hasMember(it->second.nickname)) {
            it->second.wbuf += kickMsg;
            setPollout(it->first);
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

 void handleInvite(Client& client, const Message& msg) {
    if (msg.args.size() < 2) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "INVITE :Not enough parameters.");
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
        sendNotonchannel(client,channelName);
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
    for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
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
     Client::clients[targetFd].wbuf += inviteMsg;
    
    // 送り主に341確認を送信
    sendNumeric(client, 341, client.nickname, targetNick + " " + channelName);
    
    std::printf("INVITE: %s invited %s to %s\n", 
                client.nickname.c_str(), targetNick.c_str(), channelName.c_str());
}

 void handleTopic(Client& client, const Message& msg) {
    if (msg.args.size() < 1) {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "TOPIC :Not enough parameters.");
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
    
	//変更
	if (msg.args.size() == 1) {
        if (channel->topic.empty()) {
            // トピック未設定
            sendNumeric(client, 331, client.nickname, channelName + " :No topic is set");
        } else {
            // トピック設定済み
            sendNumeric(client, 332, client.nickname, channelName + " :" + channel->topic);

            std::stringstream ss;
            ss << channel->topicTime;
            std::string topicTimeString = ss.str();

            sendNumeric(client, 333, client.nickname, channelName + " " + client.nickname + "!" + client.username + " " + topicTimeString);
        }
        return;
    }
	//

    // チャンネルのメンバーかチェック
    if (!channel->hasMember(client.nickname)) {
        sendNumeric(client, 442, client.nickname.empty() ? "*" : client.nickname, channelName + " :You're not on that channel");
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
    for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
        if (it->second.nickname == client.nickname || channel->hasMember(it->second.nickname)) {
            it->second.wbuf += topicMsg;
            setPollout(it->first);
        }
    }
    
    std::printf("TOPIC: %s set topic for %s: %s\n", 
                client.nickname.c_str(), channelName.c_str(), newTopic.c_str());
    
}


 void handlePrivmsg(Client& client, const Message& msg) {
    if (msg.args.size() < 2) {
        sendNeedMoreParams(client, msg.cmd);
        return;
    }
    
    std::string target = msg.args[0];
    std::string message = msg.args[1];
    
    // チャンネル宛の場合
    if (target[0] == '#') {
        Channel* channel = findChannel(target);
        if (!channel) {
            sendNumeric(client, 403, client.nickname.empty() ? "*" : client.nickname, target + " :No such channel");
            return;
        }
        
        // メンバー外404
        if (!channel->hasMember(client.nickname)) {
            sendNumeric(client, 404, client.nickname.empty() ? "*" : client.nickname, target + " :You cannot send external messages to this channel whilst the +n (noextmsg) mode is set.");
            return;
        }
        
        // チャンネル内の全メンバーにブロードキャスト（自分除外）
        std::string privmsgMsg = prefix(client) + " PRIVMSG " + target + " :" + message + "\r\n";
        
        // チャンネル内の全メンバーに送信
        for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
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
        for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
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

 void handlePart(Client& client, const Message& msg) {
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
        sendNotonchannel(client,channelName);
        return;
    }
    
    // 退出メッセージを準備
    std::string partMsg;
    if (msg.args.size() > 1) {
        // カスタムメッセージがある場合
        partMsg = prefix(client) + " PART " + channelName + " :"+'"' + msg.args[1]+'"' + "\r\n";
    } else {
        // デフォルトメッセージ
        partMsg = prefix(client) + " PART " + channelName + " :Leaving\r\n";
    }
    
            // チャンネル内の全メンバーに送信
        for (std::map<int, Client>::iterator it =  Client::clients.begin(); it !=  Client::clients.end(); ++it) {
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
