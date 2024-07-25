#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "solver.hpp"

class Button {
	private:
		sf::Vector2f position;
		float sizeX;
		float sizeY;
		std::string text;
		int fontSize;
		sf::Color color;
		sf::Font font;
	
	public:
		Button(sf::Vector2f pos, float _sizeX, float _sizeY, std::string _text, int _fontSize);
		bool canPress(sf::Vector2f pos);
		void drawButton(sf::RenderTarget& target) const;
};

bool Button::canPress(sf::Vector2f pos) {
	float halfX = sizeX / 2.0f;
	float halfY = sizeY / 2.0f;

	if (pos.x > position.x + halfX) {
		color = sf::Color::White;
		return false;
	}
	if (pos.x < position.x - halfX) {
		color = sf::Color::White;
		return false;
	}
	if (pos.y > position.y + halfY) {
		color = sf::Color::White;
		return false;
	}
	if (pos.y < position.y - halfY) {
		color = sf::Color::White;
		return false;
	}

	color = { 200,200,200 };

	return true;
}

Button::Button(sf::Vector2f pos, float _sizeX, float _sizeY, std::string _text, int _fontSize) :
	position(pos), sizeX(_sizeX), sizeY(_sizeY), text(_text), fontSize(_fontSize) {

	font.loadFromFile("THSarabunNew.ttf");
	color = sf::Color::White;
}

void Button::drawButton(sf::RenderTarget& target) const {
	sf::RectangleShape button;
	button.setSize({ sizeX, sizeY });
	button.setFillColor(color);
	button.setOrigin({ sizeX / 2.0f , sizeY / 2.0f });
	button.setPosition(position);

	sf::Text textObj;
	textObj.setFont(font);
	textObj.setString(text);
	textObj.setCharacterSize(fontSize);
	textObj.setFillColor(sf::Color::Blue);
	textObj.setPosition(position);
	textObj.setOrigin({0.5f , 0.5f});

	target.draw(button);
	target.draw(textObj);
}