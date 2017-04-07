#pragma once
#include <RakPeerInterface.h>
#include <unordered_map>
#include "Application.h"
#include <glm/glm.hpp>
#include "GameObject.h"
//struct GameObject
//{
//	glm::vec3 position;
//	glm::vec4 colour;
//};

class Client : public aie::Application {
public:

	Client();
	virtual ~Client();


	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	void sendClientGameObject();

protected:

	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	GameObject m_myGameObject;
	

	std::unordered_map<int, GameObject> m_otherClientGameObjects;

	
	RakNet::RakPeerInterface* m_pPeerInterface;
	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;

	void handleNetworkConnection();
	void initialiseClientConnection();

	void onSetClientIDPacket(RakNet::Packet * packet);

	

	void onReceivedClientDataPacket(RakNet::Packet * packet);

	void handleNetworkMessages();

	float level_Height = 10.0f;
	float level_Width = 10.0f;
	float level_Bottom = -10.0f;
	float level_Right = -10.0f;


	
};