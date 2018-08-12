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
#include "renderFeedback.h"

class GameRequestBoard : public sf::Drawable, public sf::Transformable {
public:
	GameRequestBoard(unsigned int seed);

	sf::Texture gradient_tex;
	sf::Texture background_tex;
	sf::Sprite background;
	std::vector<GameRequest> requests;
	std::mt19937 randgen;
	sf::Vector2i selected;
	sf::Vector2i dragging;

	void SpawnNewRequest(const GameProgram& srcProg);
	GameRequest* GetRequest(unsigned int y);
	void Placed(int prog_id, int dropped);
	void UpdateRequests(LogicFeedback& logicfb);

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void DrawRequest(sf::RenderTarget &target, sf::RenderStates states, int rOffset, float maxWidth, float y, sf::Vector2f &mouseInLocal) const;
};
