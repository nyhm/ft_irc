/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 09:41:49 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/21 09:55:36 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <string>
#include <set>
#include <map>
#include "Client.hpp"

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

// チャンネル管理（Serverクラスの概念）
extern std::map<std::string, Channel> channels;
// クライアントが在室している全チャンネルを検索
extern std::vector<std::string> channelsToLeave;
// チャンネルを検索
Channel* findChannel(const std::string& name) ;

// チャンネルを取得または作成
Channel* getOrCreateChannel(const std::string& name);

// 空のチャンネルを削除
void removeEmptyChannels() ;
bool isValidChannelName(const std::string& channelName) ;
#endif