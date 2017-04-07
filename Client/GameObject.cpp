#include "GameObject.h"
#ifndef NETWORK_SERVER
#include "Input.h"
#include "../bootstrap/Gizmos.h"
#endif
#include "Client.h"
#include <time.h>
#include <iostream>
#include <chrono>

GameObject::GameObject()
{
	health = 100;
	currentHealth = health;
	timer = 0.0f;
}

GameObject::~GameObject() {
}

glm::vec4 colours[] = {
	glm::vec4(0.5, 0.5, 0.5, 1), // Grey
	glm::vec4(1, 0, 0, 1), // red
	glm::vec4(0, 1, 0, 1), // green
	glm::vec4(0, 0, 1, 1), // blue
	glm::vec4(1, 1, 0, 1), // yellow
	glm::vec4(1, 0, 1, 1), // magenta
	glm::vec4(0, 1, 1, 1), // cyan
	glm::vec4(0, 0, 0, 1), // black
};

glm::vec4 GameObject::getColour(int id)
{
	return colours[(id - 1) & 7];
}

void GameObject::updateHealth(RakNet::RakPeerInterface * pPeerInterface, Client* c)
{
	if (currentHealth != health)
	{
		//currentHealth = health;
		std::cout << (currentHealth) << std::endl;
	}
	if (currentHealth <= 0 && !dead)
	{
		//std::cout << "you are very dead" << std::endl;
		dead = true;

		std::cout << "I'm Dead \n";

		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID) GameMessages::ID_CLIENT_PLAYER_DEAD);
		bs.Write(m_myClientID);
		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		//c->quit();
	}
	

}
#ifndef NETWORK_SERVER

bool GameObject::updateTranforms(float deltaTime, Client* client)
{
	aie::Input* input = aie::Input::getInstance();

	bool changed = false;
	if (!dead && changed == false)
	{
		if (input->isKeyDown(aie::INPUT_KEY_LEFT))
		{
			position.x += 1.0f * deltaTime;
			rotation = 3;
			currentHealth--;
			changed = true;
		}

		if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
		{
			position.x -= 1.0f * deltaTime;
			rotation = 1;
			changed = true;
		}

		if (input->isKeyDown(aie::INPUT_KEY_UP))
		{
			position.z += 1.0f * deltaTime;
			rotation = 0;
			changed = true;
		}

		if (input->isKeyDown(aie::INPUT_KEY_DOWN))
		{
			position.z -= 1.0f * deltaTime;
			rotation = 2;
			changed = true;
		}
		
	
	}
	if (dead == true)
	{
		

		// if a second has elapsed since the last position change, do another one!
		static float timeTillNextShuffle = 0;
		timeTillNextShuffle -= deltaTime;

		if (timeTillNextShuffle <= 0)
		{
			int value = (rand() % 4) + 1;
			timeTillNextShuffle = 0.2f;

			if (value == 1)
			{
				position.x = 5.0f;
				position.z = 5.0f;
				//	float respawn_Point_A = 9.0f;
			}

			if (value == 2)
			{
				position.x = -5.0f;
				position.z = 5.0f;
				//float respawn_Point_B = 9.0f;

			}
			if (value == 3)
			{
				position.x = 5.0f;
				position.z = -5.0f;
				//float respawn_Point_C = -9.0f;

			}
			if (value == 4)
			{
				position.x = -5.0f;
				position.z = -5.0f;
				//float respawn_Point_D = -9.0f;
			}
		}
		Respawn(client);
	
	}
	return changed;
}

void GameObject::Respawn(Client* client)
{
	if (dead == true)
	{
		timer++;
	}
	if (timer >= 100)
	{
		//respawn player
		std::cout << "get respawned" << std::endl;
		currentHealth = 100;
		dead = false;
		timer = 0.0f;
		

		// send a message to the server
		client->sendClientGameObject();
	}
}
#endif

void GameObject::Read(RakNet::Packet * packet) // send
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	bsIn.Read(m_myClientID);
	bsIn.Read((char*)&position, sizeof(glm::vec3));
	bsIn.Read((char*)&colour, sizeof(glm::vec4));
	bsIn.Read((char*)&rotation, sizeof(int));
	bsIn.Read((char*)&currentHealth, sizeof(int));

	// set the dead state based on health?
	dead = (currentHealth <= 0);
}

void GameObject::Write(RakNet::RakPeerInterface * pPeerInterface, const RakNet::SystemAddress & address, bool broadcast) //recieve
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID) GameMessages::ID_CLIENT_CLIENT_DATA);
	bs.Write(m_myClientID);
	bs.Write((char*)&position, sizeof(glm::vec3));
	bs.Write((char*)&colour, sizeof(glm::vec4));
	bs.Write((char*)&rotation, sizeof(int));
	bs.Write((char*)&currentHealth, sizeof(int));
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, broadcast);
}

#ifndef NETWORK_SERVER
void GameObject::Draw()
{
	if (!dead)
	{
		aie::Gizmos::addSphere(position, 1.0f, 32, 32, colour);

		glm::vec3 rotationDir = glm::vec3(0);
		if (rotation == 0) { rotationDir = glm::vec3(0, 0, 10); };
		if (rotation == 1) { rotationDir = glm::vec3(-10, 0, 0); };
		if (rotation == 2) { rotationDir = glm::vec3(0, 0, -10); };
		if (rotation == 3) { rotationDir = glm::vec3(10, 0, 0); };
		// does not change the rotation of the sphere, edit transforms for that

		aie::Gizmos::addLine(position, (position + (rotationDir)), glm::vec4(1));
		//aie::Gizmos::addSphere(position, 0.5f, 64, 64, colour);
	}
	//else
	//{
	//	Respawn();
	//}

}
#endif
