#include "HyperLinkForFrame.h"

HyperLinkForFrame::HyperLinkForFrame(HyperMediaLink * normalLink)
{
	targetFilename = normalLink->targetFilename;
	targetFrame = normalLink->targetFrame;
	X = normalLink->X;
	Y = normalLink->Y;
	height = normalLink->height;
	width = normalLink->width;
}

HyperLinkForFrame::HyperLinkForFrame(HyperMediaLinkFast * fastLink)
{
	targetFilename = fastLink->targetFilename;
	targetFrame = fastLink->targetFrame;
	X = fastLink->X;
	Y = fastLink->Y;
	height = fastLink->height;
	width = fastLink->width;
}