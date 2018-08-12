#include "GameRequestBoard.h"

#include "renderFeedback.h"
#include <random>
#include <time.h>


static float xStart = 70.0;
static float yStart = 82.f;
static float yStep = 28.0f;
static float yPad = 16.0f;
static float usedWidthPer = 0.65f;

GameRequestBoard::GameRequestBoard(unsigned int seed)
{
	background_tex.loadFromFile("request.png");
	background.setTexture(background_tex);
	gradient_tex.loadFromFile("redgrad.png");

	prog_counter = 1;
	selected = sf::Vector2i(-1, -1);
	randgen.seed(seed);
}

void GameRequestBoard::SpawnNewRequest()
{
	int size = 2 + randgen() % 20;
	GameRequest r = GameRequest(prog_counter++, size, sf::seconds(1.0f + size * (0.5f + 4.0f * sinf((randgen() % 31415) * 0.0001f))));

	requests.push_back(r);
}

void GameRequestBoard::Placed(int prog_id, int dropped)
{
	for (GameRequest& req : requests) {
		if (prog_id == req.prog_id) {
			req.cellsplaced += dropped;
		}
	}
}

void GameRequestBoard::UpdateRequests(LogicFeedback logicfb)
{
	std::vector<int> removeReq;

	for (GameRequest& req : requests) {

		for (int pid : logicfb.explode_progid)
			if (pid == req.prog_id) {
				removeReq.push_back(req.prog_id);
				goto end_of_request_loop;				// -----------> GOTO
			}

		for (int pid : logicfb.del_progid)
			if (pid == req.prog_id) {
				removeReq.push_back(req.prog_id);
				goto end_of_request_loop;				// -----------> GOTO
			}

		if (req.cellsplaced >= req.numcells) {
			removeReq.push_back(req.prog_id);
			logicfb.del_progid.push_back(req.prog_id);
			goto end_of_request_loop;					// -----------> GOTO
		}

		if (req.GetProgressAsPer() >= 1.0f) {
			// Do Game over explode maybe
			removeReq.push_back(req.prog_id);
			logicfb.explode_progid.push_back(req.prog_id);
			goto end_of_request_loop;					// -----------> GOTO
		}

	end_of_request_loop:
		continue;
	}

	if (removeReq.size() > 0) {
		int i = requests.size();
		requests.erase(std::remove_if(requests.begin(), requests.end(), [&](GameRequest& req) -> bool {
			for (int pid : removeReq)
				if (pid == req.prog_id)
					return true;
			return false;
		}));

		IM_ASSERT(i != requests.size());
	}

	// TODO :: Scanning Req
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

	/*
		ImGui::DragFloat("xStart", &xStart);
		ImGui::DragFloat("yStart", &yStart);
		ImGui::DragFloat("yStep", &yStep);
		ImGui::DragFloat("yPad", &yPad);
		ImGui::DragFloat("usedWidthPer", &usedWidthPer, 0.01f);*/

	float maxWidth = background_tex.getSize().x * usedWidthPer;
	float y = yStart;
	float yDrag = y;

	////////////////////////////////////////////////////////////////////////// Draw Programs
	for (int rOffset = 0; rOffset < requests.size(); ++rOffset)
	{
		if ((g_renderFeedback.dragged == this) && (g_renderFeedback.drag_cells.y == rOffset)) {
			yDrag = y;
		}
		else {
			DrawRequest(target, states, rOffset, maxWidth, y, localToObj);
		}

		
		y += yStep + yPad;
	}

	if (g_renderFeedback.dragged == this) {
		DrawRequest(target, states, g_renderFeedback.drag_cells.y, maxWidth, yDrag, localToObj);
	}
}

