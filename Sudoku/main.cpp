#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include<string>

struct glyphButton;
struct button;
struct gif;

using namespace sf;
using namespace std;

#pragma region functions decleration
#pragma endregion


const int WIDTH = 500, HEIGHT = 650;

int theme = 1;
VideoMode videoMode(WIDTH, HEIGHT);
RenderWindow window(videoMode, "SUDOKU", Style::Close);

/**
* Time formatting and manipulation
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
	/**
	* Resets the clock
	*/
	void reset() {
		mC.restart();
		runTime = 0;
		bPaused = false;
	}
	/**
	* Resumes the clock
	*/
	void resume() {
		if (bPaused) {
			mC.restart();
		}
		bPaused = false;
	}
	/**
	* Pauses the clock
	*/
	void pause() {
		if (!bPaused) {
			runTime += mC.getElapsedTime().asSeconds();
		}
		bPaused = true;
	}
	/**
	* Returns the elapsed seconds
	*/
	float getElapsedSeconds() {
		if (!bPaused) {
			return runTime + mC.getElapsedTime().asSeconds();
		}
		return runTime;
	}
	/**
	* Return a string representation of the elapsed time
	*/
	string getElapsedTime()
	{
		int seconds = int(getElapsedSeconds());
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

	/**
	* Generates a position on the screen using three factors: margin, factor and index
	*/
	Vector2f generatePosition()
	{
		float x = margin.x + (index.x * factor.x),
			y = margin.y + (index.y * factor.y);
		
		position = {x, y};
		return position;
	}
};

/**
* Creates a pen instance with it default properties
* @param pen The pen to be used.
* @param type The type of the pen, either primary pen, shadow pen or a highlighter.
*/
void createPenInstance(RectangleShape &pen, short type);

struct highlightingSystem
{
	Vector2i index = {0,0};

	/**
	* Highlights the row of a specific cell in the sudoku grid
	* @param y the y index of the cell
	* @param posObj The PositionSystem object that positions the pointers.
	*/
	void highlightRow(int y, positionSystem posObj)
	{
		for (int i = 0; i < 9; i++)
		{
			RectangleShape rect;
			createPenInstance(rect,2);
			
			posObj.index = {i , y};
			rect.setPosition(posObj.generatePosition());
			if(posObj.index != index)
				window.draw(rect);
		}
	}

	/**
	* Highlights the column of a specific cell in the sudoku grid
	* @param x the x index of the cell
	* @param posObj The PositionSystem object that positions the pointers.
	*/
	void highlightColumn(int x, positionSystem posObj)
	{
		for (int i = 0; i < 9; i++)
		{		
			RectangleShape rect;
			createPenInstance(rect, 2);
			
			posObj.index = {x , i};
			rect.setPosition(posObj.generatePosition());
			if(posObj.index != index)
				window.draw(rect);
		}
	}

