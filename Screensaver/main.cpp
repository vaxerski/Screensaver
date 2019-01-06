#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <string>
#undef main
#include <tchar.h>
#include <chrono>
#include <ctime>
#include <ratio>
#include <random>
#include <sstream>
#include "INIReader.h"
#include <fstream>  
#include <winsock.h>
#include <math.h>


std::string sections(INIReader &reader)
{
	std::stringstream ss;
	std::set<std::string> sections = reader.Sections();
	for (std::set<std::string>::iterator it = sections.begin(); it != sections.end(); ++it)
		ss << *it << ",";
	return ss.str();
}

int main()
{

	//init values 

	int widX;
	int widY;
	bool toBounceX;
	bool toBounceY;
	float velX;
	float velY;
	int spriteR, spriteG, spriteB;
	bool isFullscreen = false;
	bool deGlitch = false;
	bool changeCol = true;
	int texX, texY;
	float scaleS;
	float rainbowH, rainbowS, rainbowV;
	bool rainbow;
	float reainbowSpeed;
	bool cornerCol;
	velX = velY = 0.07f;
	widX = 1920;
	widY = 1080;

	float startX, startY;
	float r, g, b;
	r = 255;
	g = 0;
	b = 0;

	//--------------RANDOM CAPTION------------------//

	int captionNum;
	std::string caption;
	std::string captions[12] = { "Screensaver: Will it hit the corner?", "Screensaver: Don't you have better things to do?", "Screensaver: I'm a gnome.", "Screensaver: Not rigged.", "Screensaver: Now in 16K!", "Screensaver: Who am I?", "Screensaver: To be or not to be?", "Screensaver: I am bored.", "Screensaver: Have you tried playing Terraria?", "Screensaver: Isn't Russia the biggest country?", "Screensaver: I was really bored so I coded this.", "Screensaver: If you see this, you're an awesome person!"};

	std::mt19937 gen;
	gen.seed(std::time(0));
	std::uniform_int_distribution<uint32_t> rande(1, 12);

	captionNum = rande(gen);
	caption = captions[captionNum];






	//-------------------HERE WE PARSE OUR CONFIG----------------------//

	INIReader reader("config.ini");
	if (reader.ParseError() < 0) {  //todo: auto generate default config
		MessageBox(nullptr, TEXT("Couldn't locate config.ini, autogenerating default."), TEXT("Fatal Error"), MB_OK);
		std::ofstream outfile("config.ini");
		outfile << "[user]\nwidth = 1920                 ; Width of the window\nheight = 1080                 ; Height of the window\nchangeColours = true                 ; Changes sprite colour every bounce.\ncornerColOnly = true      ; Changes sprite colour ONLY if it hits the corner.\nspeed = 0.1                 ; Speed of the icon, default = 0.1\ntextureX = 600                 ; Width of the sprite\ntextureY = 400                 ; Height of the sprite\nscale = 2                 ; scale of the sprite, bigger scale = smaller sprite. Default = 2\nrainbow = false                 ; rainbow mode.\nrainbowSpeed = 2                 ; rainbow mode speed." << std::endl;
		outfile.close();
		MessageBox(nullptr, TEXT("File created, please restart Screensaver."), TEXT("Success!"), MB_OK);
		return 0;
	}

	widX = reader.GetInteger("user", "width", 0);
	widY = reader.GetInteger("user", "height", 0);

	if (widX == 0 || widY == 0) {  //if width or height is invalid, we do not proceed.
		MessageBox(nullptr, TEXT("Couldn't create window: invalid width or height."), TEXT("Fatal Error"), MB_OK);
		Sleep(3000);
		return 0;
	}
	if (widX < 128 || widY < 128) {
		MessageBox(nullptr, TEXT("Very funny. Now change the resolution to something normal."), TEXT("Ha-ha"), MB_OK);
		Sleep(3000);
		return 0;
	}

	if (reader.Get("user", "changeColours", "UNDEFINED") == "true") {
		changeCol = true;
	}
	else if (reader.Get("user", "changeColours", "UNDEFINED") == "false") {
		changeCol = false;
	}
	else if (reader.Get("user", "changeColours", "UNDEFINED") == "UNDEFINED") {
		changeCol = false;
		MessageBox(nullptr, TEXT("config.ini: value changeColours is undefined."), TEXT("Warning"), MB_OK);
	}
	else {
		changeCol = false;
		MessageBox(nullptr, TEXT("config.ini: unknown error parsing value changeColours."), TEXT("Warning"), MB_OK);
	}

	if (reader.Get("user", "rainbow", "UNDEFINED") == "true") {
		rainbow = true;
	}
	else if (reader.Get("user", "rainbow", "UNDEFINED") == "false") {
		rainbow = false;
	}
	else if (reader.Get("user", "rainbow", "UNDEFINED") == "UNDEFINED") {
		rainbow = false;
		MessageBox(nullptr, TEXT("config.ini: value rainbow is undefined."), TEXT("Warning"), MB_OK);
	}
	else {
		rainbow = false;
		MessageBox(nullptr, TEXT("config.ini: unknown error parsing value rainbow."), TEXT("Warning"), MB_OK);
	}

	velX = velY = reader.GetReal("user", "speed", 0);

	if (velX == 0 && velY == 0) {
		MessageBox(nullptr, TEXT("config.ini: value speed is undefined or invalid."), TEXT("Warning"), MB_OK);
		velX = velY = 0.1;
	}

	scaleS = reader.GetReal("user", "scale", -1);
	if (scaleS == -1) {
		MessageBox(nullptr, TEXT("Could not initialize sprite: invalid scale"), TEXT("Fatal error"), MB_OK);
		Sleep(3000);
		return 0;
	}

	reainbowSpeed = reader.GetReal("user", "rainbowSpeed", -1);
	if (reainbowSpeed == -1) {
		MessageBox(nullptr, TEXT("Invalid rainbow speed."), TEXT("Fatal error"), MB_OK);
		Sleep(3000);
		return 0;
	}

	cornerCol = reader.GetBoolean("user", "cornerColOnly", false);


	std::uniform_int_distribution<uint32_t> rX(1, widX - widX/4);
	std::uniform_int_distribution<uint32_t> rY(1, widY - widY/4);

	startX = rX(gen);
	startY = rY(gen);


	sf::RenderWindow window(sf::VideoMode(widX, widY), caption, sf::Style::Fullscreen);   //render window

	sf::Texture tex;


	//-------------DETECT PNG SIZE-------------//

	std::ifstream in("sprite.png");
	unsigned int width, height;

	in.seekg(16);
	in.read((char *)&width, 4);
	in.read((char *)&height, 4);

	texX = ntohl(width);
	texY = ntohl(height);





	if (!tex.loadFromFile("sprite.png", sf::IntRect(0,0,texX,texY))) {
		MessageBox(nullptr, TEXT("Couldn't initiate texture."), TEXT("Fatal Error"), MB_OK);
		window.close();
	}

	sf::Sprite sprite;
	sprite.setTexture(tex);

	//----AUTOSCALE----//

	float temp;
	temp = texX / 600;
	temp *= scaleS;
	temp = 1 / temp;

//	sprite.setScale(temp, temp);

	float fixedX, fixedY;
	fixedX = temp * texX;
	fixedY = temp * texY;

	sprite.scale(sf::Vector2f(temp, temp));


	sf::Text text;
	sf::Font font;
	if (!font.loadFromFile("font.ttf"))
	{
		return 0;
	}
	text.setFont(font);

	text.setCharacterSize(24);




	using namespace std::chrono;
	window.setFramerateLimit(240);
	high_resolution_clock::time_point last = high_resolution_clock::now();
	
	sprite.setPosition(sf::Vector2f(startX, startY));

	while (window.isOpen())
	{
		
		if (!deGlitch) {
			window.close();
			window.create(sf::VideoMode(widX, widY), caption);
			isFullscreen = true;
			deGlitch = true;
			rainbowH = 360;
			rainbowS = 1;
			rainbowV = 1;
		}  //fix the time glitch, reload window once

		sf::Event event;
		high_resolution_clock::time_point now = high_resolution_clock::now();



		while (window.pollEvent(event))
		{


			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::F11)
				{
					if (isFullscreen) {
						window.close();
						window.create(sf::VideoMode(widX, widY), caption);
						isFullscreen = false;
					}
					else {
						window.close();
						window.create(sf::VideoMode(widX, widY), caption, sf::Style::Fullscreen);
						isFullscreen = true;
					}
				}

			}
		}

		spriteR = rand() % 255 + 1;
		spriteG = rand() % 255 + 1;
		spriteB = rand() % 255 + 1;

		if (spriteR < 100 && spriteG < 100 && spriteB < 100) {
			spriteR = 200;
		}

		

		
		if (last + std::chrono::steady_clock::duration(1) < now) {
			if (rainbow) {
				if (r <= 255 && b == 0 && r != 0) {
					r -= reainbowSpeed / 100;
					g += reainbowSpeed / 100;
				}
				else if (g <= 255 && r == 0 && g != 0) {
					g -= reainbowSpeed / 100;
					b += reainbowSpeed / 100;
				}
				else if (b <= 255 && g ==0 && b != 0) {
					b -= reainbowSpeed / 100;
					r += reainbowSpeed / 100;
				}

				if (r > 255) r = 255;
				if (g > 255) g = 255;
				if (b > 255) b = 255;
				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;


				sprite.setColor(sf::Color(r, g, b));
			}
			if (sprite.getPosition().x >(widX - fixedX)) toBounceX = true;
			if (sprite.getPosition().x < 0) toBounceX = true;
			if (sprite.getPosition().y >(widY - fixedY)) toBounceY = true;
			if (sprite.getPosition().y < 0) toBounceY = true;

			if (cornerCol) {
				if (sprite.getPosition().x < 10 && sprite.getPosition().y >(widY - fixedY - 10)) {
					//RUpCorner hit
					sprite.setColor(sf::Color(spriteR, spriteG, spriteB));
				}
				if (sprite.getPosition().x < 10 && sprite.getPosition().y < 10) {
					//RDownCorner hit
					sprite.setColor(sf::Color(spriteR, spriteG, spriteB));
				}
				if (sprite.getPosition().x >(widX - fixedX - 10) && sprite.getPosition().y >(widX - fixedX - 10)) {
					//LUpCorner hit
					sprite.setColor(sf::Color(spriteR, spriteG, spriteB));
				}
				if (sprite.getPosition().x > (widX - fixedX - 10) && sprite.getPosition().y < 10) {
					//LDownCorner hit
					sprite.setColor(sf::Color(spriteR, spriteG, spriteB));
				}
			}


			if (toBounceX) {
				velX = -velX;
				if(changeCol && !rainbow && !cornerCol) sprite.setColor(sf::Color(spriteR, spriteG, spriteB));
				toBounceX = false;
			}
			if (toBounceY) {
				velY = -velY;
				if (changeCol && !rainbow && !cornerCol) sprite.setColor(sf::Color(spriteR, spriteG, spriteB));
				toBounceY = false;
			}

			std::string maximus = "\nR:" + std::to_string(r) + "\nG:" + std::to_string(g) + "\nB:" + std::to_string(b);
			text.setString(maximus);
			window.clear();
			sprite.move(velX, velY);
			window.draw(sprite);
		//	window.draw(text);
			last = high_resolution_clock::now();
			
		}
		window.display();
		
	}

	return 0;
}