void GameRequestBoard::DrawRequest(sf::RenderTarget &target, sf::RenderStates states, int rOffset, float maxWidth, float y, sf::Vector2f &mouseInLocal) const
{
	sf::RectangleShape progRect;
	const GameRequest& r = requests[rOffset];
	float borderThick = 2.0f;
	float xWidth = std::min(yStep, (maxWidth / r.numcells) - borderThick);


	bool witnessMeForIamselected = false;

	if ((g_renderFeedback.dragged == this) && (g_renderFeedback.drag_cells.y == rOffset)) {
		states = sf::RenderStates::Default;
		states.transform = states.transform.combine(getTransform());
		states.transform = states.transform.translate(g_renderFeedback.cursorPos - g_renderFeedback.dragStartPos);
	}
	else if ((g_renderFeedback.hovered == this) &&
		(mouseInLocal.y > y) &&
		(mouseInLocal.y < (y + yStep))) {
		g_renderFeedback.hover_cell.y = rOffset;
		witnessMeForIamselected = true;

		if ((mouseInLocal.x > xStart) && (mouseInLocal.x < (xStart + maxWidth))) {
			g_renderFeedback.hover_cell.x = std::max(0, std::min(r.numcells - 1, int((mouseInLocal.x - xStart) / (xWidth + borderThick))));
		}
	}

	////////////////////////////////////////////////////////////////////////// Draw Border
	{
		progRect.setSize(sf::Vector2f{
			(xWidth + borderThick) * r.numcells + borderThick,
			yStep + borderThick * 2.0f
			});

		progRect.setPosition(sf::Vector2f(xStart - borderThick, y - borderThick));

		sf::Color col = r.color;
		if (selected.y == rOffset)
			col = sf::Color::White;
		else if (witnessMeForIamselected)
		{
			col.r += 50; col.g += 50; col.b += 50;
		}
		else {
			col.r /= 5; col.g /= 5; col.b /= 5;
		}

		progRect.setFillColor(col);
		progRect.setOutlineThickness(0);

		target.draw(progRect, states);
	}


	////////////////////////////////////////////////////////////////////////// DRaw cells
	progRect.setSize(sf::Vector2f{
		xWidth,
		yStep
		});

	// Loop the fuckers
	for (int i = 0; i < r.numcells; i++) {
		if (witnessMeForIamselected && (i == g_renderFeedback.hover_cell.x))
			continue;

		// color me pretty                             <<<<---- DANGER YOU FUCKING IDIOT
		if (i < r.cellsplaced) {
			sf::Color color = r.color;
			color.r /= 2; color.g /= 2; color.b /= 2;
			progRect.setFillColor(color);
			progRect.setOutlineColor(sf::Color{ 150,150,150 });
		}
		else {
			progRect.setFillColor(r.color);
			sf::Color outline = r.color;

			outline.r /= 5; outline.g /= 5; outline.b /= 5;
			if (witnessMeForIamselected) {
				outline.r += 50; outline.g += 50; outline.b += 50;
			}

			progRect.setOutlineColor(outline);
		}

		progRect.setOutlineThickness(0);
		if (witnessMeForIamselected && (i == g_renderFeedback.hover_cell.x))
			progRect.setOutlineThickness(borderThick*2.0f);

		progRect.setPosition(sf::Vector2f(xStart + i * (xWidth + borderThick), y));
		target.draw(progRect, states);
	}

	if (witnessMeForIamselected)
	{
		int i = g_renderFeedback.hover_cell.x;

		// color me pretty                                <<<<---- DANGER YOU FUCKING IDIOT
		if (i < r.cellsplaced) {
			sf::Color color = r.color;
			color.r /= 2; color.g /= 2; color.b /= 2;
			progRect.setFillColor(color);
			progRect.setOutlineColor(sf::Color{ 150,150,150 });
		}
		else {
			progRect.setFillColor(r.color);
			sf::Color outline = r.color;

			outline.r /= 5; outline.g /= 5; outline.b /= 5;
			if (witnessMeForIamselected) {
				outline.r += 50; outline.g += 50; outline.b += 50;
			}

			progRect.setOutlineColor(outline);
		}

		progRect.setOutlineThickness(borderThick*2.0f);
		progRect.setPosition(sf::Vector2f(xStart + i * (xWidth + borderThick), y));
		target.draw(progRect, states);
	}


	{
		float timeOut = r.GetProgressAsPer();

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
}
