#pragma once
enum  Client_Message : unsigned char
{
	Join,
	Leave,
	Input,
	PlayerData
	
};

enum Server_Message : unsigned char
{
	Join_Result,
	State,
	AsteroidData
};