#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <time.h>
#include <chrono>

#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

#define DEBUG 0

#if DEBUG
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif

const float gravity{9.8};

struct Animation
{
	sf::Texture& texture;
	unsigned int frames;
	bool textureLoaded;
};

class Background
{
private:
	sf::Sprite m_backgroundImage;
	float m_speed{ 0 };
	float m_position{};

public:
	Background(sf::Texture& texture)
	{
		texture.setRepeated(true);
		m_backgroundImage.setTexture(texture);
	}

	void draw(sf::RenderTexture& texture)
	{
		texture.draw(m_backgroundImage);
	}

	void tick()
	{
		m_speed += 0.001;
		m_position += m_speed;

		unsigned int
			left{ (unsigned int)m_position },
			up{ 0 },
			width{ m_backgroundImage.getTexture()->getSize().x },
			height{ m_backgroundImage.getTexture()->getSize().x };
		m_backgroundImage.setTextureRect(sf::IntRect(left, up, width, height));
	}
};

class AnimationComponent
{
private:
	Animation *m_currentAnimation;
	sf::Sprite m_sprite;
	unsigned int m_currentFrame;

	sf::IntRect getRect()
	{
		unsigned int tileSize{ m_currentAnimation->texture.getSize().x / m_currentAnimation->frames };
		unsigned int
			left{ m_currentFrame * tileSize },
			up{ 0 },
			width{ tileSize },
			height{ m_currentAnimation->texture.getSize().y };

		LOG(m_currentFrame);

		return sf::IntRect(left, up, width, height);
	}

public:
	AnimationComponent(Animation *startAnim) : m_currentAnimation{ startAnim }, m_sprite{}, m_currentFrame{0}
	{
		if (m_currentAnimation->textureLoaded)
		{
			m_sprite.setTexture(startAnim->texture);
			m_sprite.setTextureRect(getRect());
		}
	}

	void update()
	{
		if (m_currentFrame >= ( m_currentAnimation->frames - 1 ))
		{
			m_currentFrame = 0;
		}
		else
		{
			++m_currentFrame;
		}

		if (m_currentAnimation->textureLoaded)
		{
			m_sprite.setTextureRect(getRect());
		}
	}

	void playAnimation(Animation *newAnim)
	{
		m_currentAnimation = newAnim;
		m_currentFrame = 0;
		if (m_currentAnimation->textureLoaded)
		{
			m_sprite.setTextureRect(getRect());
		}
	}

	sf::Sprite& getSprite()
	{
		return m_sprite;
	}
};

class Collision
{
private:
	sf::Vector2f m_relativeLocation{};
	sf::Vector2f m_size{};
	sf::Vertex m_lines[8];
	bool m_isKill{ false };

public:
	Collision() = default;

	void setupCollision(sf::Vector2f size, sf::Vector2f relativeLocation, sf::Vector2f parentLocation)
	{
		m_relativeLocation = relativeLocation;
		m_size = size;
		updateCollision(parentLocation);
	}

	void setRelativeLocation(sf::Vector2f newRelativeLocation)
	{
		m_relativeLocation = newRelativeLocation;
	}

	void drawCollision(sf::RenderTexture& texture)
	{
		texture.draw(m_lines, 8, sf::Lines);
	}

	void updateCollision(sf::Vector2f parentLocation)
	{
		sf::Vector2f tempLocation{parentLocation + m_relativeLocation};

		m_lines[0].position = sf::Vector2f(tempLocation.x, tempLocation.y);
		m_lines[1].position = sf::Vector2f(tempLocation.x + m_size.x, tempLocation.y);

		m_lines[2].position = sf::Vector2f(tempLocation.x + m_size.x, tempLocation.y);
		m_lines[3].position = sf::Vector2f(tempLocation.x + m_size.x, tempLocation.y + m_size.y);

		m_lines[4].position = sf::Vector2f(tempLocation.x + m_size.x, tempLocation.y + m_size.y);
		m_lines[5].position = sf::Vector2f(tempLocation.x, tempLocation.y + m_size.y);

		m_lines[6].position = sf::Vector2f(tempLocation.x, tempLocation.y + m_size.y);
		m_lines[7].position = sf::Vector2f(tempLocation.x, tempLocation.y);
	}

