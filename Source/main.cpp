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

sf::Color presetColours[] = {
	sf::Color::Red,
	sf::Color::Green,
	sf::Color::Blue,
	sf::Color::Yellow,
	sf::Color::Magenta,
	sf::Color::Cyan // 6
};

sf::Color GetFromID(int id)
{
	return presetColours[id % 6];
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

	// Background
	sf::Color bgColor = { 100, 100, 100 };
	sf::Texture background_img;
	sf::Sprite background;
	{
		background_img.loadFromFile("background.png");
		background.setTexture(background_img);
	}


	char windowTitle[255] = "ImGui + SFML = <3";
	window.setTitle(windowTitle);

	//////////////////////////////////////////////////////////////////////////// Setup Game Bullshit
	std::mt19937 game_rand;

	GameGrid ggrid = GameGrid(8, 12, sf::Vector2f{ 45,45 });
	ggrid.border_size = 2.0f;
	ggrid.cell_size.x = floorf(window.getSize().y * 0.7f / ggrid.numCells.y);
	ggrid.cell_size.y = ggrid.cell_size.x;
	ggrid.RebuildVerts();
	ggrid.setPosition(sf::Vector2f{
		100.0f, 
		window.getSize().y * 0.5f - ggrid.TotalSize().y * 0.5f
	});

	GameRequestBoard grequest = GameRequestBoard(game_rand());
	grequest.setPosition(sf::Vector2f{
		window.getSize().x - grequest.background.getLocalBounds().width - 50.0f,
		window.getSize().y * 0.5f - grequest.background.getLocalBounds().height * 0.5f
		});



	while (window.isOpen())
	{
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
					grequest.SpawnNewRequest();

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
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
				if (g_renderFeedback.hovered == &ggrid) {
					ggrid.selected = g_renderFeedback.hover_cell;
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

			// Window title text edit
			ImGui::InputText("Window title", windowTitle, 255);

			if (ImGui::Button("Update window title")) {
				// this code gets if user clicks on the button
				// yes, you could have written if(ImGui::InputText(...))
				// but I do this to show how buttons work :)
				window.setTitle(windowTitle);
			}
			ImGui::End(); // end window
		}


		//////////////////////////////////////////////////////////////////////////  RENDER
		{
			g_renderFeedback.cursorPos = mouseVecf;
			g_renderFeedback.hovered = nullptr;

			window.clear(bgColor);

			window.draw(background);

			window.draw(ggrid);
			window.draw(grequest);

			
			ImGui::SFML::Render(window);
			window.display();
		}
	}


	ImGui::SFML::Shutdown();

	return 0;
}