	/**
	* Highlights the block that a specific cell belongs to in the sudoku grid
	* @param index the index of the cell
	* @param posObj The PositionSystem object that positions the pointers.
	*/
	void highlightBlock(Vector2i index, positionSystem posObj)
	{
		Vector2i indexes[9];
		int row = index.y, column = index.x;

		//determining the block
		//first rank
		if((row >= 0) & (row <= 2))
		{
			if((column >= 0) & (column <= 2)) //first block
			{
				indexes[0] = {0,0};
				indexes[1] = {1,0};
				indexes[2] = {2,0};
				indexes[3] = {0,1};
				indexes[4] = {1,1};
				indexes[5] = {2,1};
				indexes[6] = {0,2};
				indexes[7] = {1,2};
				indexes[8] = {2,2};
			}
			else if((column >= 3) & (column <= 5)) //second block
			{
				indexes[0] = {3,0};
				indexes[1] = {4,0};
				indexes[2] = {5,0};
				indexes[3] = {3,1};
				indexes[4] = {4,1};
				indexes[5] = {5,1};
				indexes[6] = {3,2};
				indexes[7] = {4,2};
				indexes[8] = {5,2};
			}
			else if((column >= 6) & (column <= 8)) //third block
			{
				indexes[0] = {6,0};
				indexes[1] = {7,0};
				indexes[2] = {8,0};
				indexes[3] = {6,1};
				indexes[4] = {7,1};
				indexes[5] = {8,1};
				indexes[6] = {6,2};
				indexes[7] = {7,2};
				indexes[8] = {8,2};
			}
		}
		//second rank
		else if((row >= 3) & (row <= 5))
		{
			if((column >= 0) & (column <= 2)) //fourth block
			{
				indexes[0] = {0,3};
				indexes[1] = {1,3};
				indexes[2] = {2,3};
				indexes[3] = {0,4};
				indexes[4] = {1,4};
				indexes[5] = {2,4};
				indexes[6] = {0,5};
				indexes[7] = {1,5};
				indexes[8] = {2,5};
			}
			else if((column >= 3) & (column <= 5)) //fifth block
			{
				indexes[0] = {3,3};
				indexes[1] = {4,3};
				indexes[2] = {5,3};
				indexes[3] = {3,4};
				indexes[4] = {4,4};
				indexes[5] = {5,4};
				indexes[6] = {3,5};
				indexes[7] = {4,5};
				indexes[8] = {5,5};
			}
			else if((column >= 6) & (column <= 8)) //sixth block
			{
				indexes[0] = {6,3};
				indexes[1] = {7,3};
				indexes[2] = {8,3};
				indexes[3] = {6,4};
				indexes[4] = {7,4};
				indexes[5] = {8,4};
				indexes[6] = {6,5};
				indexes[7] = {7,5};
				indexes[8] = {8,5};
			}
		}
		//third rank
		else if((row >= 6) & (row <= 8))
		{
			if((column >= 0) & (column <= 2)) //seventh block
			{
				indexes[0] = {0,6};
				indexes[1] = {1,6};
				indexes[2] = {2,6};
				indexes[3] = {0,7};
				indexes[4] = {1,7};
				indexes[5] = {2,7};
				indexes[6] = {0,8};
				indexes[7] = {1,8};
				indexes[8] = {2,8};
			}
			else if((column >= 3) & (column <= 5)) //eighth block
			{
				indexes[0] = {3,6};
				indexes[1] = {4,6};
				indexes[2] = {5,6};
				indexes[3] = {3,7};
				indexes[4] = {4,7};
				indexes[5] = {5,7};
				indexes[6] = {3,8};
				indexes[7] = {4,8};
				indexes[8] = {5,8};
			}
			else if((column >= 6) & (column <= 8)) //ninth block
			{
				indexes[0] = {6,6};
				indexes[1] = {7,6};
				indexes[2] = {8,6};
				indexes[3] = {6,7};
				indexes[4] = {7,7};
				indexes[5] = {8,7};
				indexes[6] = {6,8};
				indexes[7] = {7,8};
				indexes[8] = {8,8};
			}
		}

		//highlighting th block
		for (int i = 0; i < 9; i++)
		{
			if(indexes[i] == index)
				continue;		

			RectangleShape rect;
			createPenInstance(rect, 2);

			posObj.index = indexes[i];
			rect.setPosition(posObj.generatePosition());
			if(posObj.index != index)
				window.draw(rect);
		}
	}
	

	/**
	* Highlights the row, column and the block of a specific cell in the sudoku grid for ease of use
	* @param index the index of the cell
	*/
	void highlight(Vector2i index)
	{
		this -> index = index;

		positionSystem posRects;
		posRects.margin = posRects.PEN_MARGIN;
		posRects.factor = {50,50};
		
		highlightRow(this -> index.y, posRects);
		highlightColumn(this -> index.x, posRects);
		highlightBlock(this -> index, posRects);
	}
};