	void setColor(bool isColliding)
	{
		for (unsigned int i{}; i < sizeof(m_lines) / sizeof(m_lines[0]); ++i)
		{
			if (isColliding)
			{
				m_lines[i].color = sf::Color::Red;
			}
			else
			{
				m_lines[i].color = sf::Color::White;
			}
		}
	}

	sf::Vertex* getLines()
	{
		return m_lines;
	}

	sf::Vector2f getSize()
	{
		return m_size;
	}

	bool getIsKill()
	{
		return m_isKill;
	}

	void setIsKill(bool newKill)
	{
		m_isKill = newKill;
	}
};

class GameObject
{
protected:
	sf::Vector2f m_location;
	Collision m_collision;
	AnimationComponent m_animComp;
	bool m_kill{ false };

public:
	GameObject() = default;
	GameObject(sf::Vector2f startLocation, sf::Vector2f collisionSize, sf::Vector2f collisionRelativeLocation, Animation *startAnim) :
		m_location{ startLocation }, m_animComp{ startAnim }
	{
		m_collision.setupCollision(collisionSize, collisionRelativeLocation, startLocation);
	}

	virtual void logicTick()
	{
		m_collision.updateCollision(m_location);
	}

	virtual void graphicTick(sf::RenderTexture& texture)
	{
		m_animComp.update();
		m_animComp.getSprite().setPosition(m_location);
		//texture.draw(m_animComp.getSprite());
	}

	void drawObject(sf::RenderTexture& texture)
	{
		texture.draw(m_animComp.getSprite());

		if (DEBUG)
		{
			m_collision.drawCollision(texture);
		}
	}

	void setLocation(sf::Vector2f newLocation)
	{
		m_location = newLocation;
	}

	sf::Vector2f getLocation()
	{
		return m_location;
	}

	void addLocation(sf::Vector2f deltaLocation)
	{
		m_location += deltaLocation;
	}

	virtual void checkCollision(std::vector<GameObject*> collidedObjects)
	{

		if (!collidedObjects.empty())
		{
			m_collision.setColor(true);
		}
		else
		{
			m_collision.setColor(false);
		}
	}

	Collision* getCollision()
	{
		return &m_collision;
	}

	void setCollisionIsKill(bool newKill)
	{
		m_collision.setIsKill(newKill);
	}

	bool getKill()
	{
		return m_kill;
	}

	void setKill(bool kill)
	{
		m_kill = kill;
	}
};

class Ground : public GameObject
{
protected:

public:
	Ground(sf::Vector2f location, Animation *startAnim, sf::Vector2f collisionSize, sf::Vector2f collisionRelativeLoc)
		: GameObject(location, collisionSize, collisionRelativeLoc, startAnim)
	{}
};

class Character : public GameObject
{
protected:
	const float m_gravityModifier{0.033};
	const float m_airResistance{0.5};
	const float m_groundResistance{0.1};
	const float m_movementSpeed{ 1 };
	sf::Vector2f m_lastPosition{};
	sf::Vector2f m_force{0, 0};
	bool onGround{ false };
	sf::Sound* m_jumpSound;

public:
	Character(sf::Vector2f location, Animation* startAnim, sf::Vector2f collisionSize, sf::Vector2f collisionRelativeLoc, sf::Sound* jumpSound)
		: GameObject(location, collisionSize, collisionRelativeLoc, startAnim), m_lastPosition{ location }, m_jumpSound{ jumpSound }
	{}

	bool movingRight{ false };
	bool movingLeft{ false };
	bool jumping{ false };

	void addForce(sf::Vector2f force)
	{
		m_force.x += force.x;

		if (onGround)
		{
			m_force.y += force.y;
		}
	}

