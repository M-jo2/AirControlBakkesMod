#include "AirControlTraining.h"

BAKKESMOD_PLUGIN(AirControlTraining, "Air_Control_Training", "0.1", 0)

using namespace std;

AirControlTraining::AirControlTraining() {
	timeBegin = 0;
	MaxDistanceBall = 500;
	MinDistanceBall = 490;
	mode = 0;
}

void AirControlTraining::onLoad() {
	std::shared_ptr<CVarManagerWrapper> cv = this->cvarManager;
	std::shared_ptr<GameWrapper> gw = this->gameWrapper;
	
	cv->registerCvar("stat", "0", "0=automatic refresh 1= refresh when touch the ball", true, true, 0, true, 1, true);
	cv->registerCvar("timing", "-2.5", " dd ", true, true, 0.0f, true, 20.0f, true);
	cv->registerCvar("accuracy", "0.5", " dd ", true, true, 0.0f, true, 3.0f, true);
	cv->registerCvar("max_angular_speed", "1.5", " dd ", true, true, 0.0f, true, 100.0f, true);
	
		timeout(gw.get());
		pinCarOnTheAir(gw.get());
		
	

	gw->RegisterDrawable(bind(&AirControlTraining::Render, this, placeholders::_1));
}

void AirControlTraining::onUnload() {


}

void AirControlTraining::timeout(GameWrapper* eventName) {

	std::shared_ptr<GameWrapper> gw = this->gameWrapper;
	std::shared_ptr<CVarManagerWrapper> cv = this->cvarManager;
	if (!gw->IsInFreeplay()) return;
	if(cv->getCvar("timing").getFloatValue() > 0)
		gw->SetTimeout(bind(&AirControlTraining::timeout, this, placeholders::_1), cv->getCvar("timing").getFloatValue());
	RandomBallPosition(MaxDistanceBall,MinDistanceBall);
	
}

void AirControlTraining::pinCarOnTheAir(GameWrapper* eventName) {

	std::shared_ptr<GameWrapper> gw = this->gameWrapper;
	std::shared_ptr<CVarManagerWrapper> cv = this->cvarManager;
	if (!gw->IsInFreeplay()) return;
	gw->SetTimeout(bind(&AirControlTraining::pinCarOnTheAir, this, placeholders::_1), 0.01f);

	Vector posCar(0, 0, 1500);
	Vector velCar(0, 0, 0);
	gw->GetLocalCar().SetLocation(posCar);
	gw->GetLocalCar().SetVelocity(velCar);

	shoot();
}

void AirControlTraining::shoot() {
	std::shared_ptr<GameWrapper> gw = this->gameWrapper;
	std::shared_ptr<CVarManagerWrapper> cv = this->cvarManager;

	maxAngularSpeed = cv->getCvar("max_angular_speed").getFloatValue();
	if (!gw->IsInFreeplay()) return;
	
	ServerWrapper svw = gw->GetGameEventAsServer();
	if (svw.GetGameBalls().Count() == 0)return;
	BallWrapper ball = svw.GetBall();
	Vector directionCar = RotatorToVector(gw->GetLocalCar().GetRotation());
	Vector look = ball.GetLocation() - gw->GetLocalCar().GetLocation();
	angleCarBallByPlayer = angle(look, directionCar);

	angularSpeed = sqrt(pow(gw->GetLocalCar().GetAngularVelocity().X, 2) +
		pow(gw->GetLocalCar().GetAngularVelocity().Y, 2) +
		pow(gw->GetLocalCar().GetAngularVelocity().Z, 2));

	if (angleCarBallByPlayer < cv->getCvar("accuracy").getFloatValue() && gw->GetLocalCar().IsBoostCheap() && mustToBoost 
		&& angularSpeed < maxAngularSpeed) {
		if(nbBallTouched == 0 )  timeBegin = time(0);
		RandomBallPosition(MaxDistanceBall, MinDistanceBall);
		mustToBoost = false;
		nbBallTouched++;
		timeLastTouch = time(0);
	}
	else if (gw->GetLocalCar().IsBoostCheap() && mustToBoost && timeBegin!=0) {
		mustToBoost = false;
		nbBallMissed++;
	}
	
	if (!mustToBoost && !gw->GetLocalCar().IsBoostCheap()) mustToBoost = true;
	
}