#pragma region object center
/**
* Returns the center of a Text object
* @param text The Text object 
*/
Vector2f center(Text text)
{
	FloatRect textRect = text.getLocalBounds();

	return {textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f};
}
/**
* Returns the center of a sprite object
* @param sprite The Sprite object 
*/
Vector2f center(Sprite sprite)
{
	return {sprite.getLocalBounds().width / 2.0f, sprite.getLocalBounds().height / 2.0f};
}
/**
* Returns the center of a RectangleShape object
* @param rect The RectangleShape object 
*/
Vector2f center(RectangleShape rect)
{
	return {rect.getSize().x/2.0f, rect.getSize().y/2.0f};
}
/**
* Returns the center of a RenderWindow object
* @param window The RenderWindow object 
*/
Vector2f center(RenderWindow &window)
{
	return {window.getSize().x / 2.0f, window.getSize().y / 2.0f}; 
}
/**
* Returns the center of a gif object
* @param gif The gif object 
*/
Vector2f center(gif gif);
#pragma endregion

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
	
	/**
	* Returns true if the mouse cursor hovers over the button
	* @param sprite The sprite that the cursor hovers over.
	*/
	bool mouseHover(Sprite sprite)
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window); //mouse position according to the window

		bool foo = sprite.getGlobalBounds().contains({static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)});

		return foo;
	}
	/**
	* Returns true if the left mouse button is pressed down while the cursor is in hover state
	* @param sprite The sprite that the cursor hovers over.
	*/
	bool mouseDown(Sprite sprite){
		return mouseHover(sprite) && sf::Mouse::isButtonPressed(sf::Mouse::Left);
	}

	/**
	* Renders a glyph button on the screen
	* @param pos The pos for the button to be at.
	* @param id The id of the button. Helps with loading the glyphs
	*/
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
/**
* Returns true if a glyph button is clicked
* @param btn The button to be clicked
*/
bool clicked(glyphButton btn)
{
	if(btn.state == 2)
		return true;
	return false;
}

struct gif
{
	Texture textureSheet;
	Sprite spriteFrame;

	Vector2f gifPosition;

	int numFrames = 0;
	
	Vector2i frameSize {0,0};

	IntRect gifTracer{0,0,0,0};

	
	/**
	* starts animating a gif
	* @param directory The directory of the sprite sheet
	*/
	void animate(string directory)
	{
		textureSheet.setSmooth(true);
		textureSheet.loadFromFile(directory);
		spriteFrame.setPosition(gifPosition);

		spriteFrame.setTexture(textureSheet);
		spriteFrame.setTextureRect(gifTracer);

		gifTracer.width = frameSize.x;
		gifTracer.height = frameSize.y;
		
		int lastFrameLeft = frameSize.y * (numFrames - 1);

		if(gifTracer.left == lastFrameLeft)
			gifTracer.left = 0;
		else
			gifTracer.left += frameSize.y;

		
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

	/**
	* renders a pause menu on the screen
	* @param dueToInactivity determines whether the pause was due to inactivity or not
	*/
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
		dinoGif.animate("Assets/Sprite_sheets/dino_dance.png");
		window.draw(text);
		FloatRect frameContent = spriteContent.getGlobalBounds();
		if(!dueToInactivity)
			btnResume.create({center(window).x, frameContent.top + 320.0f}, "resume");
	}
};

#pragma region GLOBAL VARIABLES
#pragma region Game data
int completedCells = 0; //Number of cells filled with correct (user wins when 81)
int falseMoves = 0; //false moves taken, (game will be over after 3 false moves)
int hints = 0;
string difficulty; //Game difficulty, (Easy, Normal, Hard, Expert)
string gameTime;
#pragma endregion

const int MOVE_SLEEP_DURATION = 150, TAPPING_SLEEP_DURATION = 200;

Color correctMoveColor = Color(0,152,250), falseMoveColor = Color(195,20,50);

//Used to trace the pen in order to move it using the keyboard arrows and knowing the current position 
Vector2i penTracer = { 0,0 };

bool pausedDueToInactivity = false;
bool paused = false;
#pragma endregion

