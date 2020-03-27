#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include<string>

using namespace sf;
using namespace std;

int theme = 1;
VideoMode videoMode(500, 650);
RenderWindow window(videoMode, "SUDOKU", Style::Close);

/**
* Time formatting and manipulation
*
*/
struct timer {
	sf::Clock mC;
	float runTime;
	bool bPaused;
	timer() {
		bPaused = false;
		runTime = 0;
		mC.restart();
	}
	void Reset() {
		mC.restart();
		runTime = 0;
		bPaused = false;
	}
	void Resume() {
		if (bPaused) {
			mC.restart();
		}
		bPaused = false;
	}
	void Pause() {
		if (!bPaused) {
			runTime += mC.getElapsedTime().asSeconds();
		}
		bPaused = true;
	}
	float GetElapsedSeconds() {
		if (!bPaused) {
			return runTime + mC.getElapsedTime().asSeconds();
		}
		return runTime;
	}
	string getElapsedTime()
	{
		int seconds = int(GetElapsedSeconds());
		int minutes = seconds / 60;
		int hours = minutes / 60;
		minutes = (seconds - hours * 3600) / 60;
		seconds -= (minutes * 60 + hours * 3600);

		string time;
		if (hours > 1)
			time = to_string(hours) + ":" + to_string(minutes) + ":" + to_string(seconds);
		else
			time = to_string(minutes) + ":" + to_string(seconds);

		return time;
	}
} timer;

/**
* Stores the data of a move during the game for later use
*
*/
struct gameMove
{
	Vector2i index;
	int number = 0;
} lastMove;

/**
* Controls the process of drawing both pen and numbers on the grid and figuring out the index of the cell which the pen refers to
*
*/
struct positionSystem
{
	const Vector2f NUMBERS_MARGIN = { 41,125 };
	const Vector2f POINTER_MARGIN = { 32,128 };
	const Vector2f ICON_MARGIN = { 57,47 };

	Vector2f margin, factor, position;
	Vector2i index;

	Vector2f generate_position()
	{
		float x = margin.x + (index.x * factor.x),
			y = margin.y + (index.y * factor.y);

		position = { x,y };
		return position;
	}

	Vector2i generate_index()
	{
		int x = (position.x - margin.x) / factor.x,
			y = (position.y - margin.y) / factor.y;

		index = { x,y };
		return index;
	}

};

struct pauseScreen
{
	Vector2f size, position;
	Color shadeColor = { 35,35,35,200 };

	Font font;

	
	void create(Vector2f size, Vector2f position)
	{
		this->size = size;
		this->position = position;

		//Shade
		RectangleShape shade;
		shade.setSize(this->size);
		shade.setPosition(this->position);
		shade.setFillColor(shadeColor);

		//Icon
		Texture texturePause;
		texturePause.loadFromFile("Assets/Images/pause.png");
		Sprite spritePause;
		spritePause.setTexture(texturePause);
		spritePause.setOrigin(spritePause.getGlobalBounds().width / 2, spritePause.getGlobalBounds().height / 2);
		spritePause.setPosition(window.getSize().x / 2.0f, window.getSize().y / 3.0f);

		
		//Text
		Text pauseText;
		pauseText.setString("PAUSED");
		pauseText.setFont(font);
		pauseText.setFillColor(Color::White);
		pauseText.setCharacterSize(84);
		pauseText.setOrigin(pauseText.getGlobalBounds().width / 2, pauseText.getGlobalBounds().height / 2);
		pauseText.setPosition(window.getSize().x / 2.0f, spritePause.getPosition().y + spritePause.getGlobalBounds().height);


		window.draw(shade);
		window.draw(spritePause);
		window.draw(pauseText);
	}
};

struct imageButton
{
	//fields
	Texture textureStates[3];
	
	Sprite spriteState;

	Vector2f buttonSize = { 0,0 }, buttonPosition = {0,0};

	bool clicked = false;

