#include "GameGrid.h"

#include "renderFeedback.h"

GameGrid::GameGrid(int w, int h, sf::Vector2f cellSize_)
{
	border_size = 1.0f;
	bgColor = { 0,0,0 };
	borderColor = { 100, 0 , 0 };
	cell_size = cellSize_;

	numCells = { w, h };
	selected = sf::Vector2i{ -1, -1 };

	cellData.resize(w*h);
	for (auto& c : cellData) { c = GameCell(); }

	RebuildVerts();
}

void GameGrid::RebuildVerts()
{
	const sf::Vector2f bounds = TotalSize();

	border_verts = sf::VertexArray(sf::Quads, (numCells.x + 1) * 4 + (numCells.y + 1) * 4);
	int i = 0;
	for (int x = 0; x <= numCells.x; x++) { // VERTICAL LINES
		float yPos = x * (cell_size.y + border_size);
		border_verts[i++].position = { yPos - border_size * 0.5f, -border_size * 0.5f };
		border_verts[i++].position = { yPos - border_size * 0.5f, bounds.y - border_size * 0.5f };
		border_verts[i++].position = { yPos + border_size * 0.5f, bounds.y - border_size * 0.5f };
		border_verts[i++].position = { yPos + border_size * 0.5f, -border_size * 0.5f };
	}

	for (int y = 0; y <= numCells.y; y++) { // HORIZONAL LINES
		float xPos = y * (cell_size.y + border_size);

		border_verts[i++].position = { 0 - border_size * 0.5f, xPos - border_size * 0.5f };
		border_verts[i++].position = { bounds.x - border_size * 0.5f, xPos - border_size * 0.5f };
		border_verts[i++].position = { bounds.x - border_size * 0.5f, xPos + border_size * 0.5f };
		border_verts[i++].position = { 0 - border_size * 0.5f, xPos + border_size * 0.5f };
	}

	cell_verts = sf::VertexArray(sf::Quads, 4 * cellData.size());
	i = 0;
	for (int y = 0; y < numCells.y; y++) {
		for (int x = 0; x < numCells.x; x++) {
			sf::Vector2f pos = GetCellCenter({ x,y });
			sf::Vector2f tl = pos - (cell_size * 0.5f);
			sf::Vector2f br = pos + (cell_size * 0.5f);
			cell_verts[i++].position = tl;
			cell_verts[i++].position = { br.x, tl.y };
			cell_verts[i++].position = br;
			cell_verts[i++].position = { tl.x, br.y };
		}
	}

	for (int i = 0; i < border_verts.getVertexCount(); i++) border_verts[i].color = borderColor;
	for (int i = 0; i < cell_verts.getVertexCount(); i++) cell_verts[i].color = bgColor;
}

sf::Vector2f GameGrid::TotalSize() const
{
	return sf::Vector2f{
		(numCells.x * (cell_size.x + border_size)) + border_size,
		(numCells.y *(cell_size.y + border_size)) + border_size,
	};
}

sf::Vector2f GameGrid::GetCellTopLeft(sf::Vector2i pos) const
{
	return sf::Vector2f{
		pos.x * (cell_size.x + border_size),
		pos.y * (cell_size.y + border_size)
	};
}

sf::Vector2f GameGrid::GetCellCenter(sf::Vector2i pos) const
{
	return GetCellTopLeft(pos) + (cell_size + sf::Vector2f{ border_size, border_size }) * 0.5f;
}

int GameGrid::Drop(GameRequest& req)
{
	if (g_renderFeedback.hovered != this)
		return -1;

	int placedCells = 0;

	const int offset = g_renderFeedback.hover_cell.y * numCells.x + g_renderFeedback.hover_cell.x;
	const int limit = (offset + req.numcells - req.cellsplaced);

	for (int i = offset; i < limit; i++) {
		if (i >= cellData.size())
			return placedCells;
		
		if (cellData[i].prog_id >= 0)
			return placedCells;

		cellData[i].prog_id = req.prog_id;
		cellData[i].color = req.color;
		cellData[i].offset = req.cellsplaced++;
	}

	return placedCells;
}

void GameGrid::Update(LogicFeedback& logicfb)
{
	for (GameCell& gc : cellData) {
		if(gc.prog_id <0)
			continue;

		for (int pid : logicfb.explode_progid)
			if (pid == gc.prog_id) {
				gc.color = sf::Color{ 100,100,100 };
				goto end_of_cell_loop;				// -----------> GOTO
			}

		for (int pid : logicfb.del_progid)
			if (pid == gc.prog_id) {
				gc.prog_id = -1;
				gc.color = sf::Color::Transparent;
				goto end_of_cell_loop;				// -----------> GOTO
			}

	end_of_cell_loop:
		continue;
	}

	int i = 0;
	for (GameCell& gc : cellData) {
		sf::Color cc = gc.color;

		cell_verts[i++].color = cc;
		cell_verts[i++].color = cc;
		cell_verts[i++].color = cc;
		cell_verts[i++].color = cc;
	}
}

void GameGrid::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	// Render Feedback
	sf::Vector2f localToObj = states.transform.getInverse().transformPoint(g_renderFeedback.cursorPos);
	if (sf::Rect<float>(sf::Vector2f(0, 0), TotalSize()).contains(localToObj)) {
		g_renderFeedback.hovered = this;

		g_renderFeedback.hover_cell = sf::Vector2i{
			int(localToObj.x / (cell_size.x + border_size)),
			int(localToObj.y / (cell_size.y + border_size))
		};
	}

	// Draw
	target.draw(cell_verts, states);
	target.draw(border_verts, states);

	if (selected.x >= 0) {
		// Draw Hover Highlight
		sf::RectangleShape hover;
		hover.setSize(cell_size);
		hover.setPosition(GetCellTopLeft(selected));
		hover.setFillColor(sf::Color{ 255,255,255,100 });
		hover.setOutlineColor(sf::Color::Transparent);

		target.draw(hover, states);
	}

	if (g_renderFeedback.hovered == this) {
		// Draw Hover Highlight
		sf::RectangleShape hover;
		hover.setSize(cell_size);
		hover.setPosition(GetCellTopLeft(g_renderFeedback.hover_cell));
		hover.setFillColor(sf::Color::Transparent);
		hover.setOutlineColor(sf::Color::Blue);
		hover.setOutlineThickness(border_size);
		target.draw(hover, states);
	}
}