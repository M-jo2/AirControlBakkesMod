#pragma once
#pragma comment(lib,"bakkesmod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/wrappers/linmath.h"
#include <ctime>

class AirControlTraining : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	int timeBegin;
	int timeLastTouch;
	int MaxDistanceBall;
	int MinDistanceBall;
	int mode;
	double angleCarBallByPlayer;
	int nbBallTouched = 0;
	int nbBallMissed = 0;
	
	bool mustToBoost = true;
	float angularSpeed = 0;
	float maxAngularSpeed = 1.0f;
public:

	AirControlTraining();
	virtual void onLoad();
	virtual void onUnload();
	void timeout(GameWrapper* eventName);
	void pinCarOnTheAir(GameWrapper* eventName);
	void shoot();
	void RandomBallPosition(float distMax,float distMin);
	void Render(CanvasWrapper canvas);

	float angle(Vector vec1,Vector vec2);
};