	//functions
	bool mouseHover(Vector2f pos, Vector2f size)
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window); //mouse position according to the window

		bool in_x_area = ((mouse_pos.x >= pos.x) & (mouse_pos.x <= (pos.x + size.x))), //mouse is in the y axis
			in_y_area = ((mouse_pos.y >= pos.y) & (mouse_pos.y <= (pos.y + size.y)));  //mouse is in the x axis

		return in_x_area && in_y_area;
	}

	bool mouseDown(Vector2f pos, Vector2f size)
	{
		return mouseHover(pos, size) && sf::Mouse::isButtonPressed(sf::Mouse::Left);
	}

	void init(sf::Vector2f pos, string btnId)
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
		
		if(mouseDown(spriteState.getPosition(), buttonSize))
		{
			spriteState.setTexture(textureStates[2]);
			clicked = true;
		}
		
		window.draw(spriteState);
	}
};



#pragma region GLOBAL VARIABLES

const int MOVE_SLEEP_DURATION = 175;

int totalMoves = 0; //total moves taken in the game
int correctMoves = 0; //correct moves taken in the game
int falseMoves = 0; //false moves taken, (game will be over after 3 false moves)

//Used to trace the pen in order to move it using the keyboard arrows and knowing the current position 
Vector2i penTracer = { 0,0 };

int completedCells = 41;

bool paused = false;

string difficulty; //Game difficulty, (Easy, Normal, Hard, Expert)

#pragma endregion


/**
* Evaluates if a move is correct or not comparing to the completed sudoku template
*
* @param index The index of the cell.
* @param source The reference to check whether a correct move or not.
* @param num The number played.
*
* @return true if the cell is either empty or contains a false number from a previous move
*/
bool correctMove(Vector2i index, int source[9][9], int num)
{
	return source[index.x][index.y] == num;
}

/**
* Evaluates either a specific cell is modifiable or not
*
* @param index The index of the cell.
* @param solvedSource The solved sudoku grid.
* @param unsolvedSource The unsolved sudoku grid.
*
* @return true if the cell is modifiable
*/
bool modifiableCell(Vector2i index, int solvedSource[9][9], int unsolvedSource[9][9])
{
	bool emptyCell = unsolvedSource[index.x][index.y] == 0;
	bool wrongNumber = !correctMove(index, solvedSource, solvedSource[index.x][index.y]);

	return emptyCell || wrongNumber;
}

/**
* Removes an entry entered by the used
*
* @param source The reference grid in which we are going to remove the entry
* @param index The index of the entry
*
*/
void remove(Text source[9][9], Vector2i index)
{
	source[index.x][index.y].setString("0");
}

/**
* Undo the last move taken
*
* @param source The reference grid in which we are going to remove the last move
* @param move The move to be removed
*
*/
void undo(Text source[9][9], gameMove move)
{
	remove(source, move.index);
}

/**
* Plays a sound from a sound buffer then awaits for a specific time
*
* @param name The name of the audio file
* @param sleepDuration the duration to be awaited
*/
void playSound(string name, Time sleepDuration)
{
	string directory = "Assets/Audio/SoundEffects/" + name;
	SoundBuffer buffer;
	buffer.loadFromFile(directory);

	Sound sound(buffer);
	sound.play();

	sleep(sleepDuration);
}

/**
* Updates a text object with a specific string
*
* @param textObj The text object to be updates
* @param initialString Data that does not change each frame
* @param newData Data that does change each frame
*/
void updateText(Text& textObj, string initialString, string newData)
{
	string str = initialString + newData;
	textObj.setString(str);
}


int main()
{
	int unsolvedTemplate[9][9] =
	{
		{1,0,3,0,0,0,0,0,0},
		{0,2,0,4,5,6,0,8,9},
		{1,2,3,4,0,6,7,0,0},
		{1,0,3,0,0,0,0,8,9},
		{0,2,3,4,5,6,0,0,0},
		{1,2,0,4,0,6,0,8,9},
		{0,2,3,0,5,0,7,0,0},
		{1,0,0,0,5,0,0,0,9},
		{1,2,3,0,0,6,0,8,0}
	};

	int solvedTemplate[9][9] =
	{
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9},
		{1,2,3,4,5,6,7,8,9}
	};

