#pragma once

#include "../imgui/imgui.h"
#include "imgui-sfml.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <random>


#include "GameCell.h"

class GameRequestBoard : public sf::Drawable, public sf::Transformable {
public:
	GameRequestBoard(unsigned int seed);

	sf::Texture background_tex;
	sf::Sprite background;
	std::vector<GameRequest> requests;
	std::mt19937 randgen;
	int prog_counter;

	void SpawnNewRequest();

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};
