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

#include "renderFeedback.h"
#include "GameGrid.h"
#include "GameRequestBoard.h"

RenderFeedback g_renderFeedback;
sf::Font g_console_font;

sf::Color presetColours[] = {
	sf::Color{ 255, 0, 128 },
	sf::Color{ 0, 64, 128 },
	sf::Color{ 255, 128, 128 },
	sf::Color{ 128, 64, 0 },
	sf::Color{ 0, 255, 0 },
	sf::Color{ 64, 64, 128 },
	sf::Color{ 0, 128, 255 },
	sf::Color{ 64, 128, 0 },
	sf::Color{ 255, 0, 0 },
	sf::Color{ 64, 0, 128 },
	sf::Color{ 128, 128, 255 },
	sf::Color{ 128, 255, 0 },
	sf::Color{ 0, 0, 128 },
	sf::Color{ 0, 255, 128 },
	sf::Color{ 0, 128, 64 },
	sf::Color{ 0, 0, 255 },
	sf::Color{ 128, 64, 64 },
	sf::Color{ 128, 0, 255 },
	sf::Color{ 128, 0, 0 },
	sf::Color{ 0, 128, 0 },
	sf::Color{ 128, 255, 128 },
	sf::Color{ 128, 0, 64 },
	sf::Color{ 255, 128, 0 },
	sf::Color{ 64, 128, 64 }
};

static unsigned int side_term_font_size = 20;
static float side_term_left = 24.0f;
static float side_term_top = 9.0f;
static float side_term_lineheight = 18.0f;


sf::Color GetFromID(int id)
{
	return presetColours[id % 24];
}

float RandToSinRange(unsigned int input)
{
	return sinf((input % 31415) * 0.0001f);
}


void ToImColour(sf::Color &bgColor, float color[3])
{
	color[0] = float(bgColor.r) / 255.0f;
	color[1] = float(bgColor.g) / 255.0f;
	color[2] = float(bgColor.b) / 255.0f;
}

void ToSFMLColor(sf::Color &bgColor, float color[3])
{
	bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
	bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
	bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
}

