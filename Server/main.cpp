#include <iostream> 
#include <string>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <chrono>
#include <thread>
#include <map>
#include "GameMessages.h"
#include "GameObject.h"

// player gameobjects
std::map<int, GameObject> m_gameObjects;

int nextClientID = 1;
int nextBulletID = 100;

void sendClientPing(RakNet::RakPeerInterface* pPeerInterface)
{
	while (true)
	{
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_TEXT_MESSAGE);
		bs.Write("Ping!");

		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
			RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
	}
}


void updateObjects(RakNet::RakPeerInterface* pPeerInterface)
{
	while (true)
	{
		// foreach bullet, call update
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		for (auto& object : m_gameObjects)
		{
			object.second.Update(pPeerInterface, 0.1f);
			if (object.second.isBullet())
			{
				std::cout << object.second.m_myClientID << ":(" << object.second.position.x << "." << object.second.position.z << ")" << std::endl;
			}
		}

		// Check collisions
		for (auto& object : m_gameObjects)
		{
			for (auto& other : m_gameObjects)
			{
				//Do not collide with self
				if (object.first == other.first)
					continue;

				// Bullets dont collide together
				if ((object.second.isBullet()) == (other.second.isBullet()))
					continue;

				//Check collision between object and other
				float collisionDistance = object.second.radius + other.second.radius;

				float distance = glm::distance(object.second.position, other.second.position);

				if (distance <= collisionDistance)
				{
					//THEY HAVE COLLIDED
					std::cout << "BOOM";
				}
			}
		}

	}
}

void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SET_CLIENT_ID);
	bs.Write(nextClientID);
	nextClientID++;
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);

	for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); it++)
	{		
		GameObject obj = it->second;
		obj.Write(pPeerInterface, address, false);
	}

	// send us to all other clients
	int id = nextClientID - 1;
	GameObject obj;
	obj.position = glm::vec3(0);
	obj.colour = GameObject::getColour(id);
	obj.m_myClientID = id;
	obj.Write(pPeerInterface, address, true);
	m_gameObjects[id] = obj;
}

void sendClientDeath(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress address, int clientID)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_PLAYER_DEAD);
	bs.Write(clientID);
	// TODO: Delete the object here.
	// TODO: Remove this gameobject from the map stored in client and server
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, true);
}

void OnBulletFired(RakNet::RakPeerInterface* pPeerInterface, RakNet::Packet* packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	// read the packet and store in our list of game objects on the server
	int clientID;
	int rotation;
	bs.Read(clientID);
	bs.Read(rotation);

	// now we can spawn a bullet!
	m_gameObjects[nextBulletID].position = m_gameObjects[clientID].position;
	m_gameObjects[nextBulletID].colour = m_gameObjects[clientID].colour;
	m_gameObjects[nextBulletID].velocity = GameObject::directions[rotation];
	m_gameObjects[nextBulletID].m_myClientID = nextBulletID;
	m_gameObjects[nextBulletID].Write(pPeerInterface, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

	nextBulletID++;
}

int main()
{
	const unsigned short PORT = 5456;
	RakNet::RakPeerInterface* pPeerInterface = nullptr;

	//Startup the server, and start it listening to clients
	std::cout << "Starting up the server..." << std::endl;

	//Initialize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();

	//Create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);

	//Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(32, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(32);

	std::thread pingThread(sendClientPing, pPeerInterface);
	std::thread updateThread(updateObjects, pPeerInterface);

	RakNet::Packet* packet = nullptr;
	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet;
			pPeerInterface->DeallocatePacket(packet),
			packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				std::cout << "A connection is incoming.\n";
				sendNewClientID(pPeerInterface, packet->systemAddress);
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "A client has disconnected.\n";
				break;
			case ID_CONNECTION_LOST:
				std::cout << "A client lost the connection.\n";
				break;
			case ID_CLIENT_CLIENT_DATA:
			{
				RakNet::BitStream bs(packet->data, packet->length, false);
				pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

				// read the packet and store in our list of game objects on the server
				GameObject clientData;
				clientData.Read(packet);
				m_gameObjects[clientData.m_myClientID] = clientData;
				break;
			}
			case ID_CLIENT_PLAYER_DEAD:
			{
				RakNet::BitStream bs(packet->data, packet->length, false);
				//pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

				bs.IgnoreBytes(sizeof(RakNet::MessageID));
				// read the packet and store in our list of game objects on the server
				int deadClient;
				bs.Read(deadClient);
				m_gameObjects[deadClient].dead = true;
				std::cout << "RIP to client#: " << deadClient << std::endl;

				// Echo death message to all clients
				sendClientDeath(pPeerInterface, packet->systemAddress, deadClient);

				break;
			}
			
			case ID_CLIENT_FIRE_BULLET:
				OnBulletFired(pPeerInterface, packet);
				break;
			default:
				std::cout << "Received a message with a unknown id: " << packet->data[0];
				break;
			}
		}
	}

	return 0;
}