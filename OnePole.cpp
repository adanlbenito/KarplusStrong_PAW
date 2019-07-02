#include "OnePole.h"
#include <math.h>
#include <stdio.h>

OnePole::OnePole() {}

OnePole::OnePole(float fc, int type)
{
	setup(fc, type);
}

int OnePole::setup(float fc, int type)
{
	ym1 = 0.0; // Reset filter state
	setFilter(fc, type);
	return 0;
}

void OnePole::setFilter(float fc, int type)
{
	setType(type);
	setFc(fc);
}

void OnePole::setFc(float fc)
{
	if(type_ == LP)
	{
		b1 = expf(-2.0f * (float)M_PI * fc);
		a0 = 1.0f - b1;
	}
	else if(type_ == HP)
	{
		b1 = -expf(-2.0f * (float)M_PI * (0.5f - fc));
		a0 = 1.0f + b1;
	}
	fc_ = fc;
}

void OnePole::setType(int type)
{
	if(type == LP || type == HP)
	{
		type_ = type;
	}	
	else
	{
		fprintf(stderr, "Invalid type\n");
	}
}	

float OnePole::process(float input)
{
	return ym1 = input * a0 + ym1 * b1;
}

OnePole::~OnePole()
{
	cleanup();
}

void OnePole::cleanup() { }