#pragma region Textures
	Texture textureBackground;
	textureBackground.loadFromFile("Assets/Images/Background.png");
	Sprite spriteBackground(textureBackground);
	spriteBackground.setPosition({ 0, 0 });
	
	Texture textureGrid;
	textureGrid.loadFromFile("Assets/Images/grid.png");
	Sprite spriteGrid(textureGrid);
	spriteGrid.setPosition({25, 120});
#pragma endregion

#pragma region Text
	float textScale = 0.7f,
		positionY = 580.0f;

	//Fonts (Product Sans)
	Font googleBlack, googleBold, googleRegular;
	googleBlack.loadFromFile("Assets/Fonts/ProductSans-Black.ttf");
	googleBold.loadFromFile("Assets/Fonts/ProductSans-Bold.ttf");
	googleRegular.loadFromFile("Assets/Fonts/ProductSans-Regular.ttf");


	Text unsolvedTemplateText[9][9]; //Sudoku numbers to be drawn

	Text difficultyText; //Game difficulty
	difficultyText.setString("NORMAL");
	difficultyText.setFont(googleRegular);
	difficultyText.setScale({ textScale,textScale });
	difficultyText.setPosition(25, positionY);

	Text falseMovesText;
	falseMovesText.setFont(googleBold);
	falseMovesText.setFillColor(Color(255, 128, 128));
	falseMovesText.setScale({ textScale,textScale });
	falseMovesText.setPosition(160, positionY);

	Text timeText; //Game time
	timeText.setString("TIME: 01:24");
	timeText.setFont(googleRegular);
	timeText.setScale({ textScale,textScale });
	timeText.setPosition(380, positionY);
#pragma endregion


#pragma region Buttons
	string idButtons[5] = { "Menu", "Hint", "Pause", "Undo", "Settings" };
	imageButton buttons[5];