void createPenInstance(RectangleShape &pen, short type)
{
	RectangleShape penObj({35,35});

	penObj.setOutlineThickness(3);

	Color fillColor, outlineColor;
	switch (type)
	{
	case 0:
	fillColor = Color::Transparent;
	outlineColor = Color::White;
		break;;
	case 1:
	fillColor = Color::Transparent;
	outlineColor = Color(255,255,255,90);
		break;
	case 2:
	fillColor = Color(0,218,255, 50);
	outlineColor = Color(0,218,255, 150);
		break;
	default:
		fillColor = Color::Green;
		outlineColor = Color::Blue;

	}
	penObj.setFillColor(fillColor);
	penObj.setOutlineColor(outlineColor);
	
	pen = penObj;
}

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
* Undo the last move taken
*
* @param source The reference grid in which we are going to remove the last move
* @param move The move to be removed
*
*/
void undo(Text source[9][9], gameMove move)
{
	source[move.index.x][move.index.y].setString("0");
}

/**
* gives a hint to the user by solving one cell at a time
*
* @param source The reference grid which we are going to solve a cell from
* @param reference Our solved reference
* @param index the index of the cell
*/
void hint(Text source[9][9], int reference[9][9], Vector2i index)
{
	if(source[index.x][index.y].getString() == "0")
	{
		source[index.x][index.y].setString(to_string(reference[index.x][index.y]));
	}
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
	textObj.setString(initialString + newData);
}

/**
* Pauses the sudoku game
*
* @param dueToInactivity Indicates whether the pause was due to inactivity or not
*/
void pause(bool dueToInactivity)
{
	timer.pause();
	paused = true;
	pausedDueToInactivity = dueToInactivity;
}

