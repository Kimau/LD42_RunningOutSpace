#pragma once

sf::Color GetFromID(int id);
float RandToSinRange(unsigned int input);

struct TermLine
{
	sf::Color color;
	char str[100];
};

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
	int offset;

	int cellsplaced;
	
	sf::Time expiryTime;
	sf::Clock timer;

	GameRequest(int prog_id_, int num, sf::Time time_max, int offset = 0) : prog_id(prog_id_), offset(0), numcells(num) {
		color = GetFromID(prog_id);
		cellsplaced = 0;
		offset = 0;
		expiryTime = time_max;
	}

	float GetProgressAsPer() const { return timer.getElapsedTime() / expiryTime; }
};

struct GameProgram
{
	sf::Color color;
	int prog_id;
	int cells_in_mem;
	int cells_requested;
};
