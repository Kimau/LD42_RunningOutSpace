#pragma once

sf::Color GetFromID(int id);

struct GameCell
{
	sf::Color color;
	int prog_id;
	int offset;

	GameCell() {
		color = sf::Color::Transparent;
		prog_id = -1;
	}
};

struct GameRequest
{
	sf::Color color;
	int prog_id;
	int numcells;
	int cellsplaced;
	
	sf::Time expiryTime;
	sf::Clock timer;

	GameRequest(int prog_id_, int num, sf::Time time_max) {
		prog_id = prog_id_;
		color = GetFromID(prog_id);
		numcells = num;
		cellsplaced = 0;
		expiryTime = time_max;
	}
};

