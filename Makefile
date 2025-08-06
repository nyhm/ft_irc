# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/06 09:18:03 by hnagashi          #+#    #+#              #
#    Updated: 2025/08/06 09:22:05 by hnagashi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

src = main.cpp
obj = $(src:.cpp=.o)
name = ft_irc

all: $(name)
$(name): $(obj)
	$(CXX) $(CXXFLAGS) -o $@ $^
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(obj)
fclean: clean
	rm -f $(name)
re: fclean all
.PHONY: all clean fclean re