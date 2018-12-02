#include "HyperMediaLinkFast.h"

HyperMediaLinkFast::HyperMediaLinkFast(HyperMediaLink *link, ChangeType type)
{
	this->linkName = link->linkName;
	this->type = type;
	if (type == Add) {
		this->frame = link->startFrame;
	}
	else {
		this->frame = link->endFrame;
	}
	this->targetFilename = link->targetFilename;
	this->targetFrame = link->targetFrame;
	this->X = link->X;
	this->Y = link->Y;
	this->height = link->height;
	this->width = link->width;
}
