/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 19:20:40 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/25 05:16:27 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
#define COMMAND_HPP

//追加
#include <cstdlib>
#include <sstream>
//
#include "Client.hpp"
#include "Util.hpp"
#include "Channel.hpp"

void handleWhois(Client& client, const Message& msg);
void handleWho(Client& client, const Message& msg);
void handleJoin(Client& client, const Message& msg);
void handleMode(Client& client, const Message& msg);
void handleKick(Client& client, const Message& msg);
void handleInvite(Client& client, const Message& msg);
void handleTopic(Client& client, const Message& msg);
void handlePrivmsg(Client& client, const Message& msg);
void handlePart(Client& client, const Message& msg);

#endif