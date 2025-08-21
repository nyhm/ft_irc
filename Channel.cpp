#include "Channel.hpp"

std::map<std::string, Channel> channels;
std::vector<std::string> channelsToLeave;
// チャンネルを検索
Channel* findChannel(const std::string& name) {
    std::map<std::string, Channel>::iterator it = channels.find(name);
    return (it != channels.end()) ? &(it->second) : NULL;
}



// チャンネルを取得または作成
Channel* getOrCreateChannel(const std::string& name) {
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

// 空のチャンネルを削除
void removeEmptyChannels() {
    std::map<std::string, Channel>::iterator it = channels.begin();
    while (it != channels.end()) {
        if (it->second.isEmpty()) {
            channels.erase(it++);
        } else {
            ++it;
        }
    }
}
bool isValidChannelName(const std::string& channelName) {
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
