#include "GameRequestBoard.h"

#include "renderFeedback.h"
#include <random>
#include <time.h>

GameRequestBoard::GameRequestBoard(unsigned int seed)
{
	background_tex.loadFromFile("request.png");
	background.setTexture(background_tex);
	gradient_tex.loadFromFile("redgrad.png");

	prog_counter = 1;
	randgen.seed(seed);
}

void GameRequestBoard::SpawnNewRequest()
{
	int size = 2 + randgen() % 20;
	GameRequest r = GameRequest(prog_counter++, size, sf::seconds(1.0f + size * (0.5f + 4.0f * sinf((randgen() % 31415) * 0.0001f))));

	requests.push_back(r);
}

void GameRequestBoard::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	// Render Feedback
	sf::Vector2f localToObj = states.transform.getInverse().transformPoint(g_renderFeedback.cursorPos);
	if (background.getLocalBounds().contains(localToObj)) {
		g_renderFeedback.hovered = this;
	}

	target.draw(background, states);

	// Draw the Requests
	static float xStart = 70.0;
	static float yStart = 82.f;
	static float yStep = 28.0f;
	static float yPad = 16.0f;
	static float usedWidthPer = 0.65f;

	/*
		ImGui::DragFloat("xStart", &xStart);
		ImGui::DragFloat("yStart", &yStart);
		ImGui::DragFloat("yStep", &yStep);
		ImGui::DragFloat("yPad", &yPad);
		ImGui::DragFloat("usedWidthPer", &usedWidthPer, 0.01f);*/

	float maxWidth = background_tex.getSize().x * usedWidthPer;
	float y = yStart;

	////////////////////////////////////////////////////////////////////////// Draw Programs
	sf::RectangleShape progRect;
	for (int rOffset = 0; rOffset < requests.size(); ++rOffset)
	{
		const GameRequest& r = requests[rOffset];

		float borderThick = 1.0f;

		float xWidth = std::min(yStep, (maxWidth - (r.numcells + 1)*(borderThick)) / r.numcells);
		progRect.setSize(sf::Vector2f{
			xWidth,
			yStep
			});


		bool witnessMeForIamselected = false;
		if ((g_renderFeedback.hovered == this) &&
			(localToObj.y > y) &&
			(localToObj.y < (y + yStep))) {
			g_renderFeedback.hover_cell.y = rOffset;
			witnessMeForIamselected = true;

			if ((localToObj.x > xStart) && (localToObj.x < (xStart + maxWidth))) {
				g_renderFeedback.hover_cell.x = int((localToObj.x - xStart) / xWidth);
			}
		}


		////////////////////////////////////////////////////////////////////////// color me pretty
		if (r.cellsplaced > 0) {
			sf::Color color = r.color;
			color.r /= 2; color.g /= 2; color.b /= 2;
			progRect.setFillColor(color);

			if (witnessMeForIamselected)
				progRect.setOutlineColor(sf::Color{ 150,150,150 });
			else
				progRect.setOutlineColor(sf::Color::Black);

			progRect.setOutlineThickness(borderThick);
		}


		////////////////////////////////////////////////////////////////////////// DRaw cells
		for (int i = 0; i < r.numcells; i++) {
			if (i == r.cellsplaced) {
				progRect.setFillColor(r.color);
				sf::Color outline = r.color;

				outline.r /= 5; outline.g /= 5; outline.b /= 5;
				if (witnessMeForIamselected) {
					outline.r += 50; outline.g += 50; outline.b += 50;
				}

				progRect.setOutlineColor(outline);
				progRect.setOutlineThickness(borderThick);
			}

			progRect.setPosition(sf::Vector2f(xStart + i * (progRect.getSize().x + borderThick), y));
			target.draw(progRect, states);
		}

		{
			float timeOut = r.timer.getElapsedTime() / r.expiryTime;
			sf::VertexArray gradient = sf::VertexArray(sf::PrimitiveType::Quads, 3 * 4);

			float xa = xStart + borderThick;
			float xd = xStart + (xWidth + borderThick) * r.numcells - borderThick * 2.0f;
			float xb = std::min(xd, std::max(xa, xa + (xd - xa)* timeOut - 1.0f));
			float xc = std::min(xd, std::max(xa, xa + (xd - xa) * timeOut + 1.0f));

			sf::Color redCol = { 255, 0, 0, 200 };
			sf::Color blackCol = { 0,0,0,200 };

			int i = 0;
			gradient[i++] = sf::Vertex(sf::Vector2f{ xa, y + yStep }, redCol, sf::Vector2f{ 0,0 });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xb, y + yStep }, redCol, sf::Vector2f{ 0,0 });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xb, y + yStep + 5.0f }, redCol, sf::Vector2f{ 0,0.99f });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xa, y + yStep + 5.0f }, redCol, sf::Vector2f{ 0,0.99f });

			gradient[i++] = sf::Vertex(sf::Vector2f{ xb,y + yStep }, redCol, sf::Vector2f{ 0,0 });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xc,y + yStep }, blackCol, sf::Vector2f{ 0.99f,0 });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xc,y + yStep + 5.0f }, blackCol, sf::Vector2f{ 0.99f,0.99f });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xb,y + yStep + 5.0f }, redCol, sf::Vector2f{ 0,0.99f });

			gradient[i++] = sf::Vertex(sf::Vector2f{ xc, y + yStep }, blackCol, sf::Vector2f{ 0.99f,0 });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xd, y + yStep }, blackCol, sf::Vector2f{ 0.99f,0 });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xd, y + yStep + 5.0f }, blackCol, sf::Vector2f{ 0.99f,0.99f });
			gradient[i++] = sf::Vertex(sf::Vector2f{ xc, y + yStep + 5.0f }, blackCol, sf::Vector2f{ 0.99f,0.99f });

			target.draw(gradient, states);
		}

		y += yStep + yPad;
	}
}
