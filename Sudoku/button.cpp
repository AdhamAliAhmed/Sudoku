#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include<string>

using namespace sf;
using namespace std;

namespace sudoku
{
	struct imageButton
	{
		const short IDLE = 0, HOVER = 1, PRESSED = 2;

		//fields
		Texture textureStates[3];
		Sprite spriteState;
		Vector2f buttonSize = { 0,0 },
			buttonPosition = { 0,0 };
		bool clicked = false;

		//functions
		bool mouseHover(Vector2f pos, Vector2f size)
		{
			sf::Vector2i mouse_pos = sf::Mouse::getPosition(); //mouse position

			bool in_x_area = ((mouse_pos.x >= pos.x) & (mouse_pos.x <= (pos.x + size.x))), //mouse is in the y axis
				in_y_area = ((mouse_pos.y >= pos.y) & (mouse_pos.y <= (pos.y + size.y)));  //mouse is in the x axis

			return in_x_area && in_y_area;
		}

		bool mouseDown(Vector2f pos, Vector2f size)
		{
			return mouseHover(pos, size) && sf::Mouse::isButtonPressed(sf::Mouse::Left);
		}

		void init(sf::Vector2f pos, string btnId, RenderWindow window)
		{
			string directory = "Assets/Buttons/" + btnId + "/" + to_string(theme) + "/";
			buttonPosition = pos;

			//textures loading
			textureStates[0].loadFromFile(directory + "normal.png");
			textureStates[1].loadFromFile(directory + "hover.png");
			textureStates[2].loadFromFile(directory + "down.png");

			//sprite
			spriteState.setTexture(textureStates[0]);
			spriteState.setPosition(buttonPosition);
			//size
			buttonSize.x = static_cast<float>(spriteState.getLocalBounds().width);
			buttonSize.y = static_cast<float>(spriteState.getLocalBounds().height);

			if (mouseHover(spriteState.getPosition(), buttonSize))
				spriteState.setTexture(textureStates[1]);

			if (mouseDown(spriteState.getPosition(), buttonSize))
			{
				spriteState.setTexture(textureStates[2]);
				//cout << "Clicked" << endl;

				clicked = true;

				sleep(milliseconds(200));
			}

			window.draw(spriteState);
		}
	};
}