	virtual void logicTick()
	{
		if (movingRight)
		{
			addForce(sf::Vector2f(m_movementSpeed, 0));
			//LOG(m_force.x);
		}

		if (movingLeft)
		{
			addForce(sf::Vector2f(m_movementSpeed * -1, 0));
			//LOG(m_force.x);
		}

		m_force.y += gravity * m_gravityModifier;
		m_lastPosition = m_location;

		m_location += m_force;
		
		if (onGround)
		{
			m_force.x *= (m_groundResistance);

			if (jumping)
			{
				addForce(sf::Vector2f(0, -3));
				m_jumpSound->play();
			}
		}
		else
		{
			m_force.x *= (m_airResistance);

		}

		GameObject::logicTick();
	}

	void checkCollision(std::vector<GameObject*> collidedObjects)
	{
		GameObject::checkCollision(collidedObjects);

		onGround = false;

		if (!collidedObjects.empty())
		{
			for (unsigned int i{}; i < collidedObjects.size(); ++i)
			{
				if (collidedObjects[i]->getCollision()->getIsKill())
				{
					m_kill = true;
				}
				
				{
					float distanceX{ m_lastPosition.x - collidedObjects[i]->getCollision()->getLines()->position.x };
					float distanceY{ m_lastPosition.y - collidedObjects[i]->getCollision()->getLines()->position.y };

					float selfMin{ m_lastPosition.y };
					//LOG("SelfMin");
					//LOG(selfMin);

					float selfMax{ m_lastPosition.y + m_collision.getSize().y };
					//LOG("SelfMax");
					//LOG(selfMax);

					float colMin{ collidedObjects[i]->getCollision()->getLines()->position.y };
					//LOG("ColMin");
					//LOG(colMin);

					float colMax{ collidedObjects[i]->getCollision()->getLines()->position.y + collidedObjects[i]->getCollision()->getSize().y };
					//LOG("ColMax");
					//LOG(colMax);

					if (((selfMax > colMin) && (selfMin < colMax)))
					{
						m_force.x = 0;
						float newX{};

						if (distanceX < 0)
						{
							newX = collidedObjects[i]->getCollision()->getLines()->position.x - m_collision.getSize().x - 1;
							//LOG("Push Left");
						}
						else
						{
							newX = collidedObjects[i]->getCollision()->getLines()->position.x + collidedObjects[i]->getCollision()->getSize().x + 1;
							//LOG("Push Right");
						}

						setLocation(sf::Vector2f(newX, getLocation().y));
						m_collision.updateCollision(m_location);
					}
					else
					{
						float newY{};

						if (distanceY < 0)
						{
							newY = collidedObjects[i]->getCollision()->getLines()->position.y - m_collision.getSize().y;
							onGround = true;
							//LOG("Push Up");

							if (m_force.y > 0)
							{
								m_force.y = 0;
							}
						}
						else
						{
							sf::Vertex* vertextPtr{ collidedObjects[i]->getCollision()->getLines() + 3 };
							newY = vertextPtr->position.y + 1;

							//newY = collidedObjects[i]->getCollision()->getLines()->position.y + m_collision.getSize().y + 1;
							//LOG("Push Down");

							if (m_force.y < 0)
							{
								m_force.y = 0;
							}
						}
						setLocation(sf::Vector2f(getLocation().x, newY));
						m_collision.updateCollision(m_location);
					}
					//LOG("");
				}
			}
		}
	}

};

class Obstacle : public GameObject
{
private:
	float m_speed;

public:
	Obstacle(sf::Vector2f startLoc, Animation *startAnim, sf::Vector2f colSize, sf::Vector2f colLocation, float speed) :
		GameObject(startLoc, colSize, colLocation, startAnim), m_speed{ speed }
	{
		LOG(m_speed);
	}

	virtual void logicTick()
	{
		addLocation(sf::Vector2f(m_speed * -1 / 3, 0));

		GameObject::logicTick();
	}

	void setSpeed(float newSpeed)
	{
		m_speed = newSpeed;
	}
};

