#pragma once
enum  Client_Message : unsigned char
{
	Join,
	Leave,
	Input,
	
};

enum Server_Message : unsigned char
{
	Join_Result,
	State,
	AsteroidData
};