#pragma endregion
	
	/*************************
	*******SUDOKU PEN*********
	**************************/
	RectangleShape pen({ 35,35 });
	//Pen styling
	pen.setFillColor(Color::Transparent);
	pen.setOutlineColor(Color::White);
	pen.setOutlineThickness(3);

	/************************************************************************
	*******COPYING THE UNSOLVED ARRAY IN ORDER TO DRAW IT EACH FRAME*********
	*************************************************************************/
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			int num = unsolvedTemplate[i][j];
			//setting string
			unsolvedTemplateText[i][j].setString(to_string(num));
			//setting font (Google Sans)
			unsolvedTemplateText[i][j].setFont(googleBlack);
			//setting position
			positionSystem posNumber;
			posNumber.margin = posNumber.NUMBERS_MARGIN;
			posNumber.factor = { 50,50 };
			posNumber.index = { i,j };
			Vector2f pos = posNumber.generate_position();

			unsolvedTemplateText[i][j].setPosition(pos);
		}
	}

	//Game loop
	while (window.isOpen())
	{
		//Closing the game
		Event evnt{};
		while (window.pollEvent(evnt))
		{
			switch (evnt.type)
			{
			case Event::Closed:
				window.close();
				break;

			case Event::LostFocus:
				paused = true;
				timer.Pause();
				break;
			case Event::GainedFocus:
				paused = false;
				timer.Resume();
				break;
			default: break;
			}
		}

		/************************
		*****UPDATING LABELS*****
		*************************/
#pragma region UPDATING Text

		updateText(falseMovesText, "FALSE MOVES: ", to_string(falseMoves));

		//updating the time text unless the game is paused
		if (!paused)
			updateText(timeText, "TIME: ", timer.getElapsedTime());
#pragma endregion


		/************************************************
		 ****************KEYBOARD INPUT******************
		 ***********************************************/

		//Stop all kinds of input when the game is paused
		if (!paused)
		{
#pragma region KEYBOARD INPUT

			/********************************************************
			 *****HANDLING ARROWS INPUT IN ORDER TO MOVE THE PEN*****
			 *******************************************************/
#pragma region PEN MOVING INPUT
			bool rightPressed = Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D),
				leftPressed = Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A),
				upPressed = Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W),
				downPressed = Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S),
				directionPressed = rightPressed | leftPressed | upPressed | downPressed;

			if (directionPressed)
			{
				//right arrow
				if (rightPressed)
				{
					//moves the pen to the start of the grid if the pen reaches the end of the grid
					if (penTracer.x == 8 && penTracer.y == 8)
					{
						penTracer.x = 0;
						penTracer.y = 0;
					}
					//moves the pen to the next row if the pen reaches the end of the current row
					else if (penTracer.x == 8)
					{
						penTracer.x = 0;
						penTracer.y++;
					}
					//moves the pen horizontally towards the right
					else
					{
						penTracer.x++;
					}
				}
				//left arrow
				if (leftPressed)
				{
					//moves the pen to the end of the grid if the pen reaches the start of the grid
					if (penTracer.x == 0 && penTracer.y == 0)
					{
						penTracer.x = 8;
						penTracer.y = 8;
					}
					//moves the pen to the previous row if the pen reaches the start of the current row
					else if (penTracer.x == 0)
					{
						penTracer.x = 8;
						penTracer.y--;
					}
					//moves the pen horizontally towards the left
					else
					{
						penTracer.x--;
					}
				}
				//up arrow
				if (upPressed)
				{
					//Does nothing if the pen is already at the start of the column
					if (penTracer.y == 0)
					{
					}
					//moving up the column of the grid
					else
					{
						penTracer.y--;
					}
				}
				//down arrow
				if (downPressed)
				{
					//Does nothing if the pen is already at the end of the column
					if (penTracer.y == 8)
					{
					}
					//moving up the column of the grid
					else
					{
						penTracer.y++;
					}
				}

				//sound playing when moving the pen
				playSound("move.wav", milliseconds(MOVE_SLEEP_DURATION));
			}

			//Setting pen position
			positionSystem posPen;
			posPen.margin = posPen.POINTER_MARGIN;
			posPen.factor = { 50,50 };
			posPen.index = { penTracer.x, penTracer.y };
			Vector2f pos = posPen.generate_position();
			pen.setPosition(pos);
#pragma endregion

			/********************************************************
			 ********HANDLING NUMBERS INPUT IN ORDER TO WRITE********
			 *******************************************************/
#pragma region NUMBERS INPUT
			int numPressed = 0;

			if (Keyboard::isKeyPressed(Keyboard::Num1) || Keyboard::isKeyPressed(Keyboard::Numpad1))
				numPressed = 1;
			if (Keyboard::isKeyPressed(Keyboard::Num2) || Keyboard::isKeyPressed(Keyboard::Numpad2))
				numPressed = 2;
			if (Keyboard::isKeyPressed(Keyboard::Num3) || Keyboard::isKeyPressed(Keyboard::Numpad3))
				numPressed = 3;
			if (Keyboard::isKeyPressed(Keyboard::Num4) || Keyboard::isKeyPressed(Keyboard::Numpad4))
				numPressed = 4;
			if (Keyboard::isKeyPressed(Keyboard::Num5) || Keyboard::isKeyPressed(Keyboard::Numpad5))
				numPressed = 5;
			if (Keyboard::isKeyPressed(Keyboard::Num6) || Keyboard::isKeyPressed(Keyboard::Numpad6))
				numPressed = 6;
			if (Keyboard::isKeyPressed(Keyboard::Num7) || Keyboard::isKeyPressed(Keyboard::Numpad7))
				numPressed = 7;
			if (Keyboard::isKeyPressed(Keyboard::Num8) || Keyboard::isKeyPressed(Keyboard::Numpad8))
				numPressed = 8;
			if (Keyboard::isKeyPressed(Keyboard::Num9) || Keyboard::isKeyPressed(Keyboard::Numpad9))
				numPressed = 9;

			//The actual writing and playing sound

			if (numPressed != 0) //A number between (1-9) entered
			{
				Vector2i index = penTracer;

				if (modifiableCell(index, solvedTemplate, unsolvedTemplate))
				{
					unsolvedTemplateText[index.x][index.y].setString(to_string(numPressed));
					unsolvedTemplateText[index.x][index.y].setFont(googleRegular);

					//coloring the number to indicate either correct or incorrect move
					if (correctMove(index, solvedTemplate, numPressed))
					{
						unsolvedTemplateText[index.x][index.y].setFillColor(Color(0, 0, 180));
						completedCells++;
					}
					else
					{
						unsolvedTemplateText[index.x][index.y].setFillColor(Color(255, 0, 0));
						falseMoves++;
					}

					//record the move in case the user wants to undo
					lastMove.index = index;
					lastMove.number = numPressed;

					//playing sound
					playSound("fill.wav", milliseconds(350));
				}
			}