int WinMain()
{
	sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	std::string logfile = "sfml-log.txt";
	std::ofstream file(logfile, std::ios::binary);
	sf::err().rdbuf(file.rdbuf());

	window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.
	sf::Clock deltaClock;

	g_console_font.loadFromFile("5by5.ttf");

	// Background
	sf::Color bgColor = { 100, 100, 100 };
	sf::Texture background_img;
	sf::Sprite background;
	{
		background_img.loadFromFile("background.png");
		background.setTexture(background_img);
	}

	sf::Texture side_term_tex;
	sf::Sprite side_term;
	{
		side_term_tex.loadFromFile("side_term.png");
		side_term.setTexture(side_term_tex);

		side_term.setPosition(sf::Vector2f{ 0, window.getSize().y*0.5f - side_term_tex.getSize().y*0.5f });
	}

	sf::Texture bottom_term_tex;
	sf::Sprite bottom_term;
	{
		bottom_term_tex.loadFromFile("bottom_term.png");
		bottom_term.setTexture(bottom_term_tex);

		bottom_term.setPosition(sf::Vector2f{ window.getSize().x*0.4f - bottom_term_tex.getSize().x*0.5f, float(window.getSize().y - bottom_term_tex.getSize().y) });
	}


	char windowTitle[255] = "ImGui + SFML = <3";
	window.setTitle(windowTitle);

	//////////////////////////////////////////////////////////////////////////// Setup Game Bullshit
	std::mt19937 game_rand;

	int running_pid = -1;
	std::vector<GameProgram> progs;
	int prog_id_counter = 1;
	float seconds_till_prog_Spawn = 0;

	GameGrid ggrid = GameGrid(8, 12);
	ggrid.border_size = 2.0f;
	ggrid.cell_size.x = floorf(window.getSize().y * 0.7f / ggrid.numCells.y);
	ggrid.cell_size.y = ggrid.cell_size.x;
	ggrid.RebuildVerts();
	ggrid.setPosition(sf::Vector2f{
		300.0f,
		window.getSize().y * 0.08f
		});

	GameRequestBoard grequest = GameRequestBoard(game_rand());
	grequest.setPosition(sf::Vector2f{
		window.getSize().x - grequest.background.getLocalBounds().width - 50.0f,
		window.getSize().y * 0.5f - grequest.background.getLocalBounds().height * 0.5f
		});



	while (window.isOpen())
	{
		sf::Time delta = deltaClock.restart();

		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::KeyReleased:
			{
				switch (event.key.code)
				{
				case sf::Keyboard::S:
					seconds_till_prog_Spawn = 0.0f;

				default:
					break;
				}

			} break;

			default:
				break;
			}

		}

		auto mouseVec = sf::Mouse::getPosition(window);
		sf::Vector2f mouseVecf = sf::Vector2f(mouseVec);

		////////////////////////////////////////////////////////////////////////// GAME LOGIC UPDATE
		{
			LogicFeedback logicfb;

			if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

				if (g_renderFeedback.dragged == nullptr) {
					// Do selection
					if (g_renderFeedback.hovered == &ggrid) {
						ggrid.selected = g_renderFeedback.hover_cell;
					}
					else {
						ggrid.selected = sf::Vector2i(-1, -1);
					}

					if (g_renderFeedback.hovered == &grequest) {
						grequest.selected = g_renderFeedback.hover_cell;
					}
					else {
						grequest.selected = sf::Vector2i(-1, -1);
					}

					g_renderFeedback.drag_time = sf::Time::Zero;
					g_renderFeedback.dragStartPos = g_renderFeedback.cursorPos;
					g_renderFeedback.dragged = g_renderFeedback.hovered;
					g_renderFeedback.drag_cells = g_renderFeedback.hover_cell;
				}

				g_renderFeedback.drag_time += delta;
			}
			else {


				if (g_renderFeedback.dragged != nullptr) {
					if ((g_renderFeedback.hovered == &ggrid) && (g_renderFeedback.dragged == &grequest)) {
						if (GameRequest* req = grequest.GetRequest(g_renderFeedback.drag_cells.y)) {
							int dropped = ggrid.Drop(*req);
							if (dropped > 0) {
								ggrid.RebuildVerts();
								grequest.Placed(req->prog_id, dropped);
							}
						}
					}

					if ((g_renderFeedback.hovered != g_renderFeedback.dragged) || (g_renderFeedback.drag_time.asSeconds() > 0.5f)) {
						ggrid.selected = sf::Vector2i{ -1,-1 };
						grequest.selected = sf::Vector2i{ -1, -1 };
					}

					g_renderFeedback.dragged = nullptr;
					g_renderFeedback.drag_cells = sf::Vector2i(-1, -1);
				}
			}

			if (seconds_till_prog_Spawn <= 0) { // <---------------------------------------- SPAWN NEW PROGS
				seconds_till_prog_Spawn = 5.0f + 30.0f * RandToSinRange(game_rand());

				GameProgram newProg;
				newProg.prog_id = prog_id_counter++;
				newProg.cells_in_mem = 0;
				newProg.cells_requested = 2 + game_rand() % 6;
				newProg.color = GetFromID(newProg.prog_id);

				progs.push_back(newProg);

				grequest.SpawnNewRequest(newProg);
			}
			seconds_till_prog_Spawn -= delta.asSeconds();


			grequest.UpdateRequests(logicfb);					// <<--------------- UPdates
			ggrid.Update(logicfb);


			for (int pid : logicfb.explode_progid) {
				auto wipe = std::remove_if(progs.begin(), progs.end(), [&](GameProgram& p) -> bool {
					return (p.prog_id == pid);
				});
				
				if(wipe != progs.end())
					progs.erase(wipe);

				if (pid == g_renderFeedback.prog_hover_prev) {
					g_renderFeedback.dragged = nullptr;
					g_renderFeedback.drag_cells = sf::Vector2i(-1, -1);
				}
			}

			for (int pid : logicfb.placed_progid) {
				for (GameProgram& p : progs) {
					if (p.prog_id != pid)
						continue;

					p.cells_in_mem += p.cells_requested;
					p.cells_requested = 0;
				}

				if (pid == g_renderFeedback.prog_hover_prev) {
					g_renderFeedback.dragged = nullptr;
					g_renderFeedback.drag_cells = sf::Vector2i(-1, -1);
				}
			}
		}



		//////////////////////////////////////////////////////////////////////////   IMGUI UPDATE
		{
			ImGui::SFML::Update(window, deltaClock.restart());
			ImGui::Begin("Sample window"); // begin window

										   // Background color edit
			float color[3];
			ToImColour(bgColor, color);
			if (ImGui::ColorEdit3("Background color", color)) {
				ToSFMLColor(bgColor, color);
			}

			side_term_font_size = 20;
			ImGui::DragFloat("side_term_left", &side_term_left);
			ImGui::DragFloat("side_term_top", &side_term_top);
			ImGui::DragFloat("side_term_lineheight", &side_term_lineheight);

			ImGui::End(); // end window
		}


		//////////////////////////////////////////////////////////////////////////  RENDER
		{
			g_renderFeedback.cursorPos = mouseVecf;
			g_renderFeedback.hovered = nullptr;
			if((g_renderFeedback.dragged == false) || (g_renderFeedback.prog_hover_prev < 0))
				g_renderFeedback.prog_hover_prev = g_renderFeedback.prog_hovered;
			g_renderFeedback.prog_hovered = -1;

			window.clear(bgColor);

			window.draw(background);

			window.draw(side_term);			// <-------------- Side Term
			{
				float yPos = side_term.getGlobalBounds().top + side_term_top;
				char buffer[100];
				for (GameProgram& p : progs) {
					if (p.prog_id == running_pid)
						sprintf_s(buffer, 100, "> #%03d", p.prog_id);
					else if (p.cells_requested > 0)
						sprintf_s(buffer, 100, "! #%03d", p.prog_id);
					else
						sprintf_s(buffer, 100, "  #%03d", p.prog_id);

					sf::Text sidetext = sf::Text(buffer, g_console_font, side_term_font_size);
					sidetext.setPosition(side_term_left, yPos);
					sidetext.setFillColor(p.color);

					if (sidetext.getGlobalBounds().contains(g_renderFeedback.cursorPos)) {
						g_renderFeedback.prog_hovered = p.prog_id;
						sidetext.setFillColor(sf::Color::Black);
						sidetext.setOutlineColor(p.color);
						sidetext.setOutlineThickness(5.0f);
					}
					else if (g_renderFeedback.prog_hover_prev == p.prog_id) {
						sidetext.setFillColor(sf::Color::Black);
						sidetext.setOutlineColor(p.color);
						sidetext.setOutlineThickness(3.0f);
					}

					window.draw(sidetext);

					yPos += side_term_lineheight;
				}
			}

			window.draw(bottom_term);

			window.draw(ggrid);
			window.draw(grequest);


			ImGui::SFML::Render(window);
			window.display();
		}
	}


	ImGui::SFML::Shutdown();

	return 0;
}
