#pragma once
#include "SFML/Graphics.hpp"

namespace srManager
{
	class SfmlResizeManager
	{
	private:
		sf::RenderWindow& m_window;
		sf::View& m_view;
		sf::Vector2i m_resolution;

	public:
		SfmlResizeManager() = default;
		SfmlResizeManager(sf::RenderWindow& window, sf::View& view ,sf::Vector2i resolution);

		void resize(sf::Vector2i resolution);
	};
}