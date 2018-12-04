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

/*
HyperMediaLink::HyperMediaLink(const HyperMediaLink &link)
{
	this->linkName = link.linkName;
	this->startFrame = link.startFrame;
	this->endFrame = link.endFrame;
	this->targetFilename = link.targetFilename;
	this->targetFrame = link.targetFrame;
	this->X = link.X;
	this->Y = link.Y;
	this->height = link.height;
	this->width = link.width;
}
*/

HyperMediaLink::HyperMediaLink(std::string linkName, int startFrame, int endFrame, std::string targetFilename, int targetFrame, int X, int Y, int height, int width)
{
	this->linkName = linkName;
	this->startFrame = startFrame;
	this->endFrame = endFrame;
	this->targetFilename = targetFilename;
	this->targetFrame = targetFrame;
	this->X = X;
	this->Y = Y;
	this->height = height;
	this->width = width;
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

