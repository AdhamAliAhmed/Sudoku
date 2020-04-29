#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include<string>
#include <cstdlib>
#include <windows.h>

struct glyphButton;
struct button;
struct gif;

using namespace sf;
using namespace std;


const int WIDTH = 500, HEIGHT = 650;

VideoMode videoMode(WIDTH, HEIGHT);
RenderWindow window(videoMode, "Astro Sudoku (beta)", Style::Close);

#pragma region GLOBAL VARIABLES
#pragma region Game data
int completedCells = 0;
int falseMoves = 0;
int hints = 0;
string gameDifficulty;
string gameTime;
#pragma endregion

#pragma region constants
const int TEMPLATES_COUNT = 50;

const int MOVE_SLEEP_DURATION = 150;

#pragma region scenes ids
const string MAIN_MENU_SCENE_ID = "main_menu";
const string NEW_GAME_SCENE_ID = "new_game";
const string MAIN_GAME_SCENE_ID = "main_game";
const string CREDITS_SCENE_ID = "credits";
#pragma endregion
#pragma endregion

string renderedSceneId = MAIN_MENU_SCENE_ID;
bool gameInProgress = true;

Color correctMoveColor = Color(0,152,250), falseMoveColor = Color(194,24,8);

//Used to trace the pen in order to move it using the keyboard arrows and knowing the current position 
Vector2i penTracer = { 0,0 };

bool paused = false; bool dueToFocusLoss = false;
bool pauseMenuShown = false, pauseIndicationShown = false;

#pragma endregion

#pragma region structs

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

struct pauseIndicationScene;

/**
* pauses the sudoku game
* @param the cause for the game to be paused
*/
void pause(string cause)
{
	paused = true;
	timer.pause();

	if(cause == "on_purpose")
		pauseIndicationShown = true;
	else if(cause == "pause_menu")
		pauseMenuShown = true;
	else if(cause == "focus_loss")
		dueToFocusLoss = true;
	else
		return;
}

/**
* resumes the sudoku game
*/
void resume()
{
	paused = false;
	pauseMenuShown = false;
	pauseIndicationShown = false;
	dueToFocusLoss = false;
	
	timer.resume();
}

/**
* Stores the data of a move during the game for later use
*
*/
struct gameMove
{
	Vector2i index = {-1, -1};
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


	/**
	* Generates a position on the screen using three factors: margin, factor and index. Made specifically for pens
	* @param index index of the pen
	*/
	Vector2f generatePenPosition(Vector2i index)
	{
		margin = PEN_MARGIN;
		factor = {50,50};
		
		float x = margin.x + (index.x * factor.x),
			y = margin.y + (index.y * factor.y);
		
		position = {x, y};
		return position;
	}
};

bool correctMove(Vector2i index, int source[9][9], int num);
struct scanningSystem
{
	/**
	* updates the values of correct, false moves each frame. Parameters are self explaining
	*/	
	void scan(Text unsolvedTemplateText[9][9], int solvedTemplate[9][9], int &completedCellsHolder, int &falseMovesHolder)
	{
		completedCellsHolder = 0;
		int currentFalseMoves = 0;

		for (int i  = 0; i < 9; i++)
		{
			for (int j  = 0; j < 9; j++)
			{
				string numStr = unsolvedTemplateText[i][j].getString();
				int num = stoi(numStr);
				
				if(num != 0)
				{
					if(correctMove({i,j}, solvedTemplate, num))
						completedCellsHolder++;
					else
						currentFalseMoves++;
				}
			}	
		}

		if(currentFalseMoves > falseMovesHolder)
			falseMovesHolder = currentFalseMoves;
	}	
}scanningSys;

