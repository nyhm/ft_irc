/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 19:20:40 by hnagashi          #+#    #+#             */
/*   Updated: 2025/08/15 18:19:31 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

struct Message {
    std::string cmd;              // 大文字化済み
    std::vector<std::string> args;
};

class Parser {
public:
    // 1行（\r\nは除去済み）を解析。
    // 空行やコマンドなしなら false を返す。
    static bool parse(const std::string &line, Message &out);

private:
    static void to_upper(std::string &s);
};

#endif