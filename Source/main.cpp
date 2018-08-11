#include "../imgui/imgui.h"
#include "imgui-sfml.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <fstream>
#include <string>

class MyDrawable : public sf::Drawable, public sf::Transformable{
public:
	MyDrawable() {
		m_vertices = sf::VertexArray(sf::LineStrip, 4);
		m_vertices[0].position = sf::Vector2f(10, 0);
		m_vertices[1].position = sf::Vector2f(20, 0);
		m_vertices[2].position = sf::Vector2f(30, 5);
		m_vertices[3].position = sf::Vector2f(40, 2);

		printf("FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUCJK");
		IM_ASSERT(m_texture.loadFromFile("arrow.png"));
		
		m_sprite.setTexture(m_texture);

	}

	sf::Sprite m_sprite;
	sf::Texture m_texture;
	sf::VertexArray m_vertices;

	private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();

		target.draw(m_sprite, states);

		target.draw(m_vertices, states);
	}
};

void ProcessEvents(sf::RenderWindow &window)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
			window.close();
	}
}

int WinMain()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	std::string logfile = "sfml-log.txt";
	std::ofstream file(logfile, std::ios::binary);
	sf::err().rdbuf(file.rdbuf());

	window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.
	sf::Clock deltaClock;


	sf::Color bgColor;
	float color[3] = { 0.f, 0.f, 0.f };

	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	MyDrawable testObj;

	char windowTitle[255] = "ImGui + SFML = <3";
	window.setTitle(windowTitle);

	while (window.isOpen())
	{
		ProcessEvents(window);

		auto mouseVec = sf::Mouse::getPosition(window);
		sf::Vector2f mouseVecf = sf::Vector2f(mouseVec);

		testObj.setPosition(mouseVecf);

		ImGui::SFML::Update(window, deltaClock.restart());
		ImGui::Begin("Sample window"); // begin window

									   // Background color edit
		if (ImGui::ColorEdit3("Background color", color)) {
			// this code gets called if color value changes, so
			// the background color is upgraded automatically!
			bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
			bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
			bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
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

		// Render
		window.clear(bgColor);
		window.draw(shape);

		window.draw(testObj);

		ImGui::SFML::Render(window);
		window.display();
	}


	ImGui::SFML::Shutdown();

	return 0;
}

// sf::Vector2i globalPosition = sf::Mouse::getPosition();
// sf::Vector2i localPosition = sf::Mouse::getPosition(window); 
