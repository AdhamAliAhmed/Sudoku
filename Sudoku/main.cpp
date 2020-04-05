#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include<string>

struct glyphButton;
struct button;
struct gif;

using namespace sf;
using namespace std;

const int WIDTH = 500, HEIGHT = 650;

int theme = 1;
VideoMode videoMode(WIDTH, HEIGHT);
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
	const Vector2f PEN_MARGIN = { 32.6,127.5 };
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
};

#pragma region object center
Vector2f center(Text text)
{
	FloatRect textRect = text.getLocalBounds();

	return {textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f};
}
Vector2f center(Sprite sprite)
{
	return {sprite.getLocalBounds().width / 2.0f, sprite.getLocalBounds().height / 2.0f};
}
Vector2f center(RectangleShape rect)
{
	return {rect.getSize().x/2.0f, rect.getSize().y/2.0f};
}
Vector2f center(RenderWindow &window)
{
	return {window.getSize().x / 2.0f, window.getSize().y / 2.0f}; 
}
Vector2f center(gif gif);
#pragma endregion


struct button
{
	//components
	RectangleShape frame;
	Text text;

	//properties
	Vector2f buttonPosition, buttonSize;
	Color idleColor, hoverColor, downColor, textColor;
	Font textFont;

	short state = 0;
	
	bool mouseHover()
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window); //mouse position according to the window
		FloatRect bounds = frame.getGlobalBounds(); //rectangle bounds
		
		bool xInBounds = ((mouse_pos.x >= bounds.left) & (mouse_pos.x <= (bounds.left + bounds.width))), //mouse is in the y axis
			yInBounds = ((mouse_pos.y >= bounds.top) & (mouse_pos.y <= (bounds.top + bounds.height)));  //mouse is in the x axis

		return xInBounds && yInBounds;
	}
	
	bool mouseDown()
	{
		return mouseHover() && sf::Mouse::isButtonPressed(sf::Mouse::Left);
	}

	void create(sf::Vector2f pos, string text)
	{		
		buttonPosition = pos;
		this -> text.setString(text);

		frame.setSize(buttonSize);	
		frame.setOrigin(center(frame));
		frame.setPosition(buttonPosition);

		if(mouseHover())
		{
			state = 1;
			
			frame.setFillColor(hoverColor);		
			
			if(mouseDown())
			{
				state = 2;
				
				frame.setFillColor(downColor);
				frame.setOutlineColor(Color(200,200,200));

				frame.setScale(0.97f,0.97f);
			}

		}
		else
		{
			state = 0;
			
			frame.setFillColor(idleColor);
		}

		this -> text.setFont(textFont);
		
		this -> text.setOrigin(center(this -> text));

		FloatRect frameRect = frame.getGlobalBounds();
		this -> text.setPosition(frameRect.left + frameRect.width /2.0f, 
			frameRect.top + frameRect.height /2.0f);
		this -> text.setFont(textFont);
		this -> text.setCharacterSize(19);
		this -> text.setFillColor(textColor);
		
		window.draw(frame);
		window.draw(this -> text);
	}
};

bool clicked(button btn)
{
	if(btn.state == 2)
		return true;
	return false;
}


bool mouseHover(Sprite sprite){
	sf::Vector2i mouse_pos = sf::Mouse::getPosition(window); //mouse position according to the window

	bool foo = sprite.getGlobalBounds().contains({static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)});

	return foo;
}
bool mouseDown(Sprite sprite){
	return mouseHover(sprite) && sf::Mouse::isButtonPressed(sf::Mouse::Left);
}
struct glyphButton
{
	//Variables
	Vector2f buttonSize = { 50, 50 },
	buttonPosition = { 0,0 };
	
	Texture textureSheet;
	
	IntRect tracer{0,0, 50,50};
	Sprite spriteState;

	short state = 0;
	
