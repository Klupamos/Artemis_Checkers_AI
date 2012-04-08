/*
 *  Client.cpp
 *  Artemis_Checkers_AI
 *
 *  Created by Gregory Klupar on 4/6/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include <iostream>
using std::cout;
using std::endl;

#include <istream>
#include <ostream>

#include <boost/asio.hpp>

#include "Color.h"
#include "MoveGenerator.h"
#include "Client.h"

Client::Client():my_socket(io_service){};

bool Client::connect(std::string ip_address, std::string port){
		
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip_address, port);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
	
    boost::asio::connect(my_socket, iterator);
	
	// receive which player I am
    boost::asio::streambuf response;
    boost::asio::read_until(my_socket, response, "\n");
	
    std::istream response_stream(&response);
	std::string my_role;
    response_stream >> my_role;
	
	cout << "My Role is: " << my_role << endl;
	if (my_role.find("black") != std::string::npos){
		my_color = BLACK;
		return true;
	}
	if (my_role.find("white") != std::string::npos){
		my_color = WHITE;
		return true;
	}
	my_color = SPEC;
	return true;
};

void Client::recv(board & b){
	boost::asio::streambuf response;
    boost::asio::read_until(my_socket, response, "\n");
	
    std::istream response_stream(&response);
	std::string message;
	std::getline(response_stream, message);
//    response_stream >> message;
	cout << "received: " << message << endl;
	
	// Check that response is OK.
	if(message.size() != 32 || message.find_first_not_of("Ww_bB") != std::string::npos){
		cout << "Not a board - finding response!" << endl;
		
		if(message.find("invalid") != std::string::npos){
			// my opponent thinks I cheated
			cout << "You think I cheated!!" << endl;
			exit(0);
		}
		
		if(message.find("black declared itself the winner") != std::string::npos || 
		   message.find("white declared itself the winner") != std::string::npos){
			// The server should have ended the game by this point
			// havn't changed the board yet, so lets inspect it
			moveGenerator mg(my_color, b);
			if (*mg){
				//I can still move
				cout << "\nThats Wrong!" << endl;
				cout << "I can still move." << endl;
				exit(0);
			}else{
				// Your right I can't move
				cout << "My bad your right, Congradulations!" << endl;
				exit(0);
			}
		}
		
		if(message.find(" wins") != std::string::npos){
			
			exit(0);
		}
		
		if (message.find(" declared") != std::string::npos) {
			
			exit(0);
		}
		
		cout << "Could not find response." << endl;
		exit(0);
		
	}else {
		b.fromSeq(message);
	}
};

void Client::send(const board & b){
	//Create the msg 
    boost::asio::streambuf message;
    std::ostream response_stream(&message);

	cout << "Sent:     " << b.toSeq() << endl;
	response_stream << b.toSeq() << "\n" << endl;
	
	// Send the msg.
	boost::asio::write(my_socket, message);
};

void Client::send(const std::string msg){
	//Create the msg 
    boost::asio::streambuf message;
    std::ostream response_stream(&message);
	
	cout << "Sent:     " << msg << endl;
	response_stream << msg << endl;
	
	// Send the msg.
	boost::asio::write(my_socket, message);
};