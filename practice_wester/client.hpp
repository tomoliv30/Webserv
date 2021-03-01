/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: wester <wester@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/02/16 11:42:36 by wester        #+#    #+#                 */
/*   Updated: 2021/02/19 14:08:34 by wester        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "address.hpp"
# include "fd_files.hpp"
# include <iostream>

class fd_files;

class client
{
    int newfd;
    struct sockaddr_in their_addr;
    socklen_t addr_size;
    // fd_files fd;
    
    client();
    char*       cutStr(char* msg, int num);
    
  public:
    client(int sockfd, fd_files& fd);
    client(const client& other);
    client& operator=(const client& other);
    ~client();
    
};

#endif