	//functions
	void create(sf::Vector2f pos, string id)
	{
		buttonPosition = pos;
		spriteState.setPosition(buttonPosition);
		
		string directory = "Assets/buttons/" + id + ".png";
		textureSheet.setSmooth(true);
		textureSheet.loadFromFile(directory);

		spriteState.setTexture(textureSheet);
		
		if(mouseHover(spriteState))
		{
			tracer.left = 50;

			state = 1;
			
			if(mouseDown(spriteState))
			{
				tracer.left = 100;

				state = 2;
			}
		}
		else
		{
			tracer.left = 0;

			state = 0;
		}
		
		spriteState.setTextureRect(tracer);
		
		window.draw(spriteState);
	}
};
bool clicked(glyphButton btn)
{
	if(btn.state == 2)
		return true;
	return false;
}

IntRect gifTracer{0,0,200,200};
struct gif
{
	Texture textureSheet;
	Sprite spriteFrame;

	Vector2f gifPosition;
	
	int numFrames = 0;
	
	Vector2i frameSize {0,0};

	
	void animate(string directory, bool repeat = false)
	{
		textureSheet.setSmooth(true);
		textureSheet.loadFromFile(directory);
		spriteFrame.setPosition(gifPosition);

		spriteFrame.setTexture(textureSheet);
		spriteFrame.setTextureRect(gifTracer);

		if(gifTracer.left == frameSize.y * (numFrames - 1))
		{
			gifTracer.left = 0;
		}
		else
		{
			gifTracer.left += frameSize.y;
			sleep(milliseconds(30));
		}

		
		window.draw(spriteFrame);
	}
};
Vector2f center(gif gif)
{
	return {gif.frameSize.x / 2.0f, gif.frameSize.y / 2.0f};
}


struct pauseScreen
{
	bool dueToInactivity = false;
	
	Vector2f position;

	RectangleShape shade;
	Color shadeColor = { 20,20,20,150 };

	Texture textureContent;
	Sprite spriteContent;

	gif dinoGif;
	
	Text text;

	Font textFont;
	
	glyphButton btnResume;

	
	void create(bool dueToInactivity)
	{
		float margin = 20.0f;

		//Shade
		shade.setPosition({0,0});
		shade.setSize({WIDTH, HEIGHT});
		shade.setFillColor(shadeColor);
		
		//Content
		textureContent.loadFromFile("Assets/Menus/pause-content.png");
		spriteContent.setTexture(textureContent);
		spriteContent.setOrigin(center(spriteContent));
		spriteContent.setPosition(center(window));

		//gif
		dinoGif.frameSize = {200,200};
		dinoGif.numFrames = 10;
		dinoGif.spriteFrame.setOrigin(center(dinoGif));
		dinoGif.gifPosition = {center(window).x, spriteContent.getGlobalBounds().top + 110};

		//Text
		text.setString("Paused");
		text.setFont(textFont);
		text.setFillColor(Color(52,193,206));
		text.setCharacterSize(64);
		text.setOrigin(center(text));
		text.setPosition(center(window).x, spriteContent.getGlobalBounds().top + 220);
		
		
		//Button
		btnResume.spriteState.setOrigin(25,25);
		btnResume.spriteState.setScale(0.8,0.8);
		
		window.draw(shade);
		window.draw(spriteContent);
		dinoGif.animate("Assets/Sprite_sheets/dino_dance.png", true);
		window.draw(text);
		FloatRect frameContent = spriteContent.getGlobalBounds();
		if(!dueToInactivity)
			btnResume.create({center(window).x, frameContent.top + 320.0f}, "resume");
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

bool pausedDueToInactivity = false;
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

void pause(bool dueToInactivity)
{
	timer.Pause();
	paused = true;
	pausedDueToInactivity = dueToInactivity;
}

void resume()
{
	timer.Resume();
	paused = false;
	pausedDueToInactivity = false;
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

#pragma region Images
	Texture textureBackground;
	textureBackground.loadFromFile("Assets/Images/Background9.png");
	Sprite spriteBackground = Sprite(textureBackground);
	
	Texture textureGrid;
	textureGrid.loadFromFile("Assets/Images/grid.png");
	Sprite spriteGrid(textureGrid);
	spriteGrid.setPosition({25, 120});
#pragma endregion

#pragma region Top glyph buttons
	string btnIds[5] = {"menu", "hint", "pause", "undo", "settings"};
	glyphButton buttons[5];
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
	difficultyText.setScale({ textScale,textScale});
	difficultyText.setPosition(25, positionY);

	Text falseMovesText; //Number of false moves
	falseMovesText.setFont(googleBold);
	falseMovesText.setFillColor(Color(255, 128, 128));
	falseMovesText.setScale({ textScale,textScale });
	falseMovesText.setPosition(160, positionY);

	Text timeText; //Game time
	timeText.setFont(googleRegular);
	timeText.setScale({ textScale,textScale });
	timeText.setPosition(380, positionY);
#pragma endregion

#pragma region Sudoku Pens
	RectangleShape pen({ 35,35 });
	RectangleShape shadowPen({ 35, 35 });

	//Styling
		//Main pen
	pen.setFillColor(Color::Transparent);
	pen.setOutlineColor(Color::White);
	pen.setOutlineThickness(3);
	
		//Shadow pen
	bool drawShadowPen = false;
	shadowPen.setFillColor(Color::Transparent);
	shadowPen.setOutlineColor(Color(255,255,255,90));
	shadowPen.setOutlineThickness(3);
#pragma endregion


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
		Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case Event::Closed:
				window.close();
				break;
			case Event::LostFocus:
				if(!paused)
					pause(true);
				break;
			case Event::GainedFocus:
				if(paused && pausedDueToInactivity)
					resume();
				break;
			default: break;
			}
		}