/**
* produces a sudoku pen with some special characteristics
*
* @param pen The sudoku pen.
* @param type The type of the pen.
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
* Returns the center of a  object
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
	Event event;
	
	Vector2f buttonSize = { 0, 0 },
	buttonPosition = { 0,0 };
	
	Texture textureSheet;
	
	IntRect tracer;
	Sprite spriteState;

	bool downOnce = false; bool clicked = false;
	short state = 0;

	//functions
	
	/**
	* Returns true if the mouse cursor hovers over the button
	* @param sprite The sprite that the cursor hovers over.
	*/
	bool mouseHover(Sprite sprite)
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window); //mouse position according to the window

		bool inBounds = sprite.getGlobalBounds().contains({static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)});

		return inBounds;
	}
	/**
	* Returns true if the left mouse button is pressed down while the cursor is in hover state
	* @param sprite The sprite that the cursor hovers over.
	*/
	bool mouseDown(Sprite sprite){
		return  mouseHover(sprite) && Mouse::isButtonPressed(Mouse::Left);
	}

	/**
	* Returns true if the left mouse button is released while the cursor is in hover state
	* @param sprite The sprite that the cursor hovers over.
	*/
	bool mouseUp(Sprite sprite){
		return mouseHover(sprite) && (!mouseDown(sprite)) && downOnce;
	}

	/**
	* Renders a glyph button on the screen
	* @param pos The pos for the button to be at.
	* @param id The id of the button. Helps with loading the glyphs
	*/
	void create(sf::Vector2f pos, Vector2f size, string directory)
	{
		clicked = false;
		
		buttonPosition = pos;
		spriteState.setPosition(buttonPosition);

		buttonSize = size;
		tracer = {0,0, static_cast<int>(buttonSize.x), static_cast<int>(buttonSize.y)};
		
		textureSheet.setSmooth(true);
		textureSheet.loadFromFile(directory);

		spriteState.setTexture(textureSheet);

		//appearance
		if(mouseHover(spriteState))
		{
			tracer.left = buttonSize.x;

			state = 1;
			
			if(mouseDown(spriteState))
			{
				downOnce = true;

				tracer.left = buttonSize.x * 2;

				state = 2;
			}
		}
		else
		{
			tracer.left = 0;

			state = 0;
		}
		spriteState.setTextureRect(tracer);
		
		//functionality
		if(mouseUp(spriteState))
		{
			clicked = true;

			downOnce = false;
		}

		//cancels all operations if the mouse cursor exits the bounds of the button
		if(!mouseHover(spriteState))
			downOnce = false;
		
		window.draw(spriteState);
	}
};
/**
* Returns true if a glyph button is clicked
* @param btn The button to be clicked
*/
bool clicked(glyphButton btn)
{
	if(btn.clicked)
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

/**
* Opens a url in the default browser
* @url the url to be opened
*/
void navigateToUrl(string url)
{
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

struct pauseIndicationScene
{	
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnResume;

	/**
	* renders a pause indication scene
	* @param dueToFocusLoss determines whether the pause was due to inactivity or not
	*/
	void create(bool dueToFocusLoss)
	{
		float margin = 20.0f;

		//Content
		textureContent.loadFromFile("Assets/scenes-content/pause_indication_content.png");
		spriteContent.setTexture(textureContent);

		window.draw(spriteContent);

		btnResume.spriteState.setScale(0.8,0.8);

		if(clicked(btnResume))
			resume();

		if(!dueToFocusLoss)
			btnResume.create({225,405}, {50,50} ,"Assets/buttons/pause_indication/resume.png");
	}
} pauseIndicationScene;

struct MainMenuScene
{
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnNewGame, btnHowToPlay, btnStatistics, btnSettings, btnCredits;

	void loadResources()
	{
		textureContent.loadFromFile("Assets/scenes-content/main_menu_content.png");
	}

	void initObjects()
	{
		spriteContent = Sprite(textureContent);
	}

	void prepareForRender()
	{
		loadResources();
		initObjects();
	}

	void updateObjects() { }

	void render()
	{
		window.draw(spriteContent);

		string directory = "Assets/buttons/main_menu/";
		Vector2f buttonSize = {300,35};
		float posX = 26.0f;
		
		btnNewGame.create({posX, 240}, buttonSize, directory+"new_game.png");
		btnHowToPlay.create({posX, 275}, buttonSize,  directory+"how_to_play.png");
		btnStatistics.create({posX, 310}, buttonSize, directory+"statistics.png");
		btnSettings.create({posX, 345}, buttonSize, directory + "settings.png");
		btnCredits.create({posX, 380}, buttonSize, directory + "credits.png");
		
		if(clicked(btnNewGame))
			renderedSceneId = "new_game";
		if(clicked(btnHowToPlay))
			renderedSceneId = "not_yet_implemented";
		if(clicked(btnStatistics))
			renderedSceneId = "not_yet_implemented";
		if(clicked(btnSettings))
			renderedSceneId = "not_yet_implemented";
		if(clicked(btnCredits))
			renderedSceneId = "credits";
	}
	
}mainMenuScene;

struct newGameScene
{
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnEasy, btnMedium, btnHard, btnExpert;
	Vector2f buttonSize = {265,35};
	float posX = 26.0f;
	
	void loadResources()
	{
		textureContent.loadFromFile("Assets/scenes-content/new_game_content.png");
		
	}

	void initObjects()
	{
		spriteContent = Sprite(textureContent);
	}

	void updateObjects()
	{
		
	}

	void prepareForRender()
	{
		loadResources();
		initObjects();
	}
	
	void render()
	{
		window.draw(spriteContent);
		
		string directory = "Assets/buttons/new_game/";
		
		btnEasy.create({posX, 240}, buttonSize, directory+"easy.png");
		btnMedium.create({posX, 275}, buttonSize,  directory+"medium.png");
		btnHard.create({posX, 310}, buttonSize, directory+"hard.png");
		btnExpert.create({posX, 345}, buttonSize, directory + "expert.png");

		if(clicked(btnEasy))
			gameDifficulty = "easy";
		if(clicked(btnMedium))
			gameDifficulty = "medium";
		if(clicked(btnHard))
			gameDifficulty = "hard";
		if(clicked(btnExpert))
			gameDifficulty = "expert";
	}
}newGameScene;

struct creditsScene
{
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnGithub, btnWebsite;

	void loadResources()
	{
		textureContent.loadFromFile("Assets/scenes-content/credits_content.png");
	}

	void initObjects()
	{
		spriteContent = Sprite(textureContent);
	}

	void perpareForRender()
	{
		loadResources();
		initObjects();
	}

	void updateObjects(){ }

	void render()
	{
		window.draw(spriteContent);
		
		//buttons
		Vector2f buttonSize = {64,64};
		float posY = 410.0f;
		
		string directory = "Assets/buttons/credits/";
		
		btnWebsite.create({175, posY}, buttonSize, directory+"website.png");
		btnGithub.create({261, posY}, buttonSize,  directory+"github.png");

		if(clicked(btnWebsite))
			navigateToUrl("https://google.com");
		if(clicked(btnGithub))
			navigateToUrl("https://github.com/adhamali450/Sudoku");
	}
} creditsScene;

struct notYetImplementedScene
{
	Texture textureContent;
	Sprite spriteContent;

	void loadResources()
	{
		textureContent.loadFromFile("Assets/scenes-content/not_yet_implemented_content.png");
	}

	void initObjects()
	{
		spriteContent = Sprite(textureContent);
	}

	void prepareForRender()
	{
		loadResources();
		initObjects();
	}

	void updateObjects() { }

	void render()
	{
		window.draw(spriteContent);
	}
} notYetImplementedScene;
#pragma endregion 

string toUpper(string str)
{
	string strUpper = "";
	for(char c : str)
			strUpper += std::toupper(c);
	return strUpper;
}

/**
* produces a sudoku pen with some special characteristics
*
* @param pen The sudoku pen.
* @param type The type of the pen.
*/
void createPenInstance(RectangleShape &pen, short type)
{
	RectangleShape penObj({34,35});

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
	fillColor = Color(170, 183, 184, 100);
	outlineColor = Color(33, 47, 60);
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
	bool moveInBounds = (move.index.x >= 0 && move.index.x <= 8) & (move.index.y >= 0 && move.index.y <= 8); 

	if(moveInBounds)
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
		hints++;
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
* Returns the number pressed on the keyboard between 1,9. If none is pressed 0 is returned
*/
int fetchPressedNumber()
{
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

	return numPressed;
}

/**
* Returns a random number between 2 bounds
* 
* @param begin the left bound
* @param end the right bound
*/
int getRandom(int begin, int end)
{
	srand(time(NULL));
	return (rand() % end) + begin;
}

/**
* Loads a sudoku template from an external file
* 
* @param host the integer array to host the template
* @param directory the directory of the file
*/
void loadTemplate(int host[9][9], string directory)
{
	string templateStr;
	
	ifstream stream(directory);
	if(stream.is_open())
	{
		int templateIndex = getRandom(0,49);

		int pointerPos = (83* templateIndex);
		stream.seekg(pointerPos);
		
		stream >> templateStr;	
	}
	else
	{
		cout << "A crucial file/s containing data cannot be located" << endl;
		return;
	}

	for (int i = 0; i < 9; i++)
	{
		string row = templateStr.substr(9 * i, 9);
		for (int j = 0; j < 9; j++)
		{
			int number = stoi(row.substr(j,1));
			host[i][j] = number;
		}
	}
}

bool gameRestarted = false;

void quit(string sceneID)
{
	resume();
	gameInProgress = false;
	gameDifficulty = "";
	renderedSceneId = sceneID;
}

struct winingMenu
{
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnContinue;

	void create(Font font)
	{
		textureContent.loadFromFile("Assets/scenes-content/win_content.png");
		spriteContent = Sprite(textureContent);

		window.draw(spriteContent);		

		Color textColor = Color(238,134,34);
		float posX = 270.0f;
		
		Text difficultyText = Text(toUpper(gameDifficulty), font, 17);
		difficultyText.setFillColor(textColor);
		difficultyText.setPosition({posX, 380.5});
		
		Text timeText = Text(timer.getElapsedTime(), font, 15);
		timeText.setFillColor(textColor);
		timeText.setPosition({posX, 411});

		Text falseMovesText = Text(to_string(falseMoves), font, 15);
		falseMovesText.setFillColor(textColor);
		falseMovesText.setPosition({posX, 441.5});

		Text hintsText = Text(to_string(hints), font, 15);
		hintsText.setFillColor(textColor);
		hintsText.setPosition({posX, 472});

		window.draw(difficultyText);
		window.draw(timeText);
		window.draw(falseMovesText);
		window.draw(hintsText);
		
		string directory = "Assets/buttons/win_lose/";
		Vector2f buttonSize = {150,29.14};

		btnContinue.create({187, 520}, buttonSize, directory+"continue.png");
	}
} winingMenu;

struct loseMenu
{
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnContinue, btnRestart;

	void create()
	{
		textureContent.loadFromFile("Assets/scenes-content/lose_content.png");
		spriteContent = Sprite(textureContent);

		window.draw(spriteContent);		

		string directory = "Assets/buttons/win_lose/";
		Vector2f buttonSize = {150,29.14};

		btnContinue.create({40, 466.5}, buttonSize, directory+"continue.png");
		btnRestart.create({301, 466.5}, buttonSize,  directory+"restart.png");
	}
} loseMenu;

struct pauseMenu
{	
	Texture textureContent;
	Sprite spriteContent;

	glyphButton btnResume, btnRestart, btnNewGame, btnQuit, btnExit;

	void create()
	{
		textureContent.loadFromFile("Assets/scenes-content/pause_menu_content.png");
		spriteContent = Sprite(textureContent);

		window.draw(spriteContent);		

		string directory = "Assets/buttons/pause_menu/";
		Vector2f buttonSize = {265,35};
		float posX = 26.0f;

		btnResume.create({posX, 220}, buttonSize, directory+"resume.png");
		btnRestart.create({posX, 275}, buttonSize,  directory+"restart.png");
		btnNewGame.create({posX, 310}, buttonSize, directory+"new_game.png");
		btnQuit.create({posX, 345}, buttonSize, directory + "quit.png");
		btnExit.create({posX, 380}, buttonSize, directory + "exit.png");

		if(clicked(btnResume))
			resume();
		if(clicked(btnRestart))
		{
			gameRestarted = true;
		}
		if(clicked(btnNewGame))
			quit("new_game");
		if(clicked(btnQuit))
			quit("main_menu");
		if(clicked(btnExit))
			exit(0);
	}
}pauseMenu;

struct mainGameScene
{
	///templates
	int unsolvedTemplate[9][9];
	Text unsolvedTemplateText[9][9];
	int solvedTemplate[9][9];
	///images
	Texture textureBackground; Sprite spriteBackground;
	Texture textureGrid; Sprite spriteGrid;
	///buttons
	string btnIds[5] = {"menu", "hint", "pause", "undo", "settings"};
	glyphButton buttons[5];
	///labels
	////fonts
	Font googleRegular, googleBlack, googleBold;
	Text difficultyText; //Game difficulty
	Text falseMovesText; //Number of false moves
	Text timeText; //Game time
	///pens
	RectangleShape pen, shadowPen;
	bool drawShadowPen = false;

	void copyTemplates()
	{
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				int num = unsolvedTemplate[i][j];
				//setting string
				unsolvedTemplateText[i][j] = Text(to_string(num), googleBlack);

				//setting position
				positionSystem posNumber;
				posNumber.margin = posNumber.NUMBERS_MARGIN;
				posNumber.factor = {50, 50};
				posNumber.index = {j, i};
				
				Vector2f pos = posNumber.generatePosition();
				unsolvedTemplateText[i][j].setPosition(pos);
			}
		}
	}

	void restart()
	{
		copyTemplates();
		falseMoves = 0;
		gameRestarted = false;
		resume();
		timer.reset();
	}
	
	void loadResources(string difficulty)
	{
		//templates
		loadTemplate(unsolvedTemplate, "Assets/templates/unsolved/" + difficulty + ".txt");		
		loadTemplate(solvedTemplate, "Assets/templates/solved/" + difficulty + ".txt");

		//images
		textureBackground.loadFromFile("Assets/Images/background7.png");
		textureGrid.loadFromFile("Assets/Images/grid.png");
		//fonts
		googleRegular.loadFromFile("Assets/fonts/ProductSans-Regular.ttf");
		googleBold.loadFromFile("Assets/fonts/ProductSans-Bold.ttf");
		googleBlack.loadFromFile("Assets/fonts/ProductSans-Black.ttf");
	}

	void initObjects()
	{
		//images
		spriteBackground = Sprite(textureBackground);
		spriteGrid = Sprite(textureGrid);
		spriteGrid.setPosition({25, 120});
		//labels
		float textScale = 0.7f,
		positionY = 580.0f;

		difficultyText.setString(toUpper(gameDifficulty));
		difficultyText.setFont(googleBold);
		difficultyText.setScale({ textScale,textScale});
		difficultyText.setPosition(25, positionY);

		falseMovesText.setFont(googleBold);
		falseMovesText.setFillColor(Color(255, 128, 128));
		falseMovesText.setScale({ textScale,textScale });
		falseMovesText.setPosition(160, positionY);

		timeText.setFont(googleBold);
		timeText.setScale({ textScale,textScale });
		timeText.setPosition(380, positionY);
		//pens
		createPenInstance(pen, 0);
		penTracer = {0,0};
		
		createPenInstance(shadowPen, 1);

		//copying templates
		copyTemplates();
	}

	void prepareForRender(string gameMode)
	{
		loadResources(gameMode);
		initObjects();

		timer.reset();
	}
	
	void updateObjects(Event event)
	{
		if(gameRestarted)
			restart();

		#pragma region Updating Labels
		updateText(falseMovesText, "FALSE MOVES: ", to_string(falseMoves));

		gameTime = timer.getElapsedTime();
		updateText(timeText, "TIME: ", gameTime);

		#pragma endregion
		/******************************************************
		 ****************KEYBOARD/MOUSE INPUT******************
		 ******************************************************/
		
		//Stop all kinds of input and updating when the game is paused
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

				sleep(milliseconds(MOVE_SLEEP_DURATION));
			}

			////////////Mouse
			Vector2i mousePos = Mouse::getPosition(window);
			if (spriteGrid.getGlobalBounds().contains(mousePos.x, mousePos.y))
			{
				Vector2i index = { (mousePos.x - 25) / 50,(mousePos.y - 120) / 50 };

				positionSystem shadowPenPos;
				shadowPen.setPosition(shadowPenPos.generatePenPosition(index));

				drawShadowPen = true;

				if (Mouse::isButtonPressed(Mouse::Left))
					penTracer = index;
			}
			else //Hide the shadow pen if the cursor is out of grid boundaries
			{
				drawShadowPen = false;
			}

			//Setting pen position
			positionSystem posPen;
			pen.setPosition(posPen.generatePenPosition(penTracer));
#pragma endregion

			/********************************************************
			 ********HANDLING NUMBERS INPUT IN ORDER TO WRITE********
			 *******************************************************/
#pragma region NUMBERS INPUT
			int numPressed = fetchPressedNumber();

			//The actual writing
			if (numPressed != 0) //A number between (1-9) is pressed
			{

				if (modifiableCell({penTracer.y, penTracer.x}, solvedTemplate, unsolvedTemplate))
				{
					unsolvedTemplateText[penTracer.y][penTracer.x].setString(to_string(numPressed));
					unsolvedTemplateText[penTracer.y][penTracer.x].setFont(googleBold);

					//coloring the number to indicate either correct or incorrect move
					if (correctMove({penTracer.y, penTracer.x}, solvedTemplate, numPressed))
						unsolvedTemplateText[penTracer.y][penTracer.x].setFillColor(correctMoveColor);
					else
						unsolvedTemplateText[penTracer.y][penTracer.x].setFillColor(falseMoveColor);

					//recording the move in case the user wants to undo
					lastMove = gameMove{{penTracer.y, penTracer.x}, numPressed};
				}
			}
#pragma endregion


			/********************************************************
			 ********************UN-DOING A MOVE*********************
			 *******************************************************/
#pragma region keyboard shortcuts
			//undo ctrl+Z
			if (Keyboard::isKeyPressed(Keyboard::Z) && event.key.control) //user pressed ctrl+z in order to undo the last move
				undo(unsolvedTemplateText, lastMove);
			//hint ctrl+h
			if (Keyboard::isKeyPressed(Keyboard::H) && event.key.control) //user pressed ctrl+h in order to get a hint
				hint(unsolvedTemplateText, solvedTemplate, {penTracer.y, penTracer.x});
#pragma endregion

#pragma endregion


#pragma region Buttons handling
		if(clicked(buttons[0])) //Pause menu
		{
			pause("pause_menu");
		}
		if(clicked(buttons[1])) //Hint
		{
			hint(unsolvedTemplateText, solvedTemplate, {penTracer.y, penTracer.x});
		}
		if(clicked(buttons[2])) //Pause
		{
			pause("on_purpose");
		}
		if(clicked(buttons[3])) //Undo
		{
			undo(unsolvedTemplateText, lastMove);
		}
#pragma endregion

			//calculating completed cells and falseMoves each frame
			scanningSys.scan(unsolvedTemplateText, solvedTemplate, completedCells, falseMoves);
		}
	}

	void render()
	{	
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

			string directory = "Assets/buttons/main_game/" + btnIds[i] + ".png";
			buttons[i].create(posButtons.generatePosition(), {50,50}, directory);
		}

		//Primary Pen and shadow pen
		window.draw(pen);
		if(drawShadowPen)
			window.draw(shadowPen);

		//Highlighting
		highlightingSystem sys;
		sys.highlight({penTracer.x, penTracer.y});
		
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
		
		//Labels
		window.draw(timeText);
		window.draw(falseMovesText);
		window.draw(difficultyText);

		if(paused)
		{
			if(pauseMenuShown)
				pauseMenu.create();
		
			if (pauseIndicationShown)
				pauseIndicationScene.create(false);
		}

		if(completedCells == 81)
		{
			pause("");
			winingMenu.create(googleBlack);
			if(clicked(winingMenu.btnContinue))
				quit("main_menu");
		}
		if(falseMoves > 3)
		{
			pause("");
			loseMenu.create();
			if(clicked(loseMenu.btnContinue))
				quit("main_menu");
			if(clicked(loseMenu.btnRestart))
				gameRestarted = true;
		}
	}
} mainGameScene;