class ObstacleSpawner : public GameObject
{
private:
	int m_lastSpawn{};
	std::vector<GameObject*>* m_staticObjectRef;
	sf::Vector2f m_spawnLoc;
	Animation* m_startAnim;
	Animation* m_bigAnim;
	Animation* m_flyingAnim;
	float m_pixelSpeed{ 1 };
	const float m_boxMinDistance{60.0};

public:
	ObstacleSpawner() = default;
	ObstacleSpawner(std::vector<GameObject*>* staticArray, sf::Vector2f startLoc, Animation* startAnim, Animation* rockAnim, Animation* treeAnim, Animation* emptyAnim, sf::Vector2f colSize, sf::Vector2f colLocation, sf::Vector2f spawnLoc) :
		GameObject(startLoc, colSize, colLocation, emptyAnim), m_staticObjectRef{ staticArray }, m_spawnLoc{ spawnLoc }, m_startAnim{ startAnim }, m_bigAnim{ rockAnim }, m_flyingAnim{ treeAnim }{}

	virtual void logicTick()
	{
		m_pixelSpeed += 0.001;

		int percChance{ rand() % 100 };

		if ((percChance > 70) && (++m_lastSpawn > m_boxMinDistance / m_pixelSpeed))
		{
			int isLargeBox{ rand() % 100};

			LOG(isLargeBox);

			if (isLargeBox < 33)
			{
				//sf::Vector2f(30, 30)
				//sf::Vector2f(0, 0)
				m_staticObjectRef->push_back(new Obstacle(m_spawnLoc, m_bigAnim, sf::Vector2f(30, 30), sf::Vector2f(0, 0), m_pixelSpeed));
			}
			else if (isLargeBox < 66)
			{
				m_staticObjectRef->push_back(new Obstacle(sf::Vector2f(m_spawnLoc.x, m_spawnLoc.y + 10), m_startAnim, sf::Vector2f(20, 20), sf::Vector2f(0, 0), m_pixelSpeed));
			}
			else
			{
				m_staticObjectRef->push_back(new Obstacle(sf::Vector2f(m_spawnLoc.x, m_spawnLoc.y - 20), m_flyingAnim, sf::Vector2f(20, 20), sf::Vector2f(0, 0), m_pixelSpeed));
			}
			m_lastSpawn = 0;
		}

		GameObject::logicTick();
	}

};

class CollisionHandler
{
private:
	bool checkAxis(GameObject* currentObject, GameObject* checkObject, int vertexToCheck, bool checkX)
	{
		sf::Vertex* currentMax{currentObject->getCollision()->getLines() + vertexToCheck};
		sf::Vertex* currentMin = currentMax + 1;

		sf::Vertex* checkMax{checkObject->getCollision()->getLines() + vertexToCheck};
		sf::Vertex* checkMin = checkMax + 1;

		bool MaxWithinBounds{};
		bool MinWithinBounds{};

		if (checkX)
		{
			MaxWithinBounds = (currentMax->position.x >= checkMax->position.x) && (currentMax->position.x <= checkMin->position.x);
			MinWithinBounds = (currentMin->position.x >= checkMax->position.x) && (currentMin->position.x <= checkMin->position.x);
		}
		else
		{
			MaxWithinBounds = (currentMax->position.y >= checkMax->position.y) && (currentMax->position.y <= checkMin->position.y);
			MinWithinBounds = (currentMin->position.y >= checkMax->position.y) && (currentMin->position.y <= checkMin->position.y);
		}

		return (MaxWithinBounds || MinWithinBounds);
	}

public:
	CollisionHandler(GameObject* objectToTest, std::vector<GameObject*> objectsToTestAgainst)
	{
		std::vector<GameObject*> tempVector;

		for (unsigned int j{}; j < objectsToTestAgainst.size(); ++j)
		{
				bool xCollides{ checkAxis(objectToTest, objectsToTestAgainst[j], 0, true) };
				bool yCollides{ checkAxis(objectToTest, objectsToTestAgainst[j], 2, false) };

				if (xCollides && yCollides)
				{
					tempVector.push_back(objectsToTestAgainst[j]);
				}
		}
		objectToTest->checkCollision(tempVector);
	}
};

