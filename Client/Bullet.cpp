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
