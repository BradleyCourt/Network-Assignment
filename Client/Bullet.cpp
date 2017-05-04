#include "Bullet.h"



Bullet::Bullet()
{
}

Bullet::Bullet(glm::vec3 pos, glm::vec3 v, glm::vec4 c)
{
	position = pos;
	velocity = v;
	colour = c;
}


Bullet::~Bullet()
{

}

void Bullet::Fire()
{
	//aie::Input* input = aie::Input::getInstance();

	
}

#ifdef NETWORK_SERVER
void sendClientDeath(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress address, int clientID);

void Bullet::Update(RakNet::RakPeerInterface* pPeerInterface)
{
	for (auto& otherClient : m_otherClientGameObjects)
	{
		otherClient.second.position += otherClient.second.velocity * deltaTime;
	}
	// check collisions
	if (isOutOfBounds(position))
	{
		sendClientDeath(pPeerInterface, RakNet::UNASSIGNED_SYSTEM_ADDRESS, this->m_myClientID);
	}
}
#endif