int main()
{
	bool playing{ true };

	while (playing)
	{
		srand(time(nullptr));

		//Initial window settings
		sf::Vector2i targetResolution{ 320, 180 };

		sf::RenderWindow window(sf::VideoMode(1280, 720), "Game", sf::Style::Default);
		window.setKeyRepeatEnabled(false);
		window.setFramerateLimit(36);
		sf::View view(sf::Vector2f(targetResolution.x / 2, targetResolution.y / 2), (sf::Vector2f)targetResolution);
		window.setView(view);

		//Create render texture and sprite
		sf::RenderTexture mainRenderTexture;
		mainRenderTexture.create(targetResolution.x, targetResolution.y);
		sf::Sprite mainRenderSprite(mainRenderTexture.getTexture());

		//Create test background
		sf::Texture background;
		background.loadFromFile("Textures/Background.png");
		sf::Sprite backgroundSprite(background);

		sf::Texture playerTexture;
		playerTexture.loadFromFile("Textures/KiwiRun.png");

		sf::Texture rockTexture;
		rockTexture.loadFromFile("Textures/Rock.png");

		sf::Texture stumpTexture;
		stumpTexture.loadFromFile("Textures/Stump.png");

		sf::Texture treeTexture;
		treeTexture.loadFromFile("Textures/Tree.png");

		sf::Texture machineTexture;
		machineTexture.loadFromFile("Textures/Machine.png");

		sf::Texture emptyTexture;

		//Create animations

		Background backgroundObject(background);

		Animation playerRun{ playerTexture, 6, true };
		Animation rock{ rockTexture, 1, true };
		Animation stump{ stumpTexture, 1, true };
		Animation tree{ treeTexture, 1, true };
		Animation machine{ machineTexture, 2, true };
		Animation emptyAnim{ emptyTexture, 0 , false };

		//Create sounds
		
		sf::SoundBuffer hurtBuffer;
		hurtBuffer.loadFromFile("Audio/Hurt.wav");
		sf::Sound hurtSound;
		hurtSound.setBuffer(hurtBuffer);

		sf::SoundBuffer jumpBuffer;
		jumpBuffer.loadFromFile("Audio/Jump.wav");
		sf::Sound jumpSound;
		jumpSound.setBuffer(jumpBuffer);

		sf::SoundBuffer deathBuffer;
		deathBuffer.loadFromFile("Audio/Death.wav");
		sf::Sound deathSound;
		deathSound.setBuffer(deathBuffer);

		//Create objects
		Character* playerRef{ new Character(sf::Vector2f(100, 130), &playerRun,sf::Vector2f(16, 16), sf::Vector2f(0, 0), &jumpSound)};

		std::vector<GameObject*> staticObjects;
		std::vector<GameObject*> dynamicObjects;

		staticObjects.push_back(new Ground(sf::Vector2f(-100, 150), &emptyAnim, sf::Vector2f(500, 30), sf::Vector2f(0, 0)));
		staticObjects.push_back(new Ground(sf::Vector2f(0, -10), &emptyAnim, sf::Vector2f(319, 10), sf::Vector2f(0, 0)));
		staticObjects.push_back(new ObstacleSpawner(&staticObjects, sf::Vector2f(310, 0), &stump, &rock, &tree, &emptyAnim, sf::Vector2f(10, 180), sf::Vector2f(0, 0), sf::Vector2f(320, 120)));

		GameObject* isKillVolume{ new Ground(sf::Vector2f(0, -30), &machine, sf::Vector2f(51, targetResolution.y + 30), sf::Vector2f(-50, 0)) };
		isKillVolume->setCollisionIsKill(true);
		staticObjects.push_back(isKillVolume);

		dynamicObjects.push_back(playerRef);

		bool isPaused{ false };

		unsigned int frameCount{ 0 };

		hurtSound.setLoop(true);
		hurtSound.setVolume(10);
		hurtSound.play();

		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{

				switch (event.type)
				{
				case sf::Event::Resized:
					break;

				case sf::Event::Closed:
					playing = false;
					window.close();
					break;

				case sf::Event::KeyPressed:
					switch (event.key.code)
					{
					case sf::Keyboard::F11:
						break;

					case sf::Keyboard::Space:
						playerRef->jumping = true;
						break;

					case sf::Keyboard::Right:
						playerRef->movingRight = true;
						break;

					case sf::Keyboard::Left:
						playerRef->movingLeft = true;
						break;

					case sf::Keyboard::Escape:
						playing = false;
						window.close();
						break;
					}
					break;

				case sf::Event::KeyReleased:
					switch (event.key.code)
					{
					case sf::Keyboard::Right:
						playerRef->movingRight = false;
						break;

					case sf::Keyboard::Left:
						playerRef->movingLeft = false;
						break;

					case sf::Keyboard::Space:
						playerRef->jumping = false;
					}
				}
			}
			//~~LOGIC FRAME~~
			if (!isPaused)
			{

				backgroundObject.tick();

				//Check Collision
				for (unsigned int i{}; i < dynamicObjects.size(); ++i)
				{
					CollisionHandler handler(dynamicObjects[i], staticObjects);
				}

				//Update static objects
				for (unsigned int i{}; i < staticObjects.size(); ++i)
				{
					staticObjects[i]->logicTick();
				}

				//Update dynamic objects
				for (unsigned int i{}; i < dynamicObjects.size(); ++i)
				{
					dynamicObjects[i]->logicTick();
				}

				//Check distances

				for (unsigned int i{}; i < staticObjects.size(); ++i)
				{
					sf::Vector2f testLoc = (staticObjects[i]->getLocation());

					if ((testLoc.x < 0 - 100 || testLoc.x > targetResolution.x + 100) || (testLoc.y < 0 - 100 || testLoc.y > targetResolution.y + 100))
					{
						staticObjects[i]->setKill(true);
					}
				}

				for (unsigned int i{}; i < dynamicObjects.size(); ++i)
				{
					sf::Vector2f testLoc = (dynamicObjects[i]->getLocation());

					if ((testLoc.x < 0 - 100 || testLoc.x > targetResolution.x + 100) || (testLoc.y < 0 - 100 || testLoc.y > targetResolution.y + 100))
					{
						dynamicObjects[i]->setKill(true);
					}
				}

				//Check if player is flagged kill
				if (playerRef->getKill())
				{
					LOG("END GAME");
					isPaused = true;
					deathSound.play();
					hurtSound.stop();
				}

				//Delete flagged objects
				for (unsigned int i{ staticObjects.size() }; i-- > 0;)
				{
					if (staticObjects[i]->getKill())
					{
						delete staticObjects[i];
						staticObjects.erase(staticObjects.begin() + i);
					}
				}

				for (unsigned int i{ dynamicObjects.size() }; i-- > 0;)
				{
					if (dynamicObjects[i]->getKill())
					{
						delete dynamicObjects[i];
						dynamicObjects.erase(dynamicObjects.begin() + i);
					}
				}

				//LOG(frameCount);

				if (++frameCount >= 2)
				{
					frameCount = 0;

					LOG("--Update frame--");

					//Graphic update static objects

					for (unsigned int i{}; i < staticObjects.size(); ++i)
					{
						staticObjects[i]->graphicTick(mainRenderTexture);
					}

					//Graphic update dynamic objects

					for (unsigned int i{}; i < dynamicObjects.size(); ++i)
					{
						dynamicObjects[i]->graphicTick(mainRenderTexture);
					}
				}
			}

			//~~DRAW FRAME~~

			mainRenderTexture.clear();
			backgroundObject.draw(mainRenderTexture);

			//Draw static objects
			for (unsigned int i{}; i < staticObjects.size(); ++i)
			{
				staticObjects[i]->drawObject(mainRenderTexture);
			}

			//Draw dynamic objects

			for (unsigned int i{}; i < dynamicObjects.size(); ++i)
			{
				dynamicObjects[i]->drawObject(mainRenderTexture);
			}

			mainRenderTexture.display();

			window.clear();
			window.draw(mainRenderSprite);
			window.display();
		}

		//~~CLEAN UP~~

		//Delete static objects

		for (unsigned int i{}; i < staticObjects.size(); ++i)
		{
			delete staticObjects[i];
		}

		//Delete dynamic objects


		for (unsigned int i{}; i < dynamicObjects.size(); ++i)
		{
			delete dynamicObjects[i];
		}
	}

	return 0;
}