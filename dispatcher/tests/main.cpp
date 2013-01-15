/*
	Dispatcher
	Copyright (c) 2012 Russell Bewley

	http://github.com/rbewley4/dispatcher

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file main.cpp 
	\brief Boostrap for unit tests.
*/

#include "vlt.hpp"
#include <iostream>

int main()
{
	RUN_TESTS();
	std::cin.get(); // pause
	return(0);
}