#pragma endregion


			/********************************************************
			 ********************DELETING A MOVE*********************
			 *******************************************************/
#pragma region 4-DELETE
			if (Keyboard::isKeyPressed(Keyboard::Delete)) //user pressed delete in order to remove a specific move
			{
				//Doesn't delete the predefined numbers
				if (modifiableCell(penTracer, solvedTemplate, unsolvedTemplate))
				{
					remove(unsolvedTemplateText, penTracer);

					//plays the undo sound
					playSound("undo.wav", milliseconds(250));
					//decrease completed cells 
				}
			}
#pragma endregion
			/********************************************************
			 ********************UN-DOING A MOVE*********************
			 *******************************************************/
#pragma region 5-UNDO
			if (Keyboard::isKeyPressed(Keyboard::Z) && evnt.key.control) //user pressed ctrl+z in order to undo the last move
			{
				//solves the issue of removing number at (0,0) at the beginning of the game
				if (modifiableCell(lastMove.index, solvedTemplate, unsolvedTemplate))
				{
					undo(unsolvedTemplateText, lastMove);

					//plays the undo sound
					playSound("undo.wav", milliseconds(250));


					//decrease completed cells 
				}
			}
#pragma endregion
#pragma endregion
		}

		/********************************************************
		 *******UPDATING AND SWITCHING BUFFERS*******************
		 *******************************************************/
#pragma region UPDATING AND SWITCHING BUFFERS

		 //clearing the window
		window.clear();


		window.draw(spriteBackground);		
		window.draw(spriteGrid);

		//buttons
		positionSystem posButtons;
		posButtons.margin = { 25, 25 };
		posButtons.factor = { 80,70 };
		for (int i = 0; i < 5; i++)
		{
			posButtons.index = { i, 0 };
			if (i > 0)
				posButtons.margin.x += 12;
			buttons[i].init(posButtons.generate_position(), idButtons[i]);
		}
		
		//Pen
		window.draw(pen);
		//Numbers
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (unsolvedTemplateText[j][i].getString() == "0")
					continue;
				window.draw(unsolvedTemplateText[j][i]);
			}

		}

		//texts
		window.draw(timeText);
		window.draw(falseMovesText);
		window.draw(difficultyText);

		//Draw an overlay shader to indicate a paused game
		if (paused)
		{
			pauseScreen pauseScreen;
			pauseScreen.font = googleBlack;
			pauseScreen.shadeColor = Color(20, 30, 48, 200);
			Vector2f size = { static_cast<float>(textureGrid.getSize().x), static_cast<float>(textureGrid.getSize().y) };
			pauseScreen.create(size,
				{ 25, 120 });
		}

		window.display();

#pragma endregion
	}


	return 0;
}