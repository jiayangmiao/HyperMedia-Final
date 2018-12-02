#pragma once
#include <iostream>
#include "HyperMediaLink.h"
#include "HyperMediaLinkFast.h"
class HyperLinkForFrame {
public:
	HyperLinkForFrame(HyperMediaLink * normalLink);
	HyperLinkForFrame(HyperMediaLinkFast * fastLink);

	std::string targetFilename;
	int targetFrame;
	int X;
	int Y;
	int height;
	int width;	
};


