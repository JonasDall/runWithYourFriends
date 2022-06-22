#include "SfmlResizeManager.h"
#include "SFML/Graphics.hpp"

srManager::SfmlResizeManager::SfmlResizeManager(sf::RenderWindow& window, sf::View& view, sf::Vector2i resolution) : m_window{ window }, m_view{ view }, m_resolution{ resolution }
{}

void srManager::SfmlResizeManager::resize(sf::Vector2i resolution)
{

}