bool difficultyChosen()
{
	return gameDifficulty == "easy" |  gameDifficulty == "medium"  |  gameDifficulty == "hard" |  gameDifficulty == "expert";
}

int main()
{
	//scenes preparation
	mainMenuScene.prepareForRender(); 
	newGameScene.prepareForRender();
	creditsScene.perpareForRender();
	notYetImplementedScene.prepareForRender();

	//back button
	glyphButton btnBack;
	
	while (window.isOpen())
	{
		while(true)
		{
			if(difficultyChosen()) break;
			
			Event event;
			while (window.pollEvent(event))
			{
				switch (event.type)
				{
				case Event::Closed:
					exit(0);
					break;
				default: break;
				}
			}

			window.clear();

#pragma region choosing which scene to render
			if(renderedSceneId == "main_menu")
				mainMenuScene.render();
			else if(renderedSceneId == "new_game")
				newGameScene.render();
			else if(renderedSceneId == "credits")
				creditsScene.render();
			else if(renderedSceneId == "not_yet_implemented")
				notYetImplementedScene.render();
#pragma endregion

			if(renderedSceneId != "main_menu")
				btnBack.create({395, 606}, {75,23}, "Assets/buttons/back.png");

			window.display();
			
			if(clicked(btnBack) | Keyboard::isKeyPressed(Keyboard::Escape))
			{
				if(renderedSceneId != "main_menu")
					renderedSceneId = "main_menu";
			}
		}

		mainGameScene.prepareForRender(gameDifficulty);

		gameInProgress = true;
		falseMoves = 0;
		
		while (gameInProgress)
		{
			Event event;
			while (window.pollEvent(event))
			{
				switch (event.type)
				{
				case Event::Closed:
					exit(0);
					break;
				case Event::LostFocus:
					if(!paused)
						pause("focus_loss");
					break;
				case Event::GainedFocus:
					if(paused & dueToFocusLoss)
						resume();
					break;
				default: break;
				}
			}

			mainGameScene.updateObjects(event);
			
			window.clear();

			mainGameScene.render();
						
			window.display();
		}	
	}
	
}