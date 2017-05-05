#pragma once
#include <glm/glm.hpp>
#include <RakPeerInterface.h>
#include <BitStream.h>
#include "GameMessages.h"
#include <list>
#include <unordered_map>


class Client;

class GameObject
{
public:

	int m_myClientID;
	int health;
	int currentHealth;
	bool dead = false;
	int rotation;		//0 = up, 1 = right, 2 = down, 3 = left
	bool isShooting;
	float timer;
	float respawn_Point_A = 9.0f;
	float respawn_Point_B = 9.0f;
	float respawn_Point_C = -9.0f;
	float respawn_Point_D = -9.0f;

	void snapToBounds(glm::vec3& pos);
	bool isOutOfBounds(glm::vec3& pos);

	float level_Height = 10.0f;
	float level_Width = 10.0f;
	float level_Bottom = -10.0f;
	float level_Right = -10.0f;

	glm::vec3 position;
	glm::vec4 colour;
	glm::vec3 velocity;

	static glm::vec3 directions[];
	
	GameObject();
	virtual ~GameObject();

	static glm::vec4 getColour(int id);
	void updateHealth(RakNet::RakPeerInterface * pPeerInterface, Client* c);

	void Fire();

#ifndef NETWORK_SERVER
	bool updateTranforms(float deltaTime, Client * client);
	void Respawn(Client* client);
	void FireBullet(RakNet::RakPeerInterface* pPeerInterface, int id, int rotation);
#else
	virtual void Update(RakNet::RakPeerInterface* pPeerInterface, float deltaTime);
#endif

	bool isBullet() { return m_myClientID >= 100; }

	void Read(RakNet::Packet* packet);
	void Write(RakNet::RakPeerInterface* pPeerInterface, const RakNet::SystemAddress& address, bool broadcast);

	float radius = 1.0f;

	void Draw();
};