/**
* Resumes the sudoku game
*/
void resume()
{
	timer.resume();
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
	textureBackground.loadFromFile("Assets/Images/foo.png");
	Sprite spriteBackground = Sprite(textureBackground);
	
	Texture textureGrid;
	textureGrid.loadFromFile("Assets/Images/grid.png");
	Sprite spriteGrid(textureGrid);
	spriteGrid.setPosition({25, 120});
#pragma endregion,

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
	RectangleShape pen;
	RectangleShape shadowPen;

	//Styling
	/////////Main pen
	createPenInstance(pen, 0);
	
	/////////Shadow pen
	bool drawShadowPen = false;
	createPenInstance(shadowPen, 1);
#pragma endregion

#pragma region Pause Screen
	pauseScreen pauseScreen;
#pragma endregion


	
	/******************************************************************************************
	*******COPYING THE CONTENT OF THE INTEGER UNSOLVED ARRAY TO THE TEXT UNSOLVED ARRAY********
	******************************************************************************************/
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			int num = unsolvedTemplate[i][j];
			//setting string
			unsolvedTemplateText[i][j].setString(to_string(num));
			unsolvedTemplateText[i][j].setFont(googleBlack);

			//setting position
			positionSystem posNumber;
			posNumber.margin = posNumber.NUMBERS_MARGIN;
			posNumber.factor = {50, 50};
			posNumber.index = {j, i};
			
			Vector2f pos = posNumber.generatePosition();
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
			gameTime = timer.getElapsedTime();
			updateText(timeText, "TIME: ", gameTime);
#pragma endregion


		/************************************************
		 ****************KEYBOARD INPUT******************
		 ***********************************************/

		
		//Stop all kinds of input when the game is paused
		if (!paused)
		{
#pragma region KEYBOARD/MOUSE INPUT

			/********************************************************
			 *****HANDLING ARROWS INPUT IN ORDER TO MOVE THE PEN*****
			 *******************************************************/
#pragma region PEN MOVING INPUT
			////////////Keyboard
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
			}

			//Setting pen position
			positionSystem posPen;
			posPen.margin = posPen.PEN_MARGIN;
			posPen.factor = { 50,50 };
			posPen.index = { penTracer.x, penTracer.y };
			Vector2f pos = posPen.generatePosition();
			pen.setPosition(pos);

			////////////Mouse
			Vector2i mousePos = Mouse::getPosition(window);
			if (spriteGrid.getGlobalBounds().contains(mousePos.x, mousePos.y))
			{
				Vector2i index = { (mousePos.x - 25) / 50,(mousePos.y - 120) / 50 };

				//system("CLS");
				//cout << index.y << ',' << index.x << endl;

				positionSystem shadowPenPos;
				shadowPenPos.margin = shadowPenPos.PEN_MARGIN;
				shadowPenPos.factor = { 50,50 };
				shadowPenPos.index = index;
				
				shadowPen.setPosition(shadowPenPos.generatePosition());

				drawShadowPen = true;

				if (Mouse::isButtonPressed(Mouse::Left))
				{
					penTracer = index;
					pen.setPosition(shadowPenPos.generatePosition());
				}
			}
			else //Hide the shadow pen if the cursor is out of grid boundaries
			{
				drawShadowPen = false;
			}

			
			
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

			//The actual writing

			if (numPressed != 0) //A number between (1-9) entered
			{

				if (modifiableCell({penTracer.y, penTracer.x}, solvedTemplate, unsolvedTemplate))
				{
					unsolvedTemplateText[penTracer.y][penTracer.x].setString(to_string(numPressed));
					unsolvedTemplateText[penTracer.y][penTracer.x].setFont(googleRegular);

					//coloring the number to indicate either correct or incorrect move
					if (correctMove({penTracer.y, penTracer.x}, solvedTemplate, numPressed))
					{
						unsolvedTemplateText[penTracer.y][penTracer.x].setFillColor(correctMoveColor);
					}
					else
					{
						unsolvedTemplateText[penTracer.y][penTracer.x].setFillColor(falseMoveColor);
						falseMoves++;
					}

					//record the move in case the user wants to undo
					lastMove = gameMove{{penTracer.y, penTracer.x}, numPressed};
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
				}
			}
#pragma endregion
			
#pragma endregion


#pragma region Buttons handling
		if(clicked(buttons[1])) //Hint
		{
			hint(unsolvedTemplateText, solvedTemplate, {penTracer.y, penTracer.x});
			cout << completedCells << endl;
		}
		if(clicked(buttons[2])) //Pause
		{
			pause(false);
		}
		if(clicked(buttons[3])) //Undo
		{
			undo(unsolvedTemplateText, lastMove);
		}
#pragma endregion

#pragma region calculating completed cells
		completedCells = 0;
		for (int i  = 0; i < 9; i++)
		{
			for (int j  = 0; j < 9; j++)
			{
				string numStr = unsolvedTemplateText[i][j].getString();
				int num = stoi(numStr);
				
				if(num != 0)
				{
					if(correctMove({i,j}, solvedTemplate, num))
						completedCells++;
				}
			}	
		}
			cout << completedCells << endl;
#pragma endregion

		}


		/********************************************************
		 *******UPDATING AND SWITCHING BUFFERS*******************
		 *******************************************************/
#pragma region UPDATING AND SWITCHING BUFFERS
		 //clearing the window
		window.clear();

		//Background 
		window.draw(spriteBackground);		
		//9x9 grid
		window.draw(spriteGrid);

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

			buttons[i].create(posButtons.generatePosition(), btnIds[i]);
		}
		
		//Drawing Numbers
		for(int i = 0; i < 9; i++)
		{
			for(int j = 0; j < 9; j++)
			{
				if (unsolvedTemplateText[j][i].getString() == "0")
					continue;
				window.draw(unsolvedTemplateText[j][i]);
			}
		}

		//Primary Pen and shadow pen
		window.draw(pen);
		if(drawShadowPen)
			window.draw(shadowPen);

		//Highlighting
		highlightingSystem sys;
		sys.highlight({penTracer.x, penTracer.y});
		
		//Labels
		window.draw(timeText);
		window.draw(falseMovesText);
		window.draw(difficultyText);
		
		//Pause menu to indicate a paused game
		if (paused)
		{
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