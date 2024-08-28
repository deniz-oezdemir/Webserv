/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:28:13 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 15:28:14 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Server
{
  public:
	Server(void);
	Server(const Server &src);
	~Server(void);

	Server &operator=(const Server &rhs);

  protected:

  private:
};
