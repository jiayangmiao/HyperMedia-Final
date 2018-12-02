#pragma once
#include "../Library/rapidxml/rapidxml.hpp"
#include <string>
#include <iostream>

class HyperMediaLink
{
public:
	HyperMediaLink(rapidxml::xml_node<> *node);

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