		/************************
		*****UPDATING LABELS*****
		*************************/
#pragma region UPDATING Labels

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
			posPen.margin = posPen.PEN_MARGIN;
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
			if (Keyboard::isKeyPressed(Keyboard::Z) && event.key.control) //user pressed ctrl+z in order to undo the last move
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


#pragma region MOUSE INPUT
			Vector2i mousePos = Mouse::getPosition(window);
			if (spriteGrid.getGlobalBounds().contains(mousePos.x, mousePos.y))
			{
				system("CLS");
				
				Vector2i index = { (mousePos.x - 25) / 50,(mousePos.y - 120) / 50 };

				cout << index.x << ',' << index.y << endl;

				positionSystem shadowPos;
				shadowPos.margin = shadowPos.PEN_MARGIN;
				shadowPos.factor = { 50,50 };
				shadowPos.index = index;
				
				shadowPen.setPosition(shadowPos.generate_position());

				drawShadowPen = true;

				if (Mouse::isButtonPressed(Mouse::Left))
				{
					penTracer = index;
					pen.setPosition(shadowPos.generate_position());
				}
			}
			else
			{
				drawShadowPen = false;
			}

			
#pragma endregion

#pragma region Buttons handling
		if(clicked(buttons[2]))
		{
			pause(false);
		}
		if(clicked(buttons[3]))
		{
			undo(unsolvedTemplateText, lastMove);
		}
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

		//Pens
		window.draw(pen);
		if(drawShadowPen)
			window.draw(shadowPen);

		
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


		//Top glyph buttons positioning and drawing
		positionSystem posButtons;
		posButtons.margin = { 25, 25 };
		posButtons.factor = { 50,45 };
		
		for (int i = 0; i < 5; i++)
		{
			posButtons.index = { i, 0 };
			if (i > 0)
			{
				posButtons.margin.x += 50;
			}

			buttons[i].create(posButtons.generate_position(), btnIds[i]);
		}
		
		//Draw an overlay shader to indicate a paused game
		if (paused)
		{
			pauseScreen pauseScreen;
			pauseScreen.textFont = googleRegular;
			pauseScreen.create(pausedDueToInactivity);

			if(clicked(pauseScreen.btnResume))
				resume();
		}
		
		window.display();

#pragma endregion
	}


	return 0;
}