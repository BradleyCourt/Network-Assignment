#pragma once
#include "GameObject.h"
#include <glm/glm.hpp>
#include "Input.h"

class Bullet : public GameObject
{
public:

	int maxBullets;
	float fireInterval = 0.5;



	
	Bullet();
	Bullet(glm::vec3 pos, glm::vec3 v, glm::vec4 c);
	~Bullet();
	void Fire();

#ifdef NETWORK_SERVER
	void Update(RakNet::RakPeerInterface* pPeerInterface);
#endif
};

