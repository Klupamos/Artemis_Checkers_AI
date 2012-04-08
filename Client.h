/*
 *  Client.h
 *  Artemis_Checkers_AI
 *
 *  Created by Gregory Klupar on 4/6/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_CLIENT_H_INCLUDED
#define FILE_CLIENT_H_INCLUDED

#include <string>

#include <boost/asio.hpp>

#include "Color.h"
#include "Board.h"

class Client{
private:
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket my_socket;
	
	color_t my_color;
public:
	Client();
	bool connect(std::string, std::string);
	color_t color(){return my_color;}
	void recv(board &);
	void send(const board &);
	void send(const std::string);
};


#endif // FILE_CLIENT_H_INCLUDED