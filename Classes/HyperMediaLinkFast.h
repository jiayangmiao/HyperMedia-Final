#pragma once
#include "HyperMediaLink.h"

enum ChangeType { Add, Remove };

class HyperMediaLinkFast
{

public:
	HyperMediaLinkFast(HyperMediaLink *link, ChangeType type);

	std::string linkName;
	int frame;
	ChangeType type;
	std::string targetFilename;
	int targetFrame;
	int X;
	int Y;
	int height;
	int width;
};