void AirControlTraining::RandomBallPosition(float distMax,float distMin)
{
	std::shared_ptr<GameWrapper> gw = this->gameWrapper;

	if (!gw->IsInFreeplay()) return;
	
	ServerWrapper svw = gw->GetGameEventAsServer();
	if (svw.GetGameBalls().Count() == 0)return;

	BallWrapper ball = svw.GetBall();

	double distanceBall = std::rand() % (MaxDistanceBall - MinDistanceBall) + MinDistanceBall;
	double angle1 = std::rand() % 360;

	double distanceBall2 = sin(angle1) * distanceBall;
	double x = cos(angle1) * distanceBall;

	double angle2 = std::rand() % 180;
	double y = cos(angle2) * distanceBall2;
	double z = sin(angle2) * distanceBall2;
	Vector posCar = svw.GetCars().Get(0).GetLocation();
	Vector posBall;
	posBall.X = posCar.X + (int)x;
	posBall.Y = posCar.Y + (int)y;
	posBall.Z = posCar.Z + (int)z;
	
	ball.SetLocation(posBall);
	ball.Stop();
	
}

void AirControlTraining::Render(CanvasWrapper canvas) {
	
	std::shared_ptr<CVarManagerWrapper> cv = this->cvarManager;
	std::shared_ptr<GameWrapper> gw = this->gameWrapper;

	if (cv->getCvar("stat").getIntValue() == 0) {
		float ratio = ((float)nbBallTouched * 100) / (float)(nbBallTouched + nbBallMissed);
		float ballSec = ((float)nbBallTouched / (float)(timeLastTouch - timeBegin));
		float maxValue = cv->getCvar("accuracy").getFloatValue();

		Vector2 pos;
		Vector2 sizeBox;
		sizeBox.X = 250;
		sizeBox.Y = 50;

		pos.X = 0;
		pos.Y = canvas.GetSize().Y - (sizeBox.Y * 5);
		LinearColor BackColor;
		BackColor.A = 100;
		if (angularSpeed < maxAngularSpeed) {
			BackColor.R = 50;
			BackColor.G = 50;
			BackColor.B = 50;
		}
		else {
			BackColor.R = 220;
			BackColor.G = 50;
			BackColor.B = 50;
		}
		for (int i = 0; i < 5; i++) {
			canvas.SetPosition(pos);
			canvas.SetColor(200,200,200,220);
			canvas.DrawBox(sizeBox);
			canvas.SetColor(BackColor);
			canvas.FillBox(sizeBox);
			pos.Y += sizeBox.Y;
		}
		if (nbBallTouched != 0 || nbBallMissed != 0) {


			pos.X += 5;
			pos.Y = canvas.GetSize().Y - (sizeBox.Y * 5) + 5;

			canvas.SetPosition(pos);
			canvas.SetColor(0, 200, 20, 250);
			canvas.DrawString("ratio : " + to_string((int)ratio), 2, 2);

			pos.Y += sizeBox.Y;
			canvas.SetPosition(pos);
			canvas.DrawString(to_string(ballSec) + " bal/sec", 2, 2);

			pos.Y += sizeBox.Y;
			canvas.SetPosition(pos);
			canvas.DrawString(to_string(nbBallTouched) + " + " + to_string(nbBallMissed) + " balles", 2, 2);

			pos.Y += sizeBox.Y;
			canvas.SetPosition(pos);
			canvas.DrawString("degres :" + to_string(cv->getCvar("accuracy").getFloatValue()), 2, 2);

			pos.Y += sizeBox.Y;
			canvas.SetPosition(pos);
			canvas.DrawString(to_string(time(0) - timeBegin) + " sec", 2, 2);
		}
	}else {
		Vector2 pos;

		pos.X = 0;
		pos.Y = canvas.GetSize().Y-15;
		canvas.SetColor(200, 200, 200, 200);
		canvas.SetPosition(pos);
		canvas.DrawString("mettre cvar <stat> à 0 pour affcher les scores");
	}

}



float AirControlTraining::angle(Vector vec1, Vector vec2) {

	Vector Resultat = vec1 * vec2;

	float angleoutput = acos((Resultat.Z + Resultat.X + Resultat.Y) /
		(sqrt(vec1.X * vec1.X + vec1.Y * vec1.Y + vec1.Z * vec1.Z) *
			sqrt(vec2.X * vec2.X + vec2.Y * vec2.Y + vec2.Z * vec2.Z)));

	return angleoutput;
}