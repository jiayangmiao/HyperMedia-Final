#include "HyperMediaLink.h"
#include "../Library/rapidxml/rapidxml.hpp"

HyperMediaLink::HyperMediaLink(rapidxml::xml_node<> *node)
{
	this->linkName = node->first_node("name")->value();
	this->startFrame = atoi(node->first_node("startFrame")->value());
	this->endFrame = atoi(node->first_node("endFrame")->value());
	this->targetFilename = node->first_node("targetFilename")->value();
	this->targetFrame = atoi(node->first_node("targetFrame")->value());
	this->X = atoi(node->first_node("originX")->value());
	this->Y = atoi(node->first_node("originY")->value());
	this->height = atoi(node->first_node("height")->value());
	this->width = atoi(node->first_node("width")->value());
}

void HyperMediaLink::beautifullyPrint()
{
	std::cout << "_________________________" << "\n";
	std::cout << "Name           : " << this->linkName << "\n";
	std::cout << "Start Frame    : " << this->startFrame << "\n";
	std::cout << "End Frame      : " << this->endFrame << "\n";
	std::cout << "Target Filename: " << this->targetFilename << "\n";
	std::cout << "Target Frame   : " << this->targetFrame << "\n";
	std::cout << "X              : " << this->X << "\n";
	std::cout << "Y              : " << this->Y << "\n";
	std::cout << "Height         : " << this->height << "\n";
	std::cout << "Width          : " << this->width << "\n";
	std::cout << "_________________________" << "\n";
}

