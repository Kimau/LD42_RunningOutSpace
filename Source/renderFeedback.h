#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

struct RenderFeedback {
	sf::Vector2f cursorPos;
	sf::Vector2i hover_cell;
	const sf::Drawable* hovered;
};

extern RenderFeedback g_renderFeedback;
