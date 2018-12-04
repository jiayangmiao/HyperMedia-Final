#pragma once
#include "../Library/rapidxml/rapidxml.hpp"
#include <string>
#include <iostream>

class HyperMediaLink
{
public:
	HyperMediaLink(rapidxml::xml_node<> *node);
	//HyperMediaLink(const HyperMediaLink &link);
	HyperMediaLink(std::string, int, int, std::string, int, int, int, int, int);

	std::string linkName;
	int startFrame;
	int endFrame;
	std::string targetFilename;
	int targetFrame;
	int X;
	int Y;
	int height;
	int width;

	void beautifullyPrint();
};