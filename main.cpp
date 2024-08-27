/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 14:19:25 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/27 14:28:16 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "colors.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	try {
		std::cout << "hello" << std::endl;
	} catch (std::exception &e) {
		std::cerr << RED << e.what() << RESET << std::endl;
	}
	return 0;
}
