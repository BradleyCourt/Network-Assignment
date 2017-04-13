#include "Client.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <GameMessages.h>
#include "GameObject.h"
using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;


Client::Client() {

}

Client::~Client() {
}

bool Client::startup() {

	srand(time(nullptr));
	setBackgroundColour(0.3141592653589792f, 0.49869563578934584379596f, 0.1943741f);



	//bullets[1] = Bullet(m_otherClientGameObjects[1].position, m_otherClientGameObjects[1].position, m_otherClientGameObjects[1].colour);
	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	m_myGameObject.position = glm::vec3(0, 0, 0);
	m_myGameObject.colour = glm::vec4(1, 0, 0, 1);

	// create simple camera transforms

	m_viewMatrix = glm::lookAt(vec3(0, 10, 0), vec3(0), vec3(0, 0, -1));



	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
		getWindowWidth() / (float)getWindowHeight(),
		0.1f, 1000.f);
	handleNetworkConnection();


	return true;
}

void Client::shutdown() {

	Gizmos::destroy();
}

void Client::update(float deltaTime) {



	// query time since application started
	float time = getTime();

	// wipe the gizmos clean for this frame
	Gizmos::clear();

	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	for (int i = 0; i < 21; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, 10),
			vec3(-10 + i, 0, -10),
			i == 10 ? white : black);
		Gizmos::addLine(vec3(10, 0, -10 + i),
			vec3(-10, 0, -10 + i),
			i == 10 ? white : black);
	}
	m_myGameObject.updateHealth(m_pPeerInterface, this);
	handleNetworkMessages();

	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
	{
		quit();
	}


	for (auto& otherClient : m_otherClientGameObjects)
	{
		otherClient.second.position += otherClient.second.velocity * deltaTime;
	}

	if (m_myGameObject.updateTranforms(deltaTime, this))
	{
		sendClientGameObject();
	}
	m_viewMatrix = glm::lookAt(m_myGameObject.position + vec3(0, 20, 0), m_myGameObject.position, vec3(0, 0, 1));
	//Gizmos::addSphere(m_myGameObject.position, 1.0f, 32, 32, m_myGameObject.colour);
	if (m_myGameObject.position.z > level_Height) // +Z
	{
		m_myGameObject.position.z = level_Height;
	}
	if (m_myGameObject.position.x > level_Width) // +X
	{
		m_myGameObject.position.x = level_Width;
	}
	if (m_myGameObject.position.z < level_Bottom) // -Z
	{
		m_myGameObject.position.z = level_Bottom;
	}
	if (m_myGameObject.position.x < level_Right) // -X
	{
		m_myGameObject.position.x = level_Right;
	}

}

void Client::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// update perspective in case window resized
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
		getWindowWidth() / (float)getWindowHeight(),
		0.1f, 1000.f);

	m_myGameObject.Draw();

	for (auto& otherClient : m_otherClientGameObjects)
	{
		otherClient.second.Draw();
	}

	Gizmos::draw(m_projectionMatrix * m_viewMatrix);
}

void Client::handleNetworkConnection()
{
	//Initialize the Raknet peer interface first
	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	initialiseClientConnection();
}

void Client::initialiseClientConnection()
{
	//Create a socket descriptor to describe this connection
	//No data needed, as we will be connecting to a server
	RakNet::SocketDescriptor sd;

	//Now call startup - max of 1 connections (to the server)
	m_pPeerInterface->Startup(1, &sd, 1);

	std::cout << "Connecting to server at: " << IP << std::endl;

	//Now call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(IP, PORT, nullptr, 0);

	//Finally, check to see if we connected, and if not, throw a error
	if (res != RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		std::cout << "Unable to start connection, Error number: " << res << std::endl;
	}

}


void Client::onSetClientIDPacket(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(m_myGameObject.m_myClientID);

	std::cout << "Set my client ID to: " << m_myGameObject.m_myClientID << std::endl;

	m_myGameObject.colour = GameObject::getColour(m_myGameObject.m_myClientID);
}

void Client::sendClientGameObject()
{
	m_myGameObject.Write(m_pPeerInterface, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::onReceivedClientDataPacket(RakNet::Packet * packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	int clientID;
	bsIn.Read(clientID);

	//If the clientID does not match our ID, we need to update
	//our client GameObject information.
	if (clientID != m_myGameObject.m_myClientID)
	{
		GameObject clientData;
		clientData.Read(packet);

		// preserve any dead variables on our map
		//if (m_otherClientGameObjects.find(clientID) != m_otherClientGameObjects.end())
		//	clientData.dead = m_otherClientGameObjects[clientID].dead;

		m_otherClientGameObjects[clientID] = clientData;

		//For now, just output the Game Object information to the console

		std::cout << "Client" << clientID << " at : " << clientData.position.x << " " << clientData.position.z << std::endl;

	}
}

void Client::handleNetworkMessages()
{
	RakNet::Packet* packet;
	for (packet = m_pPeerInterface->Receive(); packet;
		m_pPeerInterface->DeallocatePacket(packet),
		packet = m_pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			std::cout << "Another client has disconnected.\n";
			break;
		case ID_REMOTE_CONNECTION_LOST:
			std::cout << "Another client has lost the connection.\n";
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			std::cout << "Another client has connected.\n";
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Our connection request has been accepted.\n";
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			std::cout << "The server is full.\n";
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "We have been disconnected.\n";
			break;
		case ID_CONNECTION_LOST:
			std::cout << "Connection lost.\n";
			break;
		case ID_SERVER_PLAYER_DEAD:
		{
			std::cout << "A player has died\n";
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			int deadID = -1;
			bsIn.Read(deadID);
			auto& iter = m_otherClientGameObjects.find(deadID);
			if (iter != m_otherClientGameObjects.end())
			{
				GameObject* ob = &(iter->second);
				if (ob)
					ob->dead = true;
			}
		}
		break;
		case ID_SERVER_SET_CLIENT_ID:
			onSetClientIDPacket(packet);
			break;
		case ID_CLIENT_CLIENT_DATA:
			onReceivedClientDataPacket(packet);
			break;
		case ID_SERVER_TEXT_MESSAGE:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			RakNet::RakString str;
			bsIn.Read(str);
			std::cout << str.C_String() << std::endl;
			break;
		}

		default:
			std::cout << "Received a message with a unknown id: " << packet->data[0];
			break;
